#pragma once

#include <string>
#include <memory>

#include <starbase/game/fs/ifilesystem.hpp>
#include <starbase/game/resource/iresource.hpp>

namespace Starbase {
namespace Resource {

class Text : public IResource {
private:
    std::string m_text;

	static std::shared_ptr<const Text> placeholder;

public:
    virtual ~Text() {}

    const std::string& operator*() const
    { return m_text; }

    virtual std::size_t CalculateSize() const;

    virtual const char* GetResourceName() const
    { return "text"; }

    static std::shared_ptr<const Text> Placeholder();

    static std::shared_ptr<const Text> Create(id_t id, IFilesystem& filesystem);
};

typedef std::shared_ptr<const Text> TextPtr;

} // namespace Resource
} // namespace Starbase
