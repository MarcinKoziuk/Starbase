#include <cmath>

#include <starbase/game/logging.hpp>
#include <starbase/game/entity/entity.hpp>
#include <starbase/game/component/transform.hpp>

#include <starbase/game/system/shipcontrols_system.hpp>
#include <starbase/game/chipmunk_safe.hpp>

namespace Starbase {

static inline void
cpBodyApplyTorque(cpBody *body, cpFloat torque)
{
	cpVect g = cpBodyGetPosition(body);
	cpVect c = cpBodyGetCenterOfGravity(body);
	cpBodyApplyImpulseAtLocalPoint(body, cpv(0.0, torque), cpv(1.0 + c.x, c.y));
	cpBodyApplyImpulseAtLocalPoint(body, cpv(0.0, -torque), cpv(-1.0 + c.x, c.y));
}

void ShipControlsSystem::SpawnBullet(int step, const id_t spaceId, const glm::vec2& pos, const glm::vec2& vel)
{
	ResourcePtr<Model> model = m_resourceLoader.Load<Model>(ID("models/bullets/bullet-0"));
	ResourcePtr<Body> body = m_resourceLoader.Load<Body>(ID("models/bullets/bullet-0"));

	Transform transf;
	transf.pos = pos;
	//transf.rot = rot;
	transf.scale = glm::vec2(10.f, 10.f);
	transf.vel = vel;

    m_em.CreateEntity<Transform, Renderable, Physics, AutoDestruct>(
        Transform(transf),
		Renderable{model},
		Physics{spaceId, body},
		AutoDestruct(step, 400)
	);

	/*
    std::tuple<Entity&, Transform&> bullet = m_em.CreateEntity<Transform>();

    Entity& entity = std::get<0>(bullet);
    Transform& transf = std::get<1>(bullet);
    transf.pos = pos;
    //transf.rot = rot;
    transf.scale = glm::vec2(10.f, 10.f);
    transf.vel = vel;

    ResourcePtr<Model> modelPtr = m_resourceLoader.Load<Model>(ID("models/bullets/bullet-0"));
    ResourcePtr<Body> bodyPtr = m_resourceLoader.Load<Body>(ID("models/bullets/bullet-0"));

    m_em.AddComponent<Renderable>(entity, Renderable{modelPtr});
    m_em.AddComponent<Physics>(entity, ID("default"), bodyPtr);*/
}

static std::pair<glm::vec2, glm::vec2> GetBulletSpawnPosAndVel(Entity& ent, const Transform& transf, Physics& phys)
{
	if (!phys.body->GetHardpoints().empty()) {
		Body::Hardpoint hp = phys.body->GetHardpoints().front();
		float s = std::sin(transf.rot);
		float c = std::cos(transf.rot);

		glm::vec2 pos(
			hp.pos.x * c - hp.pos.y * s,
			hp.pos.x * s + hp.pos.y * c
		);

		pos += transf.pos;

		glm::vec2 vel(
			300.0 * std::cos(transf.rot - 1.5708),
			300.0 * std::sin(transf.rot - 1.5708)
		);

		vel += transf.vel;

		return std::make_pair(pos, vel);
	}
	else {
		return std::make_pair(glm::vec2(), glm::vec2());
	}
}

void ShipControlsSystem::Update(int step, Entity& ent, const Transform& transf, Physics& phys, ShipControls& scontrols)
{
	cpBody* body = phys.cp.body.get();

	cpFloat ang = cpBodyGetAngularVelocity(body);
	const cpFloat maxAng = 15.0;

	cpBodyApplyTorque(body, -ang * 4);

	if (scontrols.actionFlags.rotateLeft && ang < maxAng) {
		cpBodyApplyTorque(body, 20.0);
	}
	if (scontrols.actionFlags.rotateRight && ang > -maxAng) {
		cpBodyApplyTorque(body, -20.0);
	}
	if (scontrols.actionFlags.thrustForward) {
		cpBodyApplyForceAtLocalPoint(body, cpv(0, -140), cpv(0, 0));
	}
	if (scontrols.actionFlags.firePrimary) {
		scontrols.actionFlags.firePrimary = false;
		std::pair<glm::vec2, glm::vec2> ret = GetBulletSpawnPosAndVel(ent, transf, phys);
		SpawnBullet(step, phys.spaceId, ret.first, ret.second);
	}
}

} // namespace Starbase
