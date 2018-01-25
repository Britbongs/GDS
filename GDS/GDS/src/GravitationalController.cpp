#include "GravitationalController.h"

#include "GameBlackboard.h"

#include <KApplication.h>
#include <Components\KCPhysicsBody.h>
#include <Physics\KPhysicsWorld.h>

using namespace Krawler;
using namespace Krawler::Components;

/*--Static Planet Controller--*/

StaticPlanetController::StaticPlanetController(KEntity * pEntity, Vec2f positionToMaintain)
	: KComponentBase(pEntity), m_positionToMaintain(positionToMaintain)
{
}

void StaticPlanetController::fixedTick()
{
	getEntity()->getComponent<KCTransform>()->setTranslation(m_positionToMaintain);
	getEntity()->getComponent<KCPhysicsBody>()->setVelocity(Vec2f(0.0f, 0.0f));
}

/*--Gravitational Controller--*/

GravitationalController::GravitationalController(Krawler::KEntity * pEntity)
	: KComponentBase(pEntity)
{
}

void GravitationalController::tick()
{
	assembleList();
}

void GravitationalController::fixedTick()
{
	KCTransform* pTransA, *pTransB;
	KCPhysicsBody* pPhysA, *pPhysB;
	KEntity* pEntityA, *pEntityB;
	auto metresToPixels = KApplication::getApp()->getPhysicsWorld()->getPhysicsWorldProperties().metresToPixels;

	for (auto itI = m_gravityInteractingEntities.begin(); itI != m_gravityInteractingEntities.end(); ++itI) //iterator I 
	{
		for (auto itJ = itI + 1; itJ != m_gravityInteractingEntities.end(); ++itJ) // iterator j
		{
			if (*itI == *itJ)
			{
				continue;
			}
			pEntityA = *itI;
			pEntityB = *itJ;

			pTransA = pEntityA->getComponent<KCTransform>();
			pTransB = pEntityB->getComponent<KCTransform>();

			pPhysA = pEntityA->getComponent<KCPhysicsBody>();
			pPhysB = pEntityB->getComponent<KCPhysicsBody>();

			const Rectf boundsA = pEntityA->getComponent<KCSprite>()->getOnscreenBounds();
			const Rectf boundsB = pEntityB->getComponent<KCSprite>()->getOnscreenBounds();

			const float massA = pPhysA->getPhysicsBodyProperties()->mass;
			const float massB = pPhysB->getPhysicsBodyProperties()->mass;

			const Vec2f centrePosA(boundsA.left + boundsA.width / 2.0f, boundsA.top + boundsA.height / 2.0f);
			const Vec2f centrePosB(boundsB.left + boundsB.width / 2.0f, boundsB.top + boundsB.height / 2.0f);

			Vec2f directionAtoB = centrePosB - centrePosA;
			directionAtoB *= metresToPixels;

			float force = GRAVITATIONAL_CONSTANT*massA * massB / GetSquareLength(directionAtoB);
			force *= GRATIVTY_BALANCE;

			directionAtoB = Normalise(directionAtoB);
			directionAtoB *= force;

			pPhysA->applyForce(directionAtoB);
			pPhysB->applyForce(-directionAtoB);

		}
	}
}

void GravitationalController::assembleList()
{
	auto pCurrentScene = KApplication::getApp()->getCurrentScene();
	m_gravityInteractingEntities.clear();
	KEntity* pEntityList = pCurrentScene->getEntitiyList();
	for (int32 i = 0; i < (signed)pCurrentScene->getNumbrOfEntitiesAllocated(); ++i)
	{
		KEntity* pEntity = &pEntityList[i];
		if (!pEntity->isEntitiyInUse())
		{
			continue;
		}

		KCPhysicsBody* pPhys = pEntity->getComponent<KCPhysicsBody>();

		if (!pPhys)
		{
			continue;
		}
		m_gravityInteractingEntities.push_back(pEntity);
	}
}
