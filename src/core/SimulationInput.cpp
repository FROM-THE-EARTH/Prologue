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

    const auto& launchSite = input.environment.launchSite;
    const auto& launchRail = input.environment.launchRail;

    RequireFinite(launchSite.latitude, "Launch-site latitude must be finite.");
    RequireFinite(launchSite.longitude, "Launch-site longitude must be finite.");
    RequireFinite(launchSite.magneticDeclination,
                  "Launch-site magnetic declination must be finite.");
    if (launchSite.latitude < -90.0 || launchSite.latitude > 90.0) {
        throw std::invalid_argument{"Launch-site latitude must be in [-90, 90] degrees."};
    }
    if (launchSite.longitude < -180.0 || launchSite.longitude > 180.0) {
        throw std::invalid_argument{"Launch-site longitude must be in [-180, 180] degrees."};
    }
    if (launchSite.coordinateZone < 1 || launchSite.coordinateZone > 19) {
        throw std::invalid_argument{"Launch-site coordinate zone must be in [1, 19]."};
    }

    RequireFinite(launchRail.length, "Launch rail length must be finite.");
    RequireFinite(launchRail.azimuth, "Launch rail azimuth must be finite.");
    RequireFinite(launchRail.elevation, "Launch rail elevation must be finite.");
    if (launchRail.length < 0.0) {
        throw std::invalid_argument{"Launch rail length must not be negative."};
    }

    if (!std::isfinite(input.run.windSpeed) || input.run.windSpeed < 0.0) {
        throw std::invalid_argument{"Ground wind speed must be finite and non-negative."};
    }
    static_cast<void>(
        ResolveTrueNorthDirection(input.run.windDirection, launchSite.magneticDeclination));

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
