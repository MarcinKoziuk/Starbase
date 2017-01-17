#pragma once

#include <cstdint>
#include <bitset>

namespace Starbase {

typedef std::uint64_t entity_id;

static constexpr int MAX_COMPONENTS = 16;

struct Entity {
	typedef std::bitset<MAX_COMPONENTS> component_bitset;

	entity_id id;
	component_bitset bitset;
	bool alive : 1;
	bool needsToDie : 1;
	bool isnew : 1;

	Entity(entity_id id)
		: id(id)
		, alive(true)
		, needsToDie(false)
		, isnew(true)
	{}

	Entity()
		: id(0)
		, alive(false)
		, needsToDie(false)
		, isnew(false)
	{}
};

static bool operator==(const Entity& a, const Entity& b)
{
	return a.id == b.id
		&& a.bitset == b.bitset
		&& a.alive == b.alive
		&& a.needsToDie == b.needsToDie
		&& a.isnew == b.isnew;
}

} // namespace Starbase
