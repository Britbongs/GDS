#include "PlanetTarget.h"
#include "GameBlackboard.h"

#include <Components\KCSprite.h>
#include <Components\KCBoxCollider.h>

using namespace Krawler;
using namespace Krawler::Components;

PlanetTarget::PlanetTarget(KEntity * pTargetEntity, KEntity * pTargetPlanet)
	: KComponentBase(pTargetEntity), m_pTargetPlanet(pTargetPlanet)
{

}

Krawler::KInitStatus PlanetTarget::init()
{
	KEntity* pEntity = getEntity();

	pEntity->setEntityTag(KTEXT("target"));
	KCTransform* pTransform = pEntity->getComponent<KCTransform>();
	if (!pTransform)
	{
		KPrintf(KTEXT("Failed to find transform in %s"), __FUNCSIG__);
		return Failure;
	}

	pTransform->setParent(m_pTargetPlanet);
	pTransform->setOrigin(TARGET_SIZE / 2.0f, TARGET_SIZE / 2.0f);
	pEntity->addComponent(new KCSprite(getEntity(), Vec2f(TARGET_SIZE, TARGET_SIZE)));
	pEntity->addComponent(new KCBoxCollider(getEntity(), Vec2f(TARGET_SIZE, TARGET_SIZE)));

	return Krawler::KInitStatus::Success;
}

void PlanetTarget::onEnterScene()
{
	getEntity()->getComponent<KCColliderBase>()->subscribeCollisionCallback(&m_callback);

	// 
	float angle = Maths::RandFloat(0, 360.0f);
	Vec2f trans;
	trans.x = cosf(Maths::Radians(angle)) * PLANET_RADIUS;
	trans.y = sinf(Maths::Radians(angle)) * PLANET_RADIUS;

	getEntity()->getComponent<KCTransform>()->move(trans);
}

void PlanetTarget::handleCollision(const Krawler::KCollisionDetectionData & data)
{
	KEntity* pCollidedWith = nullptr;
	
	if (data.entityA == getEntity())
	{
		pCollidedWith = data.entityB;
	}
	else
	{
		pCollidedWith = data.entityA;
	}

	if (!pCollidedWith)
	{
		return;
	}

	const std::wstring& entityName = pCollidedWith->getEntityTag();

	uint32 findIT = entityName.find(KTEXT("projectile"), 0);

	if (findIT == std::wstring::npos)
	{
		return;
	}

	KPrintf(KTEXT("Projectile hit target!\n"));
	getEntity()->setIsInUse(false);
}
