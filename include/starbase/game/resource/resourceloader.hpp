#pragma once

#include <map>
#include <memory>
#include <string>

#include <starbase/game/fs/ifilesystem.hpp>
#include <starbase/game/resource/iresource.hpp>

namespace Starbase {

class ResourceLoader {
private:
	IFilesystem& m_filesystem;
    std::map<std::string, std::weak_ptr<const IResource>> m_resourcePtrs;

public:
	ResourceLoader(IFilesystem& filesystem) : m_filesystem(filesystem) {}

    template<class T>
    std::shared_ptr<const T> Load(const std::string& path);
};

#include "inline/resourceloader.inl"

} // namespace Starbase