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
#include "PlanetTarget.h"

using namespace Krawler;
using namespace Krawler::Input;
using namespace Krawler::Components;
using namespace Krawler::TiledMap;

#define TEST_BOX_COUNT 20
#define GRID_NODE_SIZE 1

/*-- Level Setup --*/

LevelSetup::LevelSetup(KEntity * pEntity)
	: KComponentBase(pEntity)
{
}

KInitStatus LevelSetup::init()
{
	getEntity()->addComponent(new GravitationalController(getEntity()));
	Vec2f screenBounds(KApplication::getApp()->getWindowSize());

	//add background to scene
	KAssetLoader::getAssetLoader().setRootFolder(KTEXT("res\\"));
	m_pBackground = KAssetLoader::getAssetLoader().loadTexture(KTEXT("space.png"));

	if (!sf::Shader::isAvailable())
	{
		KPrintf(L"Eror!! SHADERS NOT ALLOWED ON THIS MACHINE!\n");
		return KInitStatus::Failure;
	}

	m_gravityMapShader = KAssetLoader::getAssetLoader().loadShader(KTEXT("mapVert.glsl"), KTEXT("mapFrag.glsl"));
	m_defaultBackgroundShader = KAssetLoader::getAssetLoader().loadShader(KTEXT("defaultVert.glsl"), KTEXT("defaultFrag.glsl"));

	setupBackgroundTiledmap();
	//setup physics world properties
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
	static float modifier = 1.0f;
	if (KInput::MouseJustPressed(KMouseButton::Left))
	{
		KPrintf(KTEXT("%f-%f"), KInput::GetMouseWorldPosition().x, KInput::GetMouseWorldPosition().y);
	}

	if (KInput::JustPressed(KKey::Tab))
	{
		m_bShowMap = !m_bShowMap;
		if (m_bShowMap)
		{
			m_gravityMapTiledMap.setShader(m_gravityMapShader);
		}
		else
		{
			m_gravityMapTiledMap.setShader(m_defaultBackgroundShader);
		}
	}
#ifdef _DEBUG
	if (KInput::JustPressed(KKey::R))
	{
		m_gravityMapShader->loadFromFile("res\\mapVert.glsl", "res\\mapFrag.glsl");
	}
#endif
	if (KInput::Pressed(KKey::Escape))
	{
		KApplication::getApp()->closeApplication();
	}

	//PASSING SHADER PARAMETERS
	std::vector<Vec2f>centres;
	std::vector<sf::Glsl::Vec4> colours;
	for (auto& extra : m_extraPlanets)
	{
		centres.push_back(extra->getComponent<KCCircleCollider>()->getCentrePosition());
	}
	centres.push_back(m_pPlayerPlanet->getComponent<KCCircleCollider>()->getCentrePosition());

	for (int32 i = 0; i <= EXTRA_PLANET_COUNT; ++i)
	{
		colours.push_back(sf::Glsl::Vec4((float)(m_planetCols[i].r) / 256.0f, (float)(m_planetCols[i].g) / 256.0f, (float)(m_planetCols[i].b) / 256.0f, 1.0f));
	}

	m_gravityMapShader->setUniformArray("planetPos", &centres[0], centres.size());
	m_gravityMapShader->setUniformArray("colours", &colours[0], EXTRA_PLANET_COUNT + 1);
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

	for (int32 i = 0; i < EXTRA_PLANET_COUNT; ++i)
	{// for each planet
		for (int32 j = 0; j < TARGET_COUNT; ++j)
		{//place 3 targets
			KEntity* pTarget = currentScene->addEntityToScene();
			pTarget->addComponent(new PlanetTarget(j, pTarget, m_extraPlanets[i]));
		}
	}

	return KInitStatus::Success;
}

void LevelSetup::setupPlanetPositionsAndTextures()
{
	const Vec2f screenBounds(KApplication::getApp()->getWindowSize());
	KCollisionDetectionData data;
	constexpr int16 PlanetCollisionLayer = (0xF << 1);
	for (int32 i = 0; i < EXTRA_PLANET_COUNT; ++i)
	{
		KEntity*& pPlanet = m_extraPlanets[i];
		pPlanet->setEntityTag(KTEXT("planet ") + std::to_wstring(i));
		Vec2f pos;
		pos.x = Maths::RandFloat(0.0f, screenBounds.x);
		pos.y = Maths::RandFloat(0.0f, screenBounds.y);
		pPlanet->getComponent<KCTransform>()->setOrigin(PLANET_RADIUS, PLANET_RADIUS);
		pPlanet->getComponent<KCTransform>()->setTranslation(pos);
		KCSprite* const pSprite = pPlanet->getComponent<KCSprite>();

		pSprite->setTexture(m_pPlanetTexture);
		pSprite->setTextureRect(Recti(256, 256, 256, 256));

		pPlanet->getComponent<KCColliderBase>()->setCollisionLayer(PlanetCollisionLayer);
	}

	m_extraPlanets[0]->getComponent<KCTransform>()->setTranslation(Vec2f(134, 97));
	m_extraPlanets[0]->getComponent<StaticPlanetController>()->setPositionToMaintain(Vec2f(134, 97));

	m_extraPlanets[1]->getComponent<KCTransform>()->setTranslation(Vec2f(763, 388));
	m_extraPlanets[1]->getComponent<StaticPlanetController>()->setPositionToMaintain(Vec2f(763, 388));


	m_extraPlanets[2]->getComponent<KCTransform>()->setTranslation(Vec2f(118, 504));
	m_extraPlanets[2]->getComponent<StaticPlanetController>()->setPositionToMaintain(Vec2f(118, 504));

}

void LevelSetup::setupBackgroundTiledmap()
{
	Vec2f screenBounds(KApplication::getApp()->getWindowSize());

	Vec2i gridDim((int32)(screenBounds.x / (float)GRID_NODE_SIZE), (int32)(screenBounds.y / (float)GRID_NODE_SIZE));
	std::vector<int> tileIDs(gridDim.x * gridDim.y, 0);
	m_gravityMapTiledMap.setupTiledMapFromArray(tileIDs, gridDim, Vec2i(GRID_NODE_SIZE, GRID_NODE_SIZE), Vec2i(256, 256));
	m_gravityMapTiledMap.setTexture(m_pBackground);
	m_gravityMapTiledMap.setAllTilesColour(Colour::Green);
	m_gravityMapTiledMap.setShader(m_defaultBackgroundShader);
	KApplication::getApp()->getRenderer()->addTiledMap(0, &m_gravityMapTiledMap);

	KApplication::getApp()->getRenderer()->addTiledMap(-1, &m_backgroundTiledMap);
	gridDim /= 64;

	std::vector<int> tileIDsArray(gridDim.x * gridDim.y, 0);

	for (int i = 0; i < gridDim.x * gridDim.y; ++i)
	{
		tileIDsArray[i] = rand() % 2;
	}
	m_backgroundTiledMap.setupTiledMapFromArray(tileIDs, gridDim, Vec2i(128, 128), Vec2i(256, 256));
	m_backgroundTiledMap.setTexture(m_pBackground);


}
