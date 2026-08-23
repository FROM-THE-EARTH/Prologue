#pragma once

#include <cstddef>

#include "dynamics/WindModel.hpp"
#include "math/Vector3D.hpp"

struct Body;
struct Rocket;

class SimulationObserver {
public:
    virtual ~SimulationObserver() = default;

    virtual void pushBody() = 0;
    virtual void setLaunchClear(const Body& body) = 0;
    virtual void setFirstParachuteOpen(const Body& body) = 0;
    virtual void setBodyFinalPosition(size_t bodyIndex, const Vector3D& pos) = 0;
    virtual void update(size_t bodyIndex,
                        const Rocket& rocket,
                        const Body& body,
                        const WindModel::AtmosphericConditions& air,
                        bool combusting) = 0;
};
