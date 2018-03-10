#ifndef LEVEL_SETUP_H
#define LEVEL_SETUP_H

#include <Krawler.h>
#include <KComponent.h>

#include <SFML\Graphics\Texture.hpp>
#include <SFML\Graphics\Shader.hpp>

#include "GameBlackboard.h"

#include <TiledMap\KTiledMap.h>

#include <Components\KCPhysicsBody.h>

//#define DEBUG_COLLIDER

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
	void setupBackgroundTiledmap();
	void resolveTestColliderIntersection();

	Krawler::KEntity* m_pPlayerPlanet;

	Krawler::Components::KPhysicsBodyProperties m_planetPhysicsProperties;

	Krawler::KEntity* m_extraPlanets[EXTRA_PLANET_COUNT];

	sf::Texture* m_pPlanetTexture;
	sf::Texture* m_pPlanetTexture2;

	sf::Texture* m_p8BallTexture;
	sf::Texture* m_pBackground;

	sf::Shader* m_gravityMapShader;
	sf::Shader* m_defaultBackgroundShader;

	Krawler::TiledMap::KTiledMap m_gravityMapTiledMap;
	Krawler::TiledMap::KTiledMap m_backgroundTiledMap;

	Krawler::Colour m_planetCols[EXTRA_PLANET_COUNT + 1] =
	{
		Krawler::Colour::Red,
		Krawler::Colour(255, 118, 7),
		Krawler::Colour(52, 255, 7),
		Krawler::Colour(229, 89, 150),
	};

#ifdef DEBUG_COLLIDER
	Krawler::Components::KCColliderBaseCallback m_colliderTestCallback = [this](const Krawler::KCollisionDetectionData& collData) -> void
	{
		resolveTestColliderIntersection();
	};
	Krawler::KEntity* m_pCollisionTest;
	bool m_bColliderTestIntersection = false;
#endif 

	bool m_bShowMap = false;
};

#endif
