#pragma once

#include <cstdint>
#include <bitset>

#include "detail/tmp.hpp"

namespace Starbase {

typedef std::uint64_t entity_id;

static constexpr int MAX_COMPONENTS = 16;

template<typename CL>
class TEntityManager;

template<typename CL>
struct TEntity {
	typedef std::bitset<MAX_COMPONENTS> component_bitset;

	entity_id id;
	component_bitset bitset;
	bool alive : 1;
	bool needsToDie : 1;
	bool isnew : 1;

public:
	explicit TEntity(entity_id id, TEntityManager<CL>& entityManager)
		: id(id)
		, alive(true)
		, needsToDie(false)
		, isnew(true)
		, entityManager(&entityManager)
	{}

	explicit TEntity()
		: id(0)
		, alive(false)
		, needsToDie(false)
		, isnew(false)
		, entityManager(nullptr)
	{}

	friend TEntityManager<CL>;

private:
	TEntityManager<CL>* entityManager;

	template<typename C>
	void SetBit(component_bitset& bitset, bool val);

	template<typename C>
	void SetBit(bool val);

public:
	template<typename C>
	static bool HasComponent(const component_bitset& bitset);

	template<typename ...Cs>
	static bool HasComponents(const component_bitset& bitset);

	template<typename C>
	bool HasComponent() const;

	template<typename ...Cs>
	bool HasComponents() const;

	template<typename C>
	C& GetComponent() const;

	template<typename C>
	C* GetComponentOrNull() const;

	template<typename C, typename... Args>
	C& AddComponent(Args&&... args);

	template<typename C>
	void RemoveComponent();
};

template<typename CL, typename EL>
static bool operator==(const TEntity<CL>& a, const TEntity<CL>& b);

} // namespace Starbase

#include "detail/entity.inl"
