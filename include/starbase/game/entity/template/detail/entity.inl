#pragma once

#include "tmp.hpp"

namespace Starbase {

#define TENTITY_TEMPLATE \
template<typename CL>

#define TENTITY_DECL \
TEntity<CL>

TENTITY_TEMPLATE
template<typename C>
void TENTITY_DECL::SetBit(component_bitset& bitset, bool val)
{
	bitset[CL::template indexOf<C>()] = val;
}

TENTITY_TEMPLATE
template<typename C>
void TENTITY_DECL::SetBit(bool val)
{
	SetBit<C>(bitset, val);
}

TENTITY_TEMPLATE
template<typename C>
bool TENTITY_DECL::HasComponent(const component_bitset& bitset)
{
	return bitset[CL::template indexOf<C>()];
}

TENTITY_TEMPLATE
template<typename ...Cs>
bool TENTITY_DECL::HasComponents(const component_bitset& bitset)
{
	return TMP::And(HasComponent<Cs>(bitset)...);
}

TENTITY_TEMPLATE
template<typename C>
bool TENTITY_DECL::HasComponent() const
{
	return HasComponent<C>(bitset);
}

TENTITY_TEMPLATE
template<typename ...Cs>
bool TENTITY_DECL::HasComponents() const
{
	return TMP::And(HasComponent<Cs>()...);
}

TENTITY_TEMPLATE
template<typename C>
C& TENTITY_DECL::GetComponent() const
{
	return entityManager->template GetComponent<C>(*this);
}

TENTITY_TEMPLATE
template<typename C>
C* TENTITY_DECL::GetComponentOrNull() const
{
	return HasComponent<C>() ? &GetComponent<C>() : nullptr;
}

TENTITY_TEMPLATE
template<typename C, typename... Args>
C& TENTITY_DECL::AddComponent(Args&&... args)
{
    return entityManager->template AddComponent<C>(*this, std::forward<Args>(args)...);
}

TENTITY_TEMPLATE
template<typename C>
void TENTITY_DECL::RemoveComponent()
{
    return entityManager->template RemoveComponent<C>(*this);
}

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
