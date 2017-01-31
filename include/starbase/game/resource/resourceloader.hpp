#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>
#include <memory>
#include <utility>
#include <typeindex>

#include <starbase/game/id.hpp>
#include <starbase/game/fs/ifilesystem.hpp>
#include <starbase/game/resource/iresource.hpp>
#include <starbase/game/resource/resource_ptr.hpp>

namespace Starbase {

class ResourceLoader {
private:
	struct TypedResId {
		std::type_index typeIndex;
		id_t resourceId;

		inline TypedResId(std::type_index type, id_t resourceId);
		template<typename T> static TypedResId Of(id_t);

		inline bool operator==(const TypedResId& other) const;
		inline int operator<(const TypedResId& other) const;

		struct Hash {
			inline std::size_t operator()(const TypedResId& k) const;
		};
	};

	IFilesystem& m_filesystem;
	std::unordered_map<id_t, std::vector<std::shared_ptr<const IResource>>> m_cachePtrs;
    std::unordered_map<TypedResId, std::weak_ptr<const IResource>, TypedResId::Hash> m_resourcePtrs;

public:
	ResourceLoader(IFilesystem& filesystem) : m_filesystem(filesystem) {}

	template<class T>
	ResourcePtr<T> Get(id_t id);

    template<class T>
	ResourcePtr<T> Load(id_t id);

	template<class T>
	ResourcePtr<T> Load(id_t id, id_t cacheId);

	template<class T>
	void Preload(id_t id, id_t cacheId);

	void UnloadCache(id_t cacheId)
	{ m_cachePtrs[cacheId].clear(); }

	IFilesystem& GetFilesystem()
	{ return m_filesystem; }
};


#include "detail/resourceloader.inl"

} // namespace Starbase
