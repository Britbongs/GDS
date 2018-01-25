#include "Projectiles.h"

#include <KApplication.h>
#include <AssetLoader\KAssetLoader.h>

#include <Components\KCCircleCollider.h>
#include <Components\KCSprite.h>

using namespace Krawler;
using namespace Krawler::Components;

//init static var
/* -- Projectile -- */
KPhysicsBodyProperties Projectile::m_physicsProps = KPhysicsBodyProperties{ PROJECTILE_MASS, 0.3f, 0.3f, 0.1f };

Projectile::Projectile(KEntity * pEntity, int32 index)
	: KComponentBase(pEntity), m_p8ballTexture(nullptr)
{
	setComponentTag(KTEXT("projectile " + std::to_wstring(index)));
	pEntity->setEntityTag(KTEXT("entity projectile ") + std::to_wstring(index));
}

KInitStatus Projectile::init()
{
	KEntity* const pEntity = getEntity();

	pEntity->addComponent(new KCPhysicsBody(pEntity, m_physicsProps));
	pEntity->addComponent(new KCCircleCollider(pEntity, PROJECTILE_RADIUS));
	pEntity->addComponent(new KCSprite(pEntity, Vec2f(PROJECTILE_RADIUS * 2.0f, PROJECTILE_RADIUS * 2.0f)));
	pEntity->getComponent<KCTransform>()->setOrigin(Vec2f(PROJECTILE_RADIUS, PROJECTILE_RADIUS));
	KAssetLoader& rAsset = KAssetLoader::getAssetLoader();

	m_p8ballTexture = rAsset.loadTexture(KTEXT("asteroid.png"));
	if (!m_p8ballTexture)
	{
		KPrintf(KTEXT("Missing asteroid texture for projectile!\n"));
		return KInitStatus::MissingResource;
	}

	return KInitStatus::Success;
}

void Projectile::tick()
{
	m_aliveTime += KApplication::getApp()->getDeltaTime();

	if (m_aliveTime > PROJECTILE_ALIVE_TIME)
	{
		resetProjectile();
	}
}

void Projectile::onEnterScene()
{
	getEntity()->getComponent<KCSprite>()->setTexture(m_p8ballTexture);
	getEntity()->getComponent<KCColliderBase>()->subscribeCollisionCallback(&m_collCallback);
}

void Projectile::resetProjectile()
{
	m_aliveTime = 0.0f;
	getEntity()->setIsInUse(false);
	KCPhysicsBody* pPhysBody = getEntity()->getComponent<KCPhysicsBody>();
	pPhysBody->setVelocity(Vec2f(0.0f, 0.0f));
	pPhysBody->applyForce(-pPhysBody->getForce());
}

void Projectile::handleCollision(const Krawler::KCollisionDetectionData & data)
{
	KEntity *pCollidedWith = nullptr;

	if (data.entityA == getEntity())
	{
		pCollidedWith = data.entityB;
	}
	else
	{
		pCollidedWith = data.entityA;
	}

	const std::wstring& entityName = pCollidedWith->getEntityTag();

	uint32 findResult = entityName.find(KTEXT("projectile"));
	if (findResult != std::wstring::npos)
	{
		return;
	}

	resetProjectile();
}

/* -- Projectile Handler -- */

ProjectileHandler::ProjectileHandler(Krawler::KEntity * pEntity, const std::vector<KEntity*>& projectilesVec)
	: KComponentBase(pEntity), m_projectilesVec(projectilesVec)
{
}

void ProjectileHandler::onEnterScene()
{
	for (auto& pProjectileEntity : m_projectilesVec)
	{
		pProjectileEntity->setIsInUse(false);
	}
}

void ProjectileHandler::tick()
{
}

void ProjectileHandler::fireProjectile(const Vec2f& startPos, const Vec2f& direction)
{
	KEntity* pProjectile = getProjectileToFire();
	if (pProjectile == nullptr)
	{
		return;
	}
	pProjectile->setIsInUse(true);
	pProjectile->getComponent<KCTransform>()->setTranslation(startPos);
	pProjectile->getComponent<KCPhysicsBody>()->applyForce(direction * KICKOFF_FORCE);
}

KEntity* ProjectileHandler::getProjectileToFire()
{
	auto result = std::find_if(m_projectilesVec.begin(), m_projectilesVec.end(), [](KEntity* pEntity) -> bool
	{
		return !pEntity->isEntitiyInUse();
	});

	if (result == m_projectilesVec.end())
	{
		return nullptr;
	}
	return *result;
}
