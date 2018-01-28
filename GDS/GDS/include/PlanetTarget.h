#ifndef PLANET_TARGET_H
#define PLANET_TARGET_H

#include <KComponent.h>
#include <Components\KCTransform.h>
#include <Components\KCColliderBase.h>

#include <SFML\Graphics\Texture.hpp>

class PlanetTarget : public Krawler::KComponentBase
{
public:

	PlanetTarget(Krawler::int32 targetIndex, Krawler::KEntity* pTargetEntity, Krawler::KEntity* pTargetPlanet);
	~PlanetTarget() = default;

	virtual Krawler::KInitStatus init() override;
	virtual void onEnterScene() override;
	virtual void tick() override;

private:

	Krawler::KEntity* m_pTargetPlanet;

	void handleCollision(const Krawler::KCollisionDetectionData& data);

	Krawler::Components::KCColliderBaseCallback m_callback = [this](const Krawler::KCollisionDetectionData& data) -> void
	{
		handleCollision(data);
	};
	sf::Texture* m_pTargetTexture;

	Krawler::int32 m_targetIndex;
};

#endif 