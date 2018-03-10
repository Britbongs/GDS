#include "EditorSetup.h"
#include "GameBlackboard.h"

#include <KApplication.h>
#include <AssetLoader\KAssetLoader.h>
#include <Input\KInput.h>
#include <Components\KCSprite.h>

using namespace Krawler;
using namespace Krawler::Input;
using namespace Krawler::Components;

EditorSetup::EditorSetup(Krawler::KEntity * pEntity)
	: KComponentBase(pEntity)
{

}

EditorSetup::~EditorSetup()
{
}

KInitStatus EditorSetup::init()
{
	KAssetLoader::getAssetLoader().setRootFolder(KTEXT("res\\"));
	KINIT_CHECK(setupInLevelPlanetsArray());
	KINIT_CHECK(setupPlayerPlanet());

	return KInitStatus::Success;
}

void EditorSetup::onEnterScene()
{
	for (auto& iLP : m_inLevelPlanets)
	{
		iLP.pPlanet->getComponent<KCSprite>()->setTexture(m_pPlanetTexture);
	}
	m_playerPlanet.pPlanet->getComponent<KCSprite>()->setTexture(m_pPlayerPlanetTexture);

}

void EditorSetup::tick()
{
	updateInUseEntities();
	updateByPlacingType();

	if (KInput::JustPressed(KKey::P)) // player
	{
		switchPlacingType(PlayerPlanet);
	}

	if (KInput::JustPressed(KKey::E)) // extra planets
	{
		switchPlacingType(ExtraPlanets);
	}

	//if (KInput::JustPressed(KKey::T)) // Targets 
	//{
	//	switchPlacingType(ExtraPlanets);
	//}

	//clear all planets
	if (KInput::JustPressed(KKey::C))
	{
		for (auto& data : m_inLevelPlanets)
			data.bPlacedInScene = false;
		m_playerPlanet.bPlacedInScene = false;

		m_nextAvailablePlanetIdx = 0;
	}
}

KInitStatus EditorSetup::setupInLevelPlanetsArray()
{
	KApplication* const pApp = KApplication::getApp();

	int32 allocated = 0;
	KEntity* pEntities = pApp->getCurrentScene()->addEntitiesToScene(MAX_PLANETS_PER_LEVEL, allocated);

	KCHECK(pEntities);
	if (allocated != MAX_PLANETS_PER_LEVEL || pEntities == nullptr)
	{
		KPrintf(KTEXT("Error!:\n Unable to allocate planets in editor! %ws"), __FUNCTIONW__);
		return KInitStatus::Failure;
	}

	for (int32 i = 0; i < MAX_PLANETS_PER_LEVEL; ++i)
	{
		m_inLevelPlanets[i].pPlanet = &pEntities[i];
		m_inLevelPlanets[i].pPlanet->addComponent(new KCSprite(m_inLevelPlanets[i].pPlanet, Vec2f(2 * PLANET_RADIUS, 2 * PLANET_RADIUS)));
		m_inLevelPlanets[i].pPlanet->setEntityTag(KTEXT("Planet_") + std::to_wstring(i));
		m_inLevelPlanets[i].pPlanet->setIsInUse(false);
		m_inLevelPlanets[i].pPlanet->getComponent<KCTransform>()->setOrigin(Vec2f(PLANET_RADIUS, PLANET_RADIUS));

	}

	m_pPlanetTexture = KAssetLoader::getAssetLoader().loadTexture(KTEXT("planet_1.png"));

	if (!m_pPlanetTexture)
	{
		KPrintf(KTEXT("Error!:\n Unable to load planet texture in EditorSetup"));
		return KInitStatus::MissingResource;
	}

	return KInitStatus::Success;
}

KInitStatus EditorSetup::setupPlayerPlanet()
{
	KApplication* const pApp = KApplication::getApp();

	m_playerPlanet.pPlanet = pApp->getCurrentScene()->addEntityToScene();

	KCHECK(m_playerPlanet.pPlanet);

	if (!m_playerPlanet.pPlanet)
	{
		KPrintf(KTEXT("Error:\n Unable to create entity for player planet in editorsetup!\n"));
		return KInitStatus::Nullptr;
	}

	m_playerPlanet.pPlanet->addComponent(new KCSprite(m_playerPlanet.pPlanet, Vec2f(PLANET_RADIUS * 2.0f, PLANET_RADIUS * 2.0f)));
	m_playerPlanet.pPlanet->getComponent<KCTransform>()->setOrigin(Vec2f(PLANET_RADIUS, PLANET_RADIUS));

	m_pPlayerPlanetTexture = KAssetLoader::getAssetLoader().loadTexture(KTEXT("planet_2.png"));

	if (!m_pPlanetTexture)
	{
		KPrintf(KTEXT("Error!:\n Unable to load player planet texture in EditorSetup"));
		return KInitStatus::MissingResource;
	}
	return KInitStatus::Success;
}

void EditorSetup::updateInUseEntities()
{
	if (m_playerPlanet.bPlacedInScene && !m_playerPlanet.pPlanet->isEntityInUse())
	{
		m_playerPlanet.pPlanet->setIsInUse(true);
	}
	else if (!m_playerPlanet.bPlacedInScene && m_playerPlanet.pPlanet->isEntityInUse())
	{
		m_playerPlanet.pPlanet->setIsInUse(false);
	}

	for (auto& inLvlPlnt : m_inLevelPlanets)
	{
		if (inLvlPlnt.bPlacedInScene && !inLvlPlnt.pPlanet->isEntityInUse())
		{
			inLvlPlnt.pPlanet->setIsInUse(true);
		}
		else if (!inLvlPlnt.bPlacedInScene && inLvlPlnt.pPlanet->isEntityInUse())
		{
			inLvlPlnt.pPlanet->setIsInUse(false);
		}
	}
}

void EditorSetup::updateByPlacingType()
{
	switch (m_placingType)
	{
	case EntityPlacingType::ExtraPlanets:
	{
		if (KInput::MouseJustPressed(KMouseButton::Left) && m_nextAvailablePlanetIdx < MAX_PLANETS_PER_LEVEL)
		{
			m_inLevelPlanets[m_nextAvailablePlanetIdx].bPlacedInScene = true;
			m_inLevelPlanets[m_nextAvailablePlanetIdx].pPlanet->getComponent<KCTransform>()->setTranslation(KInput::GetMouseWorldPosition());
			++m_nextAvailablePlanetIdx;
		}

		if (KInput::Pressed(KKey::LControl) && KInput::JustPressed(KKey::Z))
		{
			if (m_nextAvailablePlanetIdx > 0)
			{
				--m_nextAvailablePlanetIdx;
				m_inLevelPlanets[m_nextAvailablePlanetIdx].bPlacedInScene = false;
			}
		}
	}
	break;


	case EntityPlacingType::PlayerPlanet:
	{
		if (KInput::MouseJustPressed(KMouseButton::Left))
		{
			m_playerPlanet.bPlacedInScene = true;
			m_playerPlanet.pPlanet->getComponent<KCTransform>()->setTranslation(KInput::GetMouseWorldPosition());
		}

		if (KInput::Pressed(KKey::LControl) && KInput::JustPressed(KKey::Z))
		{
			m_playerPlanet.bPlacedInScene = false;
		}

	}
	break;

	}
}

void EditorSetup::switchPlacingType(EntityPlacingType type)
{
	switch (type)
	{

	}

	m_placingType = type;
}


