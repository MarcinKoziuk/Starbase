#pragma once

namespace Starbase {

class IResource {
public:
    virtual ~IResource() {}

    virtual std::size_t CalculateSize() const = 0;

    virtual const char* GetResourceName() const = 0;
};

} // namespace Starbase
