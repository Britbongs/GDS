#ifndef EDITOR_SETUP_H
#define EDITOR_SETUP_H

#include <array>
#include <KComponent.h>
#include <KEntity.h>
#include <SFML\Graphics\Texture.hpp>

#include "LevelData.h"

#define MAX_PLANETS_PER_LEVEL 6
#define MAX_TARGETS_PER_LEVEL MAX_PLANETS_PER_LEVEL * 3

class EditorSetup :
	public Krawler::KComponentBase
{
public:

	EditorSetup(Krawler::KEntity* pEntity);
	~EditorSetup();

	virtual Krawler::KInitStatus init() override;
	virtual void onEnterScene() override;
	virtual void tick() override;

private:

	enum EntityPlacingType
	{
		ExtraPlanets,
		Targets,
		PlayerPlanet
	};

	Krawler::KInitStatus setupInLevelPlanetsArray();
	Krawler::KInitStatus setupPlayerPlanet();

	void updateInUseEntities();
	void updateByPlacingType();
	void writeLevelToFile();
	void updateTargetPlacing(int highlightPlanetIndex);
	void switchPlacingType(EntityPlacingType type);

	InLevelPlanet m_inLevelPlanets[MAX_PLANETS_PER_LEVEL];
	InLevelPlanet m_playerPlanet;

	Krawler::KEntity* m_pPlanetHighlight = nullptr;
	Krawler::KEntity* m_pPlaceHolderTarget = nullptr;

	sf::Texture* m_pPlanetTexture = nullptr;
	sf::Texture* m_pPlayerPlanetTexture = nullptr;
	sf::Texture* m_pTargetTexture = nullptr;

	Krawler::int32 m_nextAvailablePlanetIdx = 0;


	EntityPlacingType m_placingType = EntityPlacingType::ExtraPlanets;
};

#endif 
