#ifndef ESR_ECS_BUNDLE_H
#define ESR_ECS_BUNDLE_H

#include "entity.h"

namespace esr {

class World;

class Bundle {
public:
    virtual ~Bundle() = default;

    virtual Entity spawn(World& world) = 0;
};

} // namespace esr

#endif // ESR_ECS_BUNDLE_H
