#ifndef EDITOR_SETUP_H
#define EDITOR_SETUP_H

#include <array>
#include <KComponent.h>
#include <SFML\Graphics\Texture.hpp>

#define MAX_PLANETS_PER_LEVEL 6

class EditorSetup : public Krawler::KComponentBase
{
public:

	EditorSetup(Krawler::KEntity* pEntity);
	~EditorSetup();

	virtual Krawler::KInitStatus init() override;
	virtual void onEnterScene() override;
	virtual void tick() override;

private:

	Krawler::KInitStatus setupInLevelPlanetsArray();

	void updateInUseEntities();

	struct InLevelPlanet
	{
		Krawler::KEntity* pPlanet = nullptr;
		bool bPlacedInScene = false;
	};

	InLevelPlanet m_inLevelPlanets[MAX_PLANETS_PER_LEVEL];

	sf::Texture* m_pPlanetTexture;
	Krawler::int32 m_nextAvailablePlanetIdx = 0;

};

#endif 
