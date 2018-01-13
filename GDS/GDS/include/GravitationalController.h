#ifndef GRAVITATIONAL_CONTROLLER_H
#define GRAVITATIONAL_CONTROLLER_H

#include <Krawler.h>
#include <KComponent.h>
#include <KEntity.h>


class StaticPlanetController : public Krawler::KComponentBase
{
public:

	StaticPlanetController(Krawler::KEntity* pEntity, Krawler::Vec2f positionToMaintain);
	~StaticPlanetController() = default;

	virtual void fixedTick() override;
	void setPositionToMaintain(const Krawler::Vec2f& vec) { m_positionToMaintain = vec; }
private:

	Krawler::Vec2f m_positionToMaintain;
};


class GravitationalController : public Krawler::KComponentBase
{
public:

	GravitationalController(Krawler::KEntity* pEntity);
	~GravitationalController() = default;

	virtual void tick() override;
	virtual void fixedTick() override;

private:

	void assembleList();

	std::vector<Krawler::KEntity*> m_gravityInteractingEntities; //entities which can be influenced by gravity

};

#endif