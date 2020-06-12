#include <cmath>

#include <glm/glm.hpp>

#include <starbase/game/system/physics_system.hpp>

namespace Starbase {

static const cpTransform tzero = { 0.f, 0.f, 0.f, 0.f, 0.f, 0.f };

PhysicsSystem::PhysicsSystem(EventManager& eventManager)
	: m_eventManager(eventManager)
{
	eventManager.Connect<Physics, EventManager::component_added>([this](Entity& ent, Physics& physics) {
		this->PhysicsAdded(ent, ent.GetComponent<Transform>(), physics);
	});
	eventManager.Connect<Physics, EventManager::component_removed>([this](Entity& ent, Physics& physics) {
		this->PhysicsRemoved(ent, ent.GetComponent<Transform>(), physics);
	});
}

void PhysicsSystem::InitSpace(id_t spaceId)
{
	m_spaces.insert(
		std::make_pair(
			spaceId,
			std::shared_ptr<cpSpace>(cpSpaceNew(), cpSpaceDeleter())));
}

void PhysicsSystem::InitBody(const Entity& ent, Transform& transf, Physics& phys)
{
	const Body& bodyResource = *phys.body;
	cpSpace* space = m_spaces.at(phys.spaceId).get();

	phys.cp.body.reset(cpBodyNew(1.0, 1.0));
	phys.cp.space = m_spaces.at(phys.spaceId);

	cpBody* body = phys.cp.body.get();

	cpFloat moment = 0.0;
	cpFloat mass = bodyResource.GetMass();
	for (const auto& poly : bodyResource.GetPolygonShapes()) {
		moment += cpMomentForPoly(
			mass,
			static_cast<int>(poly.size()),
			reinterpret_cast<const cpVect*>(&poly.front()),
			cpvzero,
			0.0
		);

		cpTransform trans = cpTransformIdentity;
		trans = cpTransformMult(trans, cpTransformScale(transf.scale.x, transf.scale.y));

		cpShape* shape = cpPolyShapeNew(
			body,
			static_cast<int>(poly.size()),
			reinterpret_cast<const cpVect*>(&poly.front()),
			trans,
			0.0
		);
		/*cpShapeSetMass(shape, 1.0);
		cpShapeSetDensity(shape, 1.0);
		cpShapeSetDensity(shape, 1.0);
		cpShapeSetFriction(shape, 0.0);*/

		phys.cp.shapes.emplace_back(cpShapeUniquePtr(shape));
	}
	for (const Body::CircleShape& circle : bodyResource.GetCircleShapes()) {
		const cpVect offs = to_cpv(circle.pos * glm::tvec2<cpFloat>(transf.scale));
		moment += cpMomentForCircle(mass, 0.f, circle.radius, offs);
		cpShape* shape = cpCircleShapeNew(body, circle.radius * transf.scale.x, offs);
		cpCircleShape* cshape = (cpCircleShape*)shape;
		phys.cp.shapes.emplace_back(cpShapeUniquePtr(shape));
	}
	
	if (moment) cpBodySetMoment(body, moment);
	if (mass) cpBodySetMass(body, mass);


	cpSpaceAddBody(space, body);
	for (auto& it : phys.cp.shapes) {
		cpShape* shape = it.get();

		const cpFloat friction = phys.body->GetFriction();
		if (friction) {
			cpShapeSetFriction(shape, friction);
		}
		else {
			cpShapeSetFriction(shape, 0.05);
		}

		cpSpaceAddShape(space, shape);

	}

	cpBodySetAngle(body, transf.rot);
	cpBodySetPosition(body, cpv(transf.pos.x, transf.pos.y));

	

	//cpBodyApplyForceAtWorldPoint(body, to_cpv(transf.vel), cpvzero);
	cpBodySetVelocity(body, to_cpv(transf.vel));
}

void PhysicsSystem::PhysicsAdded(const Entity& ent, Transform& transf, Physics& phys)
{
	if (!m_spaces.count(phys.spaceId)) {
		InitSpace(phys.spaceId);
	}

	InitBody(ent, transf, phys);
}

void PhysicsSystem::PhysicsRemoved(const Entity& ent, Transform& transf, Physics& phys)
{
	// we stil leak spaces here.
}

void PhysicsSystem::ApplyGravity(cpSpace* space, float dt)
{
	const static cpFloat gravityConstant = 20.0;

	struct gcontext {
		cpSpace* space;
		cpBody* tgtBody;
	} ctx;

	ctx.space = space;

	cpSpaceEachBody(space, [](cpBody* tgtBody, void* ctx_) {
		gcontext* ctx = (gcontext*) ctx_;
		ctx->tgtBody = tgtBody;

		cpSpaceEachBody(ctx->space, [](cpBody* srcBody, void* ctx_) {
			gcontext* ctx = (gcontext*) ctx_;

			if (srcBody == ctx->tgtBody) return;

			const cpFloat tgtMass = cpBodyGetMass(ctx->tgtBody);
			const cpFloat srcMass = cpBodyGetMass(srcBody);
			const cpVect tgtPos = cpBodyGetPosition(ctx->tgtBody);
			const cpVect srcPos = cpBodyGetPosition(srcBody);
			const cpFloat dist = cpvdist(srcPos, tgtPos);


			cpFloat vel = gravityConstant * ((tgtMass * srcMass) / std::pow(dist, 2));
			cpFloat dir = std::atan2(srcPos.y - tgtPos.y, srcPos.x - tgtPos.x);
			cpVect force = cpv(std::cos(dir) * vel, std::sin(dir) * vel);
			//ctx->force = cpvadd(ctx->force, force);

			const cpVect tgtCenter = cpBodyGetCenterOfGravity(ctx->tgtBody);
			cpBodyApplyForceAtWorldPoint(ctx->tgtBody, force, cpvadd(tgtPos, tgtCenter));
		}, ctx);

		
	}, &ctx);
}

void PhysicsSystem::Simulate(float dt)
{
	for (const auto& p : m_spaces) {
		cpSpace* space = p.second.get();
		cpSpaceStep(space, dt);

		ApplyGravity(space, dt);
	}
}

void PhysicsSystem::Update(Entity& ent, Transform& transf, Physics& phys)
{
	cpBody* body = phys.cp.body.get();
	transf.prevPos = transf.pos;
	transf.pos = to_vec2f(cpBodyGetPosition(body));
	transf.rot = static_cast<float>(cpvtoangle(cpBodyGetRotation(body)));
	transf.vel = to_vec2f(cpBodyGetVelocity(body));
}

PhysicsSystem::~PhysicsSystem()
{}

} // namespace Starbase
