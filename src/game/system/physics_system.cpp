#include <cmath>

#include <glm/glm.hpp>

#include <starbase/game/system/physics_system.hpp>

namespace Starbase {

static const cpTransform tzero = { 0.f, 0.f, 0.f, 0.f, 0.f, 0.f };

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
		moment += cpMomentForPoly(mass, poly.size(), (cpVect*) &poly.front(), cpvzero, 1.0);

		cpTransform trans = cpTransformIdentity;
		trans = cpTransformMult(trans, cpTransformScale(transf.scale.x, transf.scale.y));

		cpShape* shape = cpPolyShapeNew(body, poly.size(), (cpVect*) &poly.front(), trans, 1.0);
		/*cpShapeSetMass(shape, 1.0);
		cpShapeSetDensity(shape, 1.0);
		cpShapeSetDensity(shape, 1.0);
		cpShapeSetFriction(shape, 0.0);*/

		phys.cp.shapes.emplace_back(cpShapeUniquePtr(shape));
	}
	for (const Body::CircleShape& circle : bodyResource.GetCircleShapes()) {
		moment += cpMomentForCircle(mass, 0.f, circle.radius, to_cpv(circle.pos));
		cpShape* shape = cpCircleShapeNew(body, circle.radius * transf.scale.x, cpvzero);//to_cpv(-circle.pos)
		cpCircleShape* cshape = (cpCircleShape*)shape;
		phys.cp.shapes.emplace_back(cpShapeUniquePtr(shape));
	}
	
	if (moment) cpBodySetMoment(body, moment);
	if (mass) cpBodySetMass(body, mass);

	for (auto& it : phys.cp.shapes) {
		cpShape* shape = it.get();
		cpSpaceAddShape(space, shape);
	}

	cpBodySetAngle(body, transf.rot);
	cpBodySetPosition(body, cpv(transf.pos.x, transf.pos.y));

	cpSpaceAddBody(space, body);
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
	const static cpFloat gravityConstant = 11.25;

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


			cpFloat vel = gravityConstant * ((srcMass) / std::pow(dist, 2));
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
	transf.pos = to_vec2f(cpBodyGetPosition(body));
	transf.rot = cpvtoangle(cpBodyGetRotation(body));
}

PhysicsSystem::~PhysicsSystem()
{}

} // namespace Starbase
