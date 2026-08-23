#include "SimulationInput.hpp"

#include <cmath>
#include <stdexcept>

namespace {
    void RequireFinite(double value, const char* message) {
        if (!std::isfinite(value)) {
            throw std::invalid_argument{message};
        }
    }
}

void ValidateSimulationInput(const SimulationInput& input) {
    ValidateSolverSettings(input.solver);

    RequireFinite(input.environment.railLength, "Launch rail length must be finite.");
    RequireFinite(input.environment.railAzimuth, "Launch rail azimuth must be finite.");
    RequireFinite(input.environment.railElevation, "Launch rail elevation must be finite.");
    if (input.environment.railLength < 0.0) {
        throw std::invalid_argument{"Launch rail length must not be negative."};
    }

    if (!std::isfinite(input.run.windSpeed) || input.run.windSpeed < 0.0) {
        throw std::invalid_argument{"Ground wind speed must be finite and non-negative."};
    }
    static_cast<void>(ResolveTrueNorthDirection(input.run.windDirection, input.run.magneticDeclination));

    if (!std::isfinite(input.run.detachTime)) {
        throw std::invalid_argument{"Detachment time must be finite."};
    }
    if (input.run.detachType == DetachType::Time && input.run.detachTime < 0.0) {
        throw std::invalid_argument{"Timed detachment must not use a negative time."};
    }

    // The current numerical solver intentionally preserves the legacy topology.
    // Reject unsupported typed topologies instead of indexing bodies implicitly.
    if (input.rocket.isMultiple()) {
        const auto& separations = input.rocket.separations();
        if (input.rocket.bodyCount() != 3 || separations.size() != 1
            || separations[0].sourceBodyIndex != 0
            || separations[0].productBodyIndices != std::vector<size_t>{1, 2}) {
            throw std::invalid_argument{
                "The current solver supports only the legacy separation topology 0 -> {1, 2}."};
        }
    } else if (input.rocket.bodyCount() != 1) {
        throw std::invalid_argument{"A non-separating simulation must contain exactly one body."};
    }
}
