#ifndef LEVEL_SETUP_H
#define LEVEL_SETUP_H

#include <Krawler.h>
#include <KComponent.h>

#include <SFML\Graphics\Texture.hpp>

#include "GameBlackboard.h"
#include <Components\KCPhysicsBody.h>

class LevelSetup : public Krawler::KComponentBase
{
public:

	LevelSetup(Krawler::KEntity* pEntity);
	~LevelSetup() = default;

	virtual Krawler::KInitStatus init() override;
	virtual void tick() override;
	virtual void onEnterScene() override;

private:

	constexpr static Krawler::int32 EXTRA_PLANET_COUNT = 3;

	Krawler::KInitStatus setupPlayerEntities();
	Krawler::KInitStatus addProjectiles();
	Krawler::KInitStatus createExtraPlanets();

	void setupPlanetPositionsAndTextures();

	Krawler::KEntity* m_pPlayerPlanet;

	Krawler::Components::KPhysicsBodyProperties m_planetPhysicsProperties;
	Krawler::KEntity* m_extraPlanets[EXTRA_PLANET_COUNT];

	sf::Texture* m_pPlanetTexture;
	sf::Texture* m_p8BallTexture;
};

#endif
