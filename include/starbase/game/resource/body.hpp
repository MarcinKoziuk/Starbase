#pragma once

#include <string>
#include <memory>

#include <starbase/game/fs/ifilesystem.hpp>
#include <starbase/game/resource/iresource.hpp>

namespace Starbase {
namespace Resource {

class Body : public IResource {
private:

	static std::shared_ptr<const Body> placeholder;

public:
    virtual ~Body() {}

    virtual std::size_t CalculateSize() const;

    virtual const char* GetResourceName() const
    { return "body"; }

    static std::shared_ptr<const Body> Placeholder();

    static std::shared_ptr<const Body> Create(id_t id, IFilesystem& filesystem);
};

typedef std::shared_ptr<const Body> BodyPtr;

} // namespace Resource
} // namespace Starbase
