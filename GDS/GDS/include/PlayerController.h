#ifndef PLAYER_CONTOLLER_H
#define PLAYER_CONTROLLER_H

#include <KComponent.h>

#include <Components\KCTransform.h>

#include "Projectiles.h"

class PlayerController : public Krawler::KComponentBase
{
public:

	PlayerController(Krawler::KEntity* pEntity, const Krawler::Vec2f& planetCentrePosition, float playerPlanetRadius);
	~PlayerController() = default;

	virtual Krawler::KInitStatus init() override;
	virtual void onEnterScene() override;

	virtual void tick() override;
	virtual void fixedTick() override;

private:

	const float PLANET_RADIUS;
	const Krawler::Vec2f PLANET_CENTRE_POS;

	const float ROTATION_AMOUNT;

	void updateTranslation();
	void powerMeterUpdate();

	Krawler::KInitStatus setupArrow();

	Krawler::Components::KCTransform* m_pTransformComponent;
	Krawler::KInitStatus setupSatellite();

	float m_orbitDistance;
	float m_rotationAngleDegrees;
	ProjectileHandler* m_pProjectileHandler;

	sf::Texture* m_pTankTexture;
	sf::Texture* m_pLauncherTexture;
	sf::Texture* m_pPowerMeterTexture;

	Krawler::KEntity* m_pLauncher;
	Krawler::KEntity* m_pPowerMeter;

	bool m_bSetDrawingArrow = false;
	Krawler::Vec2f m_mousePosA;
	Krawler::Vec2f m_mousePosB;

	float m_powerMeterValue = 0.0f;
	float m_spamTimer = 0.0f;
};

#endif 