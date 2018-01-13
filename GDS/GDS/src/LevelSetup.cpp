#include "LevelSetup.h"

#include <KApplication.h>
#include <KScene.h>

#include <Input\KInput.h>
#include <AssetLoader\KAssetLoader.h>

#include <Components\KCSprite.h>
#include <Components\KCCircleCollider.h>
#include <Components\KCBoxCollider.h>

#include "GravitationalController.h"
#include "PlayerController.h"
#include "Projectiles.h"

using namespace Krawler;
using namespace Krawler::Input;
using namespace Krawler::Components;

#define TEST_BOX_COUNT 20

/*-- Level Setup --*/

LevelSetup::LevelSetup(KEntity * pEntity)
	: KComponentBase(pEntity)
{
}

KInitStatus LevelSetup::init()
{
	getEntity()->addComponent(new GravitationalController(getEntity()));
	Physics::KPhysicsWorldProperties worldProperties;
	worldProperties.gravity = Vec2f(0.0f, 0.f);
	worldProperties.metresToPixels = METRES_TO_PIXELS;


	//setup physics body properties for the planet
	m_planetPhysicsProperties.restitution = 0.2f;
	m_planetPhysicsProperties.staticFriction = 0.8f;
	m_planetPhysicsProperties.dynamicFriction = 0.8f;

	KApplication::getApp()->getPhysicsWorld()->setPhysicsWorldProperties(worldProperties);
	KINIT_CHECK(setupPlayerEntities());
	KINIT_CHECK(addProjectiles());
	KINIT_CHECK(createExtraPlanets());


	m_p8BallTexture = KAssetLoader::getAssetLoader().loadTexture(L"8ball.png");
	return KInitStatus::Success;
}

void LevelSetup::onEnterScene()
{
	KScene* pCurrentScene = KApplication::getApp()->getCurrentScene();

	auto sprite = m_pPlayerPlanet->getComponent<KCSprite>();
	sprite->setTexture(m_pPlanetTexture);
	sprite->setTextureRect(Recti(0, 0, 256, 256));

	auto pTransform = m_pPlayerPlanet->getComponent<KCTransform>();
	pTransform->setOrigin(Vec2f(PLANET_RADIUS, PLANET_RADIUS));

	setupPlanetPositionsAndTextures();
}

void LevelSetup::tick()
{
	if (KInput::MouseJustPressed(KMouseButton::Left))
	{
		KPrintf(KTEXT("%f-%f"), KInput::GetMouseWorldPosition().x, KInput::GetMouseWorldPosition().y);

	}
}

KInitStatus LevelSetup::setupPlayerEntities()
{
	KScene* pCurrentScene = KApplication::getApp()->getCurrentScene();
	m_pPlayerPlanet = pCurrentScene->addEntityToScene();
	m_pPlayerPlanet->setEntityTag(KTEXT("player planet"));

	if (!m_pPlayerPlanet)
	{
		KPrintf(KTEXT("Unable to add player planet to scene!\n"));
		return Failure;
	}

	m_pPlayerPlanet->addComponent(new KCSprite(m_pPlayerPlanet, Vec2f(PLANET_RADIUS * 2.0f, PLANET_RADIUS * 2.0f)));


	auto pPhysicsWorld = KApplication::getApp()->getPhysicsWorld();

	//calculate volume of planet in m3 
	//then multiply this volume by a pre-determined density 
	//this value will produce the mass
	float radiusInMetres = PLANET_RADIUS * pPhysicsWorld->getPhysicsWorldProperties().metresToPixels;
	float mass = ((radiusInMetres * radiusInMetres)  * Maths::PI);
	mass *= MINIMUM_PLANET_DENSITY;

	KPhysicsBodyProperties planetPhysProperties(m_planetPhysicsProperties);
	planetPhysProperties.setMass(mass);

	m_pPlayerPlanet->addComponent(new KCPhysicsBody(m_pPlayerPlanet, planetPhysProperties));
	m_pPlayerPlanet->addComponent(new KCCircleCollider(m_pPlayerPlanet, PLANET_RADIUS));
	m_pPlayerPlanet->addComponent(new StaticPlanetController(m_pPlayerPlanet, Vec2f(KApplication::getApp()->getWindowSize()) * 0.5f));

	//load texture for the planet and assign it to the member pointer to texture
	KAssetLoader& assetLoader = KAssetLoader::getAssetLoader();
	assetLoader.setRootFolder(KTEXT("res\\"));
	m_pPlanetTexture = assetLoader.loadTexture(KTEXT("planet.png"));

	if (!m_pPlanetTexture)
	{
		return KInitStatus::MissingResource;
	}

	KEntity* const pPlayerSatellite = pCurrentScene->addEntityToScene();
	pPlayerSatellite->addComponent(new PlayerController(pPlayerSatellite, Vec2f(KApplication::getApp()->getWindowSize()) * 0.5f, PLANET_RADIUS));

	return KInitStatus::Success;
}

