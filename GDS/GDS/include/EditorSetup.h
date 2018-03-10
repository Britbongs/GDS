#ifndef EDITOR_SETUP_H
#define EDITOR_SETUP_H

#include <array>
#include <KComponent.h>
#include <SFML\Graphics\Texture.hpp>

#define MAX_PLANETS_PER_LEVEL 6
#define MAX_TARGETS_PER_LEVEL MAX_PLANETS_PER_LEVEL * 3



class EditorSetup : public Krawler::KComponentBase
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
	
	void switchPlacingType(EntityPlacingType type);

	struct InLevelPlanet
	{
		Krawler::KEntity* pPlanet = nullptr;
		bool bPlacedInScene = false;
	};

	InLevelPlanet m_inLevelPlanets[MAX_PLANETS_PER_LEVEL];
	InLevelPlanet m_playerPlanet; 

	sf::Texture* m_pPlanetTexture = nullptr;
	sf::Texture* m_pPlayerPlanetTexture = nullptr;
	Krawler::int32 m_nextAvailablePlanetIdx = 0;


	EntityPlacingType m_placingType = EntityPlacingType::ExtraPlanets;
};

#endif 
