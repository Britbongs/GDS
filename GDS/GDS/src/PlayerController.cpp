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

	if (!m_pTankTexture->generateMipmap())
	{
		KPrintf(KTEXT("Unable to generate mipmap for tank texture!\n"));
	}

	if (!m_pLauncherTexture->generateMipmap())
	{
		KPrintf(KTEXT("Unable to generate mipmap for launcher texture!\n"));
	}

	m_pTankTexture->setSmooth(true);
	m_pLauncherTexture->setSmooth(true);

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

	KINIT_CHECK(setupArrow());

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
	m_pLauncher->getComponent<KCTransform>()->setTranslation(Vec2f(20.0f, 12.0f));
	m_pLauncher->getComponent<KCSprite>()->setTexture(m_pLauncherTexture);
	m_pLauncher->getComponent<KCSprite>()->setTextureRect(LauncherTexRect);

	KAssetLoader& rAssetLoader = KAssetLoader::getAssetLoader();
	sf::Texture* pTexHead = rAssetLoader.loadTexture(KTEXT("ArrowHead.png"));
	sf::Texture* pTexBody = rAssetLoader.loadTexture(KTEXT("ArrowBody.png"));
	m_pArrowHead->getComponent<KCSprite>()->setTexture(pTexHead);
	m_pArrowHead->getComponent<KCSprite>()->setTextureRect(Recti(0, 0, 64, 64));
	m_pArrowHead->getComponent<KCTransform>()->setOrigin(Vec2f(pTexHead->getSize().x / 1.5f, 0));

	m_pArrowBody->getComponent<KCSprite>()->setTexture(pTexBody);
	m_pArrowBody->getComponent<KCSprite>()->setTextureRect(Recti(0, 0, 32, 128));
	m_pArrowBody->getComponent<KCTransform>()->setOrigin(Vec2f(pTexBody->getSize().x / 1.5f, 0));
	m_pArrowBody->getComponent<KCTransform>()->setParent(m_pArrowHead);
	m_pArrowBody->getComponent<KCTransform>()->setTranslation(32, 8);

	KCHECK(m_pProjectileHandler);
}

void PlayerController::tick()
{
	float dt = KApplication::getApp()->getDeltaTime();
	float rot = 0.0f;

	KCTransform* pLauncherTransform = m_pLauncher->getComponent<KCTransform>();

	if (KInput::Pressed(KKey::Q))
	{
		rot -= ROTATION_AMOUNT;
	}

	if (KInput::Pressed(KKey::E))
	{
		rot += ROTATION_AMOUNT;
	}

	const float launcherRotationIndependantOfParentTransform = pLauncherTransform->getRotation() - m_pTransformComponent->getRotation();

	if (KInput::Pressed(KKey::A))
	{
		if (launcherRotationIndependantOfParentTransform > -90.0f)
			m_pLauncher->getComponent<KCTransform>()->rotate(-ROTATION_AMOUNT*dt);
	}

	if (KInput::Pressed(KKey::D))
	{
		if (launcherRotationIndependantOfParentTransform < 90.0f)
			pLauncherTransform->rotate(ROTATION_AMOUNT*dt);
	}

	m_rotationAngleDegrees += rot * dt;
	m_pTransformComponent->rotate(rot*dt);

	updateTranslation();

	if (KInput::JustPressed(KKey::Space))
	{
		Vec2f launchDirection;
		launchDirection = RotateVector(Vec2f(0, -1), m_pLauncher->getComponent<KCTransform>()->getRotation());
		m_pProjectileHandler->fireProjectile(pLauncherTransform->getPosition(), launchDirection);
	}

	if (KInput::MouseJustPressed(KMouseButton::Left))
	{
		if (!m_bSetDrawingArrow)
		{
			m_bSetDrawingArrow = true;
			m_mousePosA = KInput::GetMouseWorldPosition();
		}
	}

	if (KInput::MousePressed(KMouseButton::Left))
	{
		if (m_bSetDrawingArrow)
		{
			m_mousePosB = KInput::GetMouseWorldPosition();
			float angleInRad = GetAngleBetween(m_mousePosA, m_mousePosB);
			angleInRad = Maths::Degrees(-angleInRad);
			m_pArrowHead->getComponent<KCTransform>()->setRotation(angleInRad);
		}
	}

	if (KInput::MouseJustReleased(KMouseButton::Left))
	{
		m_bSetDrawingArrow = false;
	}

	m_pArrowHead->getComponent<KCTransform>()->setTranslation(KInput::GetMouseWorldPosition());
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

KInitStatus PlayerController::setupArrow()
{
	KScene* pCurrentScene = KApplication::getApp()->getCurrentScene();
	KCHECK(pCurrentScene);

	m_pArrowHead = pCurrentScene->addEntityToScene();
	m_pArrowBody = pCurrentScene->addEntityToScene();

	KCHECK(m_pArrowHead);
	KCHECK(m_pArrowBody);

	if (!m_pArrowHead)
	{
		return KInitStatus::Nullptr;
	}

	if (!m_pArrowBody)
	{
		return KInitStatus::Nullptr;
	}

	sf::Texture* pTexHead = KAssetLoader::getAssetLoader().loadTexture(KTEXT("ArrowHead.png"));
	sf::Texture* pTexBody = KAssetLoader::getAssetLoader().loadTexture(KTEXT("ArrowBody.png"));
	if (!pTexHead)
	{
		return KInitStatus::MissingResource;
	}

	if (!pTexBody)
	{
		return KInitStatus::MissingResource;
	}

	m_pArrowHead->addComponent(new KCSprite(m_pArrowHead, Vec2f(pTexHead->getSize()) / 1.5f));
	m_pArrowBody->addComponent(new KCSprite(m_pArrowBody, Vec2f(pTexBody->getSize()) / 1.5f));

	return KInitStatus::Success;
}

KInitStatus PlayerController::setupSatellite()
{
	auto pEntity = getEntity();

	pEntity->addComponent(new KCSprite(pEntity, Vec2f(SATELLITE_WIDTH, SATELLITE_HEIGHT)));

	return Krawler::KInitStatus::Success;
}
