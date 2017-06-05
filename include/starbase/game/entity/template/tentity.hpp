#pragma once

#include <cstdint>
#include <bitset>

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
	void SetBit(component_bitset& bitset, bool val)
	{
		bitset[CL::template indexOf<C>()] = val;
	}

	template<typename C>
	void SetBit(bool val)
	{
		SetBit<C>(bitset, val);
	}

public:
	template<typename C>
	static bool HasComponent(const component_bitset& bitset)
	{
		return bitset[CL::template indexOf<C>()];
	}

	template<typename ...Cs>
	static bool HasComponents(const component_bitset& bitset)
	{
		return TMP::And(HasComponent<Cs>(bitset)...);
	}

	template<typename C>
	bool HasComponent() const
	{
		return HasComponent<C>(bitset);
	}

	template<typename ...Cs>
	bool HasComponents() const
	{
		return HasComponents<Cs>();
	}

	template<typename C>
	C& GetComponent() const
	{
		return entityManager->GetComponent<C>(*this);
	}

	template<typename C>
	C* GetComponentOrNull() const
	{
		return HasComponent<C>() ? &GetComponent<C>() : nullptr;
	}
};

template<typename CL>
static bool operator==(const TEntity<CL>& a, const TEntity<CL>& b)
{
	return a.id == b.id
		&& a.bitset == b.bitset
		&& a.alive == b.alive
		&& a.needsToDie == b.needsToDie
		&& a.isnew == b.isnew;
}

} // namespace Starbase
