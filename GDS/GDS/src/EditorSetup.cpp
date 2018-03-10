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

	return KInitStatus::Success;
}

void EditorSetup::onEnterScene()
{
	for (auto& iLP : m_inLevelPlanets)
	{
		iLP.pPlanet->getComponent<KCSprite>()->setTexture(m_pPlanetTexture);
	}
}

void EditorSetup::tick()
{
	updateInUseEntities();
	if (KInput::MouseJustPressed(KMouseButton::Left) && m_nextAvailablePlanetIdx < MAX_PLANETS_PER_LEVEL)
	{
		m_inLevelPlanets[m_nextAvailablePlanetIdx].bPlacedInScene = true;
		m_inLevelPlanets[m_nextAvailablePlanetIdx].pPlanet->getComponent<KCTransform>()->setTranslation(KInput::GetMouseWorldPosition());
		++m_nextAvailablePlanetIdx;
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

void EditorSetup::updateInUseEntities()
{
	for (auto& inLvlPlnt : m_inLevelPlanets)
	{
		if (inLvlPlnt.bPlacedInScene && !inLvlPlnt.pPlanet->isEntityInUse())
		{
			inLvlPlnt.pPlanet->setIsInUse(true);
		}
	}
}

