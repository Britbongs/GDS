#include "EditorSetup.h"
#include "GameBlackboard.h"

#include <fstream>
#include <string>

#include <KApplication.h>
#include <AssetLoader\KAssetLoader.h>
#include <Input\KInput.h>
#include <Components\KCSprite.h>

using namespace std;

using namespace Krawler;
using namespace Krawler::Input;
using namespace Krawler::Components;

#define LEVEL_FILE_EXT KTEXT(".dat")
#define CITY_IMAGE_FULL_HP Recti(0,0,256,256)

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

	m_pPlanetHighlight = KApplication::getApp()->getCurrentScene()->addEntityToScene();
	KCHECK(m_pPlanetHighlight);
	m_pPlaceHolderTarget = KApplication::getApp()->getCurrentScene()->addEntityToScene();
	KCHECK(m_pPlaceHolderTarget);



	m_pPlanetHighlight->addComponent(new KCSprite(m_pPlanetHighlight, Vec2f(PLANET_RADIUS * 3, PLANET_RADIUS * 3)));
	m_pPlanetHighlight->setIsInUse(false);


	m_pPlaceHolderTarget->addComponent(new KCSprite(m_pPlaceHolderTarget, Vec2f(TARGET_SIZE, TARGET_SIZE)));
	m_pPlaceHolderTarget->setIsInUse(false);
	m_pPlaceHolderTarget->setEntityTag(KTEXT("PLACEHOLDER"));

	m_pTargetTexture = KAssetLoader::getAssetLoader().loadTexture(KTEXT("city.png"));

	return KInitStatus::Success;
}

void EditorSetup::onEnterScene()
{
	for (auto& iLP : m_inLevelPlanets)
	{
		iLP.pPlanet->getComponent<KCSprite>()->setTexture(m_pPlanetTexture);
	}
	m_playerPlanet.pPlanet->getComponent<KCSprite>()->setTexture(m_pPlayerPlanetTexture);

	//Setup target-placement-planet-highlight (the yellow ring around a planet when you're placing a target)
	KCSprite* const pPlanetHighlightSprite = m_pPlanetHighlight->getComponent<KCSprite>();
	pPlanetHighlightSprite->setTexture(m_pPlanetTexture);
	pPlanetHighlightSprite->setColour(Colour::Yellow);
	pPlanetHighlightSprite->setRenderLayer(-5);
	m_pPlanetHighlight->getTransformComponent()->setOrigin(PLANET_RADIUS * 1.5f, PLANET_RADIUS * 1.5f);

	//Setup placeholder target 
	KCSprite* const pPlaceholderTargetSprite = m_pPlaceHolderTarget->getComponent<KCSprite>();
	pPlaceholderTargetSprite->setTexture(m_pTargetTexture);
	pPlaceholderTargetSprite->setTextureRect(CITY_IMAGE_FULL_HP);
	m_pPlaceHolderTarget->getTransformComponent()->setOrigin(TARGET_SIZE / 2.0f, TARGET_SIZE / 2.0f);

}

