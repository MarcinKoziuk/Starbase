#pragma once

#include <cassert>
#include <memory>

#include <starbase/game/id.hpp>

namespace Starbase {

template<typename T>
class ResourcePtr {
private:
	id_t m_id;
#ifndef NDEBUG
	std::string m_name;
#endif
	std::shared_ptr<const T> m_ref;

public:
	ResourcePtr()
		: m_id(-1L), m_ref(nullptr)
	{}

	ResourcePtr(id_t id, const std::shared_ptr<const T>& ref)
		: m_id(id), m_ref(std::move(ref))
	{
#ifndef NDEBUG
		m_name = IDD::GetString(id);
#endif
	}

	const T& operator*() const
	{
		assert(m_id != -1L && "Tried to dereference uninitialized ResourcePtr!");
		return *m_ref;
	}

	const T* operator->() const
	{
		assert(m_id != -1L && "Tried to dereference uninitialized ResourcePtr!");
		return m_ref.operator->();
	}

	const std::shared_ptr<const T>& Get() const
	{
		assert(m_id != -1L && "Tried to Get() uninitialized ResourcePtr!");
		return m_ref;
	}

	id_t Id() const
	{
		assert(m_id != -1L && "Tried to retrieve the Id of an uninitialized ResourcePtr!");
		return m_id;
	}

	const std::string& GetName() const
	{
#ifndef NDEBUG
		return m_name;
#else
		return IDD::GetString(m_id);
#endif
	}
};

} // namespace Starbase