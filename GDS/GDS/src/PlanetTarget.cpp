#include "PlanetTarget.h"
#include "GameBlackboard.h"

#include <Input\KInput.h>
#include <AssetLoader\KAssetLoader.h>

#include <Components\KCSprite.h>
#include <Components\KCBoxCollider.h>

using namespace Krawler;
using namespace Krawler::Components;

#define CITY_IMAGE_FULL_HP Recti(0,0,256,256)
#define CITY_IMAGE_HALF_HP Recti(1,0,256,256)
#define CITY_IMAGE_NO_HP   Recti(2,0,256,256)

PlanetTarget::PlanetTarget(int32 targetIndex, KEntity * pTargetEntity, KEntity * pTargetPlanet)
	: m_targetIndex(targetIndex), KComponentBase(pTargetEntity), m_pTargetPlanet(pTargetPlanet)
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
	pTransform->move(PLANET_RADIUS, PLANET_RADIUS);
	pEntity->addComponent(new KCSprite(getEntity(), Vec2f(TARGET_SIZE, TARGET_SIZE)));
	pEntity->addComponent(new KCBoxCollider(getEntity(), Vec2f(TARGET_SIZE, TARGET_SIZE)));

	m_pTargetTexture = KAssetLoader::getAssetLoader().loadTexture(KTEXT("city.png"));

	return Krawler::KInitStatus::Success;
}

void PlanetTarget::onEnterScene()
{
	getEntity()->getComponent<KCColliderBase>()->subscribeCollisionCallback(&m_callback);

	// 
	float angle = static_cast<float>( m_targetIndex * (360 / TARGETS_PER_PLANET));
	Vec2f trans;
	trans.x = cosf(Maths::Radians(angle)) * (PLANET_RADIUS * 1.2f);
	trans.y = sinf(Maths::Radians(angle)) * (PLANET_RADIUS * 1.2f);

	getEntity()->getComponent<KCTransform>()->move(trans);
	getEntity()->getComponent<KCTransform>()->rotate(angle + 90);

	getEntity()->getComponent<KCSprite>()->setTexture(m_pTargetTexture);
	getEntity()->getComponent<KCSprite>()->setTextureRect(CITY_IMAGE_FULL_HP);
}

void PlanetTarget::tick()
{
	if (Input::KInput::JustPressed(Input::KKey::R))
	{
		getEntity()->setIsInUse(true);
	}
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
