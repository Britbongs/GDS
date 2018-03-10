#include "PlayerController.h"

#include <Components\KCSprite.h>

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

#define POWER_METER_SIZE 64.0f

PlayerController::PlayerController(KEntity* pEntity, const Vec2f& planetCentrePos, float playerPlanetRadius)
	: KComponentBase(pEntity), PLANET_CENTRE_POS(planetCentrePos), PLANET_RADIUS(playerPlanetRadius), ROTATION_AMOUNT(60.0f),
	m_orbitDistance(30.0f), m_rotationAngleDegrees(-90), m_pLauncher(nullptr), m_pLauncherTexture(nullptr),
	m_pValueBar(nullptr)
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

	KINIT_CHECK(setupPowermeter());

	return Krawler::KInitStatus::Success;
}

void PlayerController::onEnterScene()
{
	KScene* const pCurrentScene = KApplication::getApp()->getCurrentScene();
	KEntity* const pLevelSetup = pCurrentScene->findEntityByTag(KTEXT("level setup"));

	m_pProjectileHandler = pLevelSetup->getComponent<ProjectileHandler>();

	const Recti TankTexRect(0, 0, m_pTankTexture->getSize().x, m_pTankTexture->getSize().y);
	const Recti LauncherTexRect(0, 0, m_pLauncherTexture->getSize().x, m_pLauncherTexture->getSize().y);

	{ // Satellite/Tank (parentEntity)
		KCSprite* const pSpriteComponent = getEntity()->getComponent<KCSprite>();

		pSpriteComponent->setTexture(m_pTankTexture);
		pSpriteComponent->setTextureRect(TankTexRect);
	}

	{ // Launcher
		KCTransform* const pLauncherTransform = m_pLauncher->getComponent<KCTransform>();
		KCSprite* const pLauncherSprite = m_pLauncher->getComponent<KCSprite>();

		pLauncherTransform->setParent(getEntity());
		pLauncherTransform->setTranslation(Vec2f(20.0f, 12.0f));

		pLauncherSprite->setTexture(m_pLauncherTexture);
		pLauncherSprite->setTextureRect(LauncherTexRect);
	}

	{// Power Meter 

		KCSprite* const pPowerMeterSprite = m_pPowerMeter->getComponent<KCSprite>();
		KCTransform* const pPowerMeterTransform = m_pPowerMeter->getComponent<KCTransform>();
		pPowerMeterSprite->setTexture(m_pPowerMeterTexture);

		m_pPowerMeter->getComponent<KCTransform>()->setOrigin(POWER_METER_SIZE / 2.0f, POWER_METER_SIZE / 2.0f);
		const Vec2f WindowSizeFloat = Vec2f(KApplication::getApp()->getWindowSize());
		pPowerMeterTransform->setTranslation(WindowSizeFloat - Vec2f(32.0f, (WindowSizeFloat.y - POWER_METER_SIZE)));

		KCSprite* const pValueBarSprite = m_pValueBar->getComponent<KCSprite>();
		KCTransform* const pValueBarTransform = m_pValueBar->getComponent<KCTransform>();
		KCHECK(pValueBarSprite);
		KCHECK(pValueBarTransform);

		pValueBarSprite->setTexture(m_pValueBarTexture);
		pValueBarTransform->setTranslation(Vec2f(0.0f, POWER_METER_SIZE * m_shotPowerValue));

	}

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
		{
			pLauncherTransform->rotate(-ROTATION_AMOUNT * dt);
		}
	}

	KCSprite* pPowerMeterSprite = m_pPowerMeter->getComponent<KCSprite>();
	KCTransform* pPowerMeterTransform = m_pPowerMeter->getComponent<KCTransform>();
	KCTransform* pValueBarTransform = m_pValueBar->getComponent<KCTransform>();

	pValueBarTransform->setTranslation(Vec2f(0.0f, POWER_METER_SIZE * (1.0f - m_shotPowerValue)));

	if (KInput::JustPressed(KKey::Up))
	{
		// up shot power
		m_shotPowerValue += 0.1f;
	}

	if (KInput::JustPressed(KKey::Down))
	{
		m_shotPowerValue -= 0.1f;
	}

	m_shotPowerValue = Maths::Clamp(0.0f, 1.0f, m_shotPowerValue);

	if (KInput::Pressed(KKey::D))
	{
		if (launcherRotationIndependantOfParentTransform < 90.0f)
		{
			pLauncherTransform->rotate(ROTATION_AMOUNT*dt);
		}
	}

	m_rotationAngleDegrees += rot * dt;
	m_pTransformComponent->rotate(rot*dt);

	updateTranslation();

	static bool spaceJustPressed = false;

	if (KInput::JustPressed(KKey::Space))
	{
		if (!spaceJustPressed)
		{
			Vec2f launchDirection;
			launchDirection = RotateVector(Vec2f(0, -1), pLauncherTransform->getRotation());
			m_pProjectileHandler->fireProjectileWithForce(pLauncherTransform->getPosition(), launchDirection, m_shotPowerValue);
			spaceJustPressed = true;
		}
	}
	else
	{
		if (m_spamTimer < 1.5f)
		{
			m_spamTimer += dt;
		}
		else
		{
			spaceJustPressed = false;
			m_spamTimer = 0.0f;
		}
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

void PlayerController::powerMeterUpdate()
{
}

KInitStatus PlayerController::setupPowermeter()
{
	KScene* pCurrentScene = KApplication::getApp()->getCurrentScene();
	KCHECK(pCurrentScene);

	m_pPowerMeter = pCurrentScene->addEntityToScene();
	KCHECK(m_pPowerMeter);
	if (!m_pPowerMeter)
	{
		return KInitStatus::Nullptr;
	}

	m_pPowerMeterTexture = KAssetLoader::getAssetLoader().loadTexture(KTEXT("Powermeter.png"));
	if (!m_pPowerMeterTexture)
	{
		return KInitStatus::MissingResource;
	}
	m_pPowerMeter->addComponent(new KCSprite(m_pPowerMeter, Vec2f(m_pPowerMeterTexture->getSize())));


	m_pValueBar = pCurrentScene->addEntityToScene();
	KCHECK(pCurrentScene);

	if (!m_pValueBar)
	{
		return KInitStatus::Nullptr;
	}

	m_pValueBarTexture = KAssetLoader::getAssetLoader().loadTexture(KTEXT("power_meter_value_bar.png"));

	if (!m_pValueBarTexture)
	{
		return KInitStatus::MissingResource;
	}

	m_pValueBar->addComponent(new KCSprite(m_pValueBar, Vec2f(m_pValueBarTexture->getSize())));
	m_pValueBar->getComponent<KCTransform>()->setParent(m_pPowerMeter);

	return KInitStatus::Success;
}

KInitStatus PlayerController::setupSatellite()
{
	auto pEntity = getEntity();

	pEntity->addComponent(new KCSprite(pEntity, Vec2f(SATELLITE_WIDTH, SATELLITE_HEIGHT)));

	return Krawler::KInitStatus::Success;
}
