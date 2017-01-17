#pragma once

#include <cstdint>
#include <unordered_map>
#include <memory>

#include <starbase/game/id.hpp>
#include <starbase/game/fs/ifilesystem.hpp>
#include <starbase/game/resource/iresource.hpp>

namespace Starbase {

class ResourceLoader {
private:
	IFilesystem& m_filesystem;
    std::unordered_map<id_t, std::weak_ptr<const IResource>> m_resourcePtrs;

public:
	ResourceLoader(IFilesystem& filesystem) : m_filesystem(filesystem) {}

    template<class T>
    std::shared_ptr<const T> Load(id_t id);

	template<class T>
	std::shared_ptr<const T> Get(id_t id);

	IFilesystem& GetFilesystem()
	{ return m_filesystem; }
};

#include "detail/resourceloader.inl"

} // namespace Starbase