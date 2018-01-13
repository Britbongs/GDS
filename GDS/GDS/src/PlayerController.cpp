#include "PlayerController.h"

#include "Components\KCSprite.h"

#include <Input\KInput.h>
#include <KApplication.h>

using namespace Krawler;
using namespace Krawler::Input;
using namespace Krawler::Components;

#define SATELLITE_WIDTH 24
#define SATELLITE_HEIGHT 16

PlayerController::PlayerController(KEntity* pEntity, const Vec2f& planetCentrePos, float playerPlanetRadius)
	: KComponentBase(pEntity), PLANET_CENTRE_POS(planetCentrePos), PLANET_RADIUS(playerPlanetRadius), ROTATION_AMOUNT(60.0f), m_orbitDistance(30.0f), m_rotationAngleDegrees(-90)
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
	//m_pTransformComponent->setTranslation(Vec2f(0.0f, -(m_planetRadius + m_orbitDistance)));
	updateTranslation();

	return Krawler::KInitStatus::Success;
}

void PlayerController::onEnterScene()
{
	KScene* const pCurrentScene = KApplication::getApp()->getCurrentScene();
	KEntity* pLevelSetup = pCurrentScene->findEntityByTag(KTEXT("level setup"));
	m_pProjectileHandler = pLevelSetup->getComponent<ProjectileHandler>();
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


	m_rotationAngleDegrees += rot * dt;
	m_pTransformComponent->rotate(rot*dt);

	updateTranslation();

	if (KInput::JustPressed(KKey::Space))
	{
		const float angleInRad = Maths::Radians(m_rotationAngleDegrees);

		const Vec2f scale = m_pTransformComponent->getScale();
		Vec2f origin = m_pTransformComponent->getOrigin();
		Vec2f centre = m_pTransformComponent->getPosition() + RotateVector(Vec2f(origin.x * scale.x, origin.y * scale.y), m_rotationAngleDegrees);
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
