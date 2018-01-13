#ifndef PROJECTILES_H
#define PROJECTILES_H

#include "GameBlackboard.h"

#include <vector>

#include <Krawler.h>
#include <KComponent.h>

#include <Components\KCTransform.h>
#include <Components\KCPhysicsBody.h>
#include <Components\KCColliderBase.h>

#include <SFML\Graphics\Texture.hpp>

class Projectile : public Krawler::KComponentBase
{
public:

	Projectile(Krawler::KEntity* pEntity, Krawler::int32 index);
	~Projectile() = default;

	virtual Krawler::KInitStatus init() override;
	virtual void tick() override;
	virtual void onEnterScene() override;

	void resetProjectile();

private:

	sf::Texture* m_p8ballTexture;
	float m_aliveTime = 0.0f;

	static Krawler::Components::KPhysicsBodyProperties m_physicsProps;
	Krawler::Components::KCColliderBaseCallback m_collCallback = [this](const Krawler::KCollisionDetectionData& data) -> void
	{
		resetProjectile();
	};
};

class ProjectileHandler : public Krawler::KComponentBase
{
public:

	ProjectileHandler(Krawler::KEntity* pEntity, const std::vector<Krawler::KEntity*>& projectilesVec);
	~ProjectileHandler() = default;

	virtual void onEnterScene() override;
	virtual void tick() override;

	void fireProjectile(const Krawler::Vec2f& startPos, const Krawler::Vec2f& direction);

private:

	Krawler::KEntity* getProjectileToFire();

	std::vector<Krawler::KEntity*> m_projectilesVec;
};

#endif
