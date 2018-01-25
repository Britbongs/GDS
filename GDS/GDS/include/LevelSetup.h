#ifndef LEVEL_SETUP_H
#define LEVEL_SETUP_H

#include <Krawler.h>
#include <KComponent.h>

#include <SFML\Graphics\Texture.hpp>
#include <SFML\Graphics\Shader.hpp>

#include "GameBlackboard.h"

#include <TiledMap\KTiledMap.h>

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


	Krawler::KInitStatus setupPlayerEntities();
	Krawler::KInitStatus addProjectiles();
	Krawler::KInitStatus createExtraPlanets();

	void setupPlanetPositionsAndTextures();

	Krawler::KEntity* m_pPlayerPlanet;

	Krawler::Components::KPhysicsBodyProperties m_planetPhysicsProperties;

	Krawler::KEntity* m_extraPlanets[EXTRA_PLANET_COUNT];

	sf::Texture* m_pPlanetTexture;
	sf::Texture* m_p8BallTexture;
	sf::Texture* m_pBackground;

	sf::Shader* m_gravityMapShader;
	sf::Shader* m_defaultBackgroundShader;
	Krawler::TiledMap::KTiledMap m_tiledMap;

	Krawler::Colour m_planetCols[EXTRA_PLANET_COUNT + 1] =
	{
		Krawler::Colour(rand() % 256, rand() % 256,rand() % 256),
		Krawler::Colour(rand() % 256, rand() % 256,rand() % 256),
		Krawler::Colour(rand() % 256, rand() % 256,rand() % 256),
		Krawler::Colour(rand() % 256, rand() % 256,rand() % 256)
	};

	bool m_bShowMap = false;
};

#endif