void EditorSetup::tick()
{
	static int highlightPlanetIndex = 0;

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

	if (KInput::JustPressed(KKey::T)) // Targets 
	{
		switchPlacingType(Targets);
		highlightPlanetIndex = 0;
	}

	if (m_placingType == Targets)
	{
		updateTargetPlacing(highlightPlanetIndex);
	}

	//clear all planets
	if (KInput::JustPressed(KKey::C))
	{
		for (auto& data : m_inLevelPlanets)
			data.bPlacedInScene = false;
		m_playerPlanet.bPlacedInScene = false;

		m_nextAvailablePlanetIdx = 0;
	}

	if (KInput::Pressed(KKey::LControl) && KInput::JustPressed(KKey::S))
	{
		writeLevelToFile();
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
		m_inLevelPlanets[i].pPlanet->getTransformComponent()->setOrigin(Vec2f(PLANET_RADIUS, PLANET_RADIUS));

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

void EditorSetup::writeLevelToFile()
{
	wstring fileName;
	KPrintf(KTEXT("Please Provide a file name: \n"));
	wcin >> fileName;

	wofstream levelFile;
	levelFile.open(fileName + LEVEL_FILE_EXT, ios::out);
	if (levelFile.fail())
	{
		KPrintf(KTEXT("Error! Unable to open file for writing level\n"));
		return;
	}

	/*
	-player planet position
	-extra planet count
	-extra planet positions
	*/

	Vec2f playerPos = m_playerPlanet.pPlanet->getComponent<KCTransform>()->getPosition();
	Vec2f centrePos;
	levelFile << std::to_wstring(playerPos.x) << KTEXT(" ") << std::to_wstring(playerPos.y) << KTEXT('\n');

	levelFile << m_nextAvailablePlanetIdx << KTEXT('\n'); // = total number allocated

	for (auto& iLP : m_inLevelPlanets)
	{
		if (!iLP.bPlacedInScene)
		{
			continue;
		}

		centrePos = iLP.pPlanet->getComponent<KCTransform>()->getPosition();
		levelFile << centrePos.x << KTEXT(" ") << centrePos.y << KTEXT('\n');
	}
	levelFile.close();
}

void EditorSetup::updateTargetPlacing(int highlightPlanetIndex)
{
	if (m_inLevelPlanets[0].bPlacedInScene)
	{
		if (KInput::JustPressed(KKey::Up))
		{
			if (highlightPlanetIndex + 1 < MAX_PLANETS_PER_LEVEL)
			{
				if (m_inLevelPlanets[highlightPlanetIndex + 1].bPlacedInScene)
				{
					highlightPlanetIndex = highlightPlanetIndex + 1;
				}
				const Vec2f position = m_inLevelPlanets[highlightPlanetIndex].pPlanet->getComponent<KCTransform>()->getPosition();
				m_pPlanetHighlight->getComponent<KCTransform>()->setTranslation(position);
				m_pPlaceHolderTarget->getComponent<KCTransform>()->setParent(m_inLevelPlanets[highlightPlanetIndex].pPlanet);

			}
		}

		if (KInput::JustPressed(KKey::Down))
		{
			if (highlightPlanetIndex - 1 >= 0)
			{
				if (m_inLevelPlanets[highlightPlanetIndex - 1].bPlacedInScene)
				{
					highlightPlanetIndex = highlightPlanetIndex - 1;
				}
			}
			const Vec2f position = m_inLevelPlanets[highlightPlanetIndex].pPlanet->getTransformComponent()->getPosition();
			m_pPlanetHighlight->getTransformComponent()->setTranslation(position);
			m_pPlaceHolderTarget->getTransformComponent()->setParent(m_inLevelPlanets[highlightPlanetIndex].pPlanet);

		}
	}

	//Below will set the planet target to correct position around planet based on a rotation angle
	KCTransform* const pHighlightPlanetTransform = m_inLevelPlanets[highlightPlanetIndex].pPlanet->getTransformComponent();
	KCTransform* const pPlaceHolderTransform = m_pPlaceHolderTarget->getTransformComponent();

	const Vec2f planetToMouse = KInput::GetMouseWorldPosition() - pHighlightPlanetTransform->getPosition();
	const float THETA = atan2(planetToMouse.y, planetToMouse.x);

	Vec2f trans;
	trans.x = cosf(THETA) * (PLANET_RADIUS * 1.2f);
	trans.y = sinf(THETA) * (PLANET_RADIUS * 1.2f);
	pPlaceHolderTransform->setTranslation(trans);

	KScene* const pCurrentScene = KApplication::getApp()->getCurrentScene();

	if (KInput::MouseJustPressed(KMouseButton::Left)) //add new target to scene by creating a new entity and copying the renderable element and transform
	{
		KEntity* const pEntity = pCurrentScene->addEntityToScene();
		KCSprite* spriteToAdd = new KCSprite(pEntity, Vec2f(TARGET_SIZE, TARGET_SIZE));
		pEntity->addComponent(spriteToAdd); //copy sprite component from the placeholder
		(*spriteToAdd) = *m_pPlaceHolderTarget->getComponent<KCSprite>();
		(*pEntity->getTransformComponent()) = (*m_pPlaceHolderTarget->getTransformComponent());
		pEntity->setEntityTag(KTEXT("In Level Target"));
		m_inLevelPlanets[highlightPlanetIndex].planetTargets.push_back(InLevelTarget{ THETA, pEntity });
	}
}

void EditorSetup::switchPlacingType(EntityPlacingType type)
{
	m_placingType = type;
	switch (type)
	{
	case Targets:
	{
		m_pPlanetHighlight->setIsInUse(true);
		const Vec2f position = m_inLevelPlanets[0].pPlanet->getComponent<KCTransform>()->getPosition();
		m_pPlanetHighlight->getComponent<KCTransform>()->setTranslation(position);

		m_pPlaceHolderTarget->setIsInUse(true);
		m_pPlaceHolderTarget->getComponent<KCTransform>()->setParent(m_inLevelPlanets[0].pPlanet);
	}
	break;
	default:
		m_pPlanetHighlight->setIsInUse(false);
		m_pPlaceHolderTarget->setIsInUse(false);

		break;
	}
}
