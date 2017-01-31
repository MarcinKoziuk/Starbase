#pragma once

namespace Starbase {

struct ShipControls {
    struct {
        bool thrustForward : 1;
        bool rotateLeft : 1;
        bool rotateRight : 1;
        bool firePrimary : 1;
        bool fireSecondary : 1;
    } actionFlags;
};

} // namespace Starbase