KInitStatus LevelSetup::addProjectiles()
{
	auto pCurrentScene = KApplication::getApp()->getCurrentScene();

	int32 allocd = 0;
	KEntity* pEntities = pCurrentScene->addEntitiesToScene(CACHED_PROJECTILE_COUNT, allocd);
	if (allocd != CACHED_PROJECTILE_COUNT || !pEntities)
	{
		KPrintf(KTEXT("Unable to create enough entities for projectile cache count!\n"));
		return KInitStatus::Failure;
	}

	std::vector<KEntity*> vec(allocd);
	for (int32 i = 0; i < allocd; ++i)
	{
		vec[i] = &pEntities[i];
		vec[i]->addComponent(new Projectile(vec[i], i));
	}

	getEntity()->addComponent(new ProjectileHandler(getEntity(), vec));


	return KInitStatus::Success;
}

KInitStatus LevelSetup::createExtraPlanets()
{
	auto currentScene = KApplication::getApp()->getCurrentScene();
	auto pPhysicsWorld = KApplication::getApp()->getPhysicsWorld();

	float radiusInMetres = PLANET_RADIUS * pPhysicsWorld->getPhysicsWorldProperties().metresToPixels;
	float mass = ((radiusInMetres * radiusInMetres)  * Maths::PI);
	mass *= MINIMUM_PLANET_DENSITY;

	KPhysicsBodyProperties planet(m_planetPhysicsProperties);
	planet.setMass(mass);

	for (int32 i = 0; i < EXTRA_PLANET_COUNT; ++i)
	{
		m_extraPlanets[i] = currentScene->addEntityToScene();
		KCHECK(m_extraPlanets[i]);
		if (!m_extraPlanets[i])
		{
			KPrintf(KTEXT("Unable to add extra planets to scene!\n"));
			return KInitStatus::Failure;
		}
		m_extraPlanets[i]->addComponent(new KCPhysicsBody(m_extraPlanets[i], planet));
		m_extraPlanets[i]->addComponent(new KCCircleCollider(m_extraPlanets[i], PLANET_RADIUS));
		m_extraPlanets[i]->addComponent(new KCSprite(m_extraPlanets[i], Vec2f(2 * PLANET_RADIUS, 2 * PLANET_RADIUS)));
		m_extraPlanets[i]->addComponent(new StaticPlanetController(m_extraPlanets[i], Vec2f(0.0f, 0.0f)));
	}

	return KInitStatus::Success;
}

void LevelSetup::setupPlanetPositionsAndTextures()
{
	const Vec2f screenBounds(KApplication::getApp()->getWindowSize());
	KCollisionDetectionData data;

	for (int32 i = 0; i < EXTRA_PLANET_COUNT; ++i)
	{
		KEntity*& pPlanet = m_extraPlanets[i];
		pPlanet->setEntityTag(KTEXT("planet ") + std::to_wstring(i));
		Vec2f pos;
		pos.x = Maths::RandFloat(0.0f, screenBounds.x);
		pos.y = Maths::RandFloat(0.0f, screenBounds.y);
		pPlanet->getComponent<KCTransform>()->setTranslation(pos);
		KCSprite* const pSprite = pPlanet->getComponent<KCSprite>();

		pSprite->setTexture(m_pPlanetTexture);
		pSprite->setTextureRect(Recti(256, 256, 256, 256));

	}

	m_extraPlanets[0]->getComponent<KCTransform>()->setTranslation(Vec2f(134, 97));
	m_extraPlanets[0]->getComponent<StaticPlanetController>()->setPositionToMaintain(Vec2f(134, 97));

	m_extraPlanets[1]->getComponent<KCTransform>()->setTranslation(Vec2f(763, 388));
	m_extraPlanets[1]->getComponent<StaticPlanetController>()->setPositionToMaintain(Vec2f(763, 388));


	m_extraPlanets[2]->getComponent<KCTransform>()->setTranslation(Vec2f(118, 504));
	m_extraPlanets[2]->getComponent<StaticPlanetController>()->setPositionToMaintain(Vec2f(118, 504));
}
