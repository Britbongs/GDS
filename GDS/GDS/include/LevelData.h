#ifndef LEVEL_DATA_H

#include <KEntity.h>

struct InLevelTarget
{
	float angle = 0.0f;
	Krawler::KEntity* pTargetEntity = nullptr;
};

struct InLevelPlanet
{
	Krawler::KEntity* pPlanet = nullptr;
	std::vector<InLevelTarget> planetTargets;
	bool bPlacedInScene = false;
};


#endif // !LEVEL_DATA_H

