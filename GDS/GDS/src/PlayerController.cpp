#include "PlayerController.h"

#include "Components\KCSprite.h"

#include <Input\KInput.h>
#include <KApplication.h>
#include <AssetLoader\KAssetLoader.h>

using namespace Krawler;
using namespace Krawler::Input;
using namespace Krawler::Components;

#define SATELLITE_WIDTH 32
#define SATELLITE_HEIGHT 32

#define CANNON_WIDTH 16
#define CANNON_HEIGHT 32

PlayerController::PlayerController(KEntity* pEntity, const Vec2f& planetCentrePos, float playerPlanetRadius)
	: KComponentBase(pEntity), PLANET_CENTRE_POS(planetCentrePos), PLANET_RADIUS(playerPlanetRadius), ROTATION_AMOUNT(60.0f),
	m_orbitDistance(30.0f), m_rotationAngleDegrees(-90), m_pLauncher(nullptr), m_pLauncherTexture(nullptr)
{
	setComponentTag(KTEXT("player controller"));
}

Krawler::KInitStatus PlayerController::init()
{
	m_pTransformComponent = getEntity()->getComponent<KCTransform>();
	if (!m_pTransformComponent)
	{
		KPrintf(KTEXT("No transform component attatched to %ws"), getEntity()->getEntityTag() + KTEXT("\n"));
	}
	KINIT_CHECK(setupSatellite());
	m_pTransformComponent->setOrigin(SATELLITE_WIDTH / 2.0f, SATELLITE_HEIGHT / 2.0f);
	updateTranslation();

	m_pTankTexture = KAssetLoader::getAssetLoader().loadTexture(KTEXT("Tank.png"));
	m_pLauncherTexture = KAssetLoader::getAssetLoader().loadTexture(KTEXT("Launcher.png"));

	if (!m_pTankTexture)
	{
		return Krawler::KInitStatus::MissingResource;
	}

	if (!m_pLauncherTexture)
	{
		return Krawler::KInitStatus::MissingResource;
	}

	KScene* pCurrentScene = KApplication::getApp()->getCurrentScene();
	KCHECK(pCurrentScene);

	m_pLauncher = pCurrentScene->addEntityToScene();

	KCHECK(m_pLauncher);

	if (!m_pLauncher)
	{
		return KInitStatus::Failure;
	}

	m_pLauncher->addComponent(new KCSprite(m_pLauncher, Vec2f(CANNON_WIDTH, CANNON_HEIGHT)));
	m_pLauncher->getComponent<KCTransform>()->setOrigin(Vec2f(CANNON_WIDTH * 0.48f, CANNON_HEIGHT*0.87f));
	getEntity()->setEntityTag(KTEXT("player_tank"));
	m_pLauncher->setEntityTag(KTEXT("player_launcher"));
	return Krawler::KInitStatus::Success;
}

void PlayerController::onEnterScene()
{
	KScene* const pCurrentScene = KApplication::getApp()->getCurrentScene();
	KEntity* pLevelSetup = pCurrentScene->findEntityByTag(KTEXT("level setup"));
	m_pProjectileHandler = pLevelSetup->getComponent<ProjectileHandler>();

	const Recti TankTexRect(0, 0, m_pTankTexture->getSize().x, m_pTankTexture->getSize().y);
	const Recti LauncherTexRect(0, 0, m_pLauncherTexture->getSize().x, m_pLauncherTexture->getSize().y);

	getEntity()->getComponent<KCSprite>()->setTexture(m_pTankTexture);
	getEntity()->getComponent<KCSprite>()->setTextureRect(TankTexRect);

	m_pLauncher->getComponent<KCTransform>()->setParent(getEntity());
	m_pLauncher->getComponent<KCSprite>()->setTexture(m_pLauncherTexture);
	m_pLauncher->getComponent<KCSprite>()->setTextureRect(LauncherTexRect);

	KCHECK(m_pProjectileHandler);
}

void PlayerController::tick()
{
	float dt = KApplication::getApp()->getDeltaTime();
	float rot = 0.0f;

	if (KInput::Pressed(KKey::Q))
	{
		rot -= ROTATION_AMOUNT;
	}

	if (KInput::Pressed(KKey::E))
	{
		rot += ROTATION_AMOUNT;
	}

	if (KInput::Pressed(KKey::A))
	{
		m_pLauncher->getComponent<KCTransform>()->rotate(ROTATION_AMOUNT*dt);
	}

	if (KInput::Pressed(KKey::D))
	{
		m_pLauncher->getComponent<KCTransform>()->rotate(-ROTATION_AMOUNT*dt);
	}

	m_rotationAngleDegrees += rot * dt;
	m_pTransformComponent->rotate(rot*dt);

	updateTranslation();

	if (KInput::JustPressed(KKey::Space))
	{
		const float angleInRad = Maths::Radians(m_rotationAngleDegrees);

		const Vec2f scale = m_pTransformComponent->getScale();
		const Vec2f origin = m_pTransformComponent->getOrigin();
		const Vec2f centre = m_pTransformComponent->getPosition() + RotateVector(Vec2f(origin.x * scale.x, origin.y * scale.y), m_rotationAngleDegrees);
		m_pProjectileHandler->fireProjectile(centre, Vec2f(cosf(angleInRad), sinf(angleInRad)));
	}
}

void PlayerController::fixedTick()
{

}

void PlayerController::updateTranslation()
{
	Vec2f trans;
	trans.x = cosf(Maths::Radians(m_rotationAngleDegrees))* (PLANET_RADIUS + m_orbitDistance);
	trans.y = sinf(Maths::Radians(m_rotationAngleDegrees))* (PLANET_RADIUS + m_orbitDistance);
	m_pTransformComponent->setTranslation(PLANET_CENTRE_POS + trans);
}

KInitStatus PlayerController::setupSatellite()
{
	auto pEntity = getEntity();

	pEntity->addComponent(new KCSprite(pEntity, Vec2f(SATELLITE_WIDTH, SATELLITE_HEIGHT)));

	return Krawler::KInitStatus::Success;
}
