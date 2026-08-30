// ------------------------------------------------
// SimulationInputBuilder.hppの実装
// ------------------------------------------------

#include "SimulationInputBuilder.hpp"

#include <utility>

#include "misc/Constant.hpp"

namespace project {
    namespace {
        ::Parachute BuildParachute(const Parachute& source) {
            const double openingTime = source.openingTimeFromLaunch.value_or(-1.0);
            const double delayTime = source.openingTimeFromPeak.value_or(-1.0);
            const double openingHeight = source.openingHeight.value_or(-1.0);
            const unsigned char openingType =
                (openingTime >= 0.0 ? PARACHUTE_OPENING_TYPE_FIXED_TIME : 0x00)
                | (delayTime >= 0.0 ? PARACHUTE_OPENING_TYPE_TIME_FROM_DETECT_PEAK : 0x00)
                | (openingHeight >= 0.0 ? PARACHUTE_OPENING_TYPE_DETECT_PEAK : 0x00);

            return {.openingType = openingType,
                    .openingTime = openingTime,
                    .delayTime = delayTime,
                    .openingHeight = openingHeight,
                    .CdS = source.CdS};
        }

        BodySpecification BuildBody(const Body& source, const ResourceProvider& resourceProvider) {
            BodySpecification result;
            result.length = source.length;
            result.diameter = source.diameter;
            result.bottomArea = result.diameter * result.diameter * 0.25 * Constant::PI;
            result.CGLengthInitial = source.centerOfGravityInitial;
            result.CGLengthFinal = source.centerOfGravityFinal;
            result.massInitial = source.massInitial;
            result.massFinal = source.massFinal;
            result.rollingMomentInertiaInitial = source.pitchYawMomentOfInertiaInitial;
            result.rollingMomentInertiaFinal = source.pitchYawMomentOfInertiaFinal;
            result.Cmq = source.pitchDampingMomentCoefficient;

            for (const auto& parachute : source.parachutes) {
                result.parachutes.emplace_back(BuildParachute(parachute));
            }

            if (source.engine.thrustFile.has_value()) {
                result.engine = ::Engine(resourceProvider.loadThrustData(*source.engine.thrustFile));
            }
            result.engine.setThrustMeasuredPressure(source.engine.thrustMeasuredPressure);
            result.engine.setNozzleDiameter(source.engine.nozzleDiameter);

            if (source.aerodynamics.coefficientFile.has_value()) {
                result.aeroCoefStorage = AeroCoefficientStorage(
                    resourceProvider.loadAeroCoefficientData(*source.aerodynamics.coefficientFile), true);
            } else {
                result.aeroCoefStorage.init(source.aerodynamics.centerOfPressure,
                                            source.aerodynamics.centerOfPressureAlpha,
                                            source.aerodynamics.dragCoefficientInitial,
                                            source.aerodynamics.dragCoefficientFinal,
                                            source.aerodynamics.dragCoefficientAlphaSquared,
                                            source.aerodynamics.normalForceCoefficient);
            }

            for (const auto& transition : source.transitions) {
                result.transitions.emplace_back(
                    ::Transition{.time = transition.time, .mass = transition.mass, .Cd = transition.Cd});
            }
            return result;
        }
    }

    SimulationInput BuildSimulationInput(const Document& document,
                                         const ResourceProvider& resourceProvider) {
        std::vector<BodySpecification> bodies;
        bodies.reserve(document.bodies.size());
        for (const auto& body : document.bodies) {
            bodies.emplace_back(BuildBody(body, resourceProvider));
        }

        std::vector<SeparationSpecification> separations;
        separations.reserve(document.separations.size());
        for (const auto& separation : document.separations) {
            separations.emplace_back(SeparationSpecification{
                .sourceBodyIndex = separation.sourceBodyIndex,
                .productBodyIndices = separation.productBodyIndices,
            });
        }

        return SimulationInput{
            .environment = {
                .launchSite = {
                    .name = document.environment.place,
                    .latitude = document.environment.latitude,
                    .longitude = document.environment.longitude,
                    .coordinateZone = document.environment.coordinateZone,
                    .magneticDeclination = document.environment.magneticDeclination,
                },
                .launchRail = {
                    .length = document.environment.railLength,
                    .azimuth = document.environment.railAzimuth,
                    .elevation = document.environment.railElevation,
                },
            },
            .rocket = RocketSpecification(std::move(bodies), std::move(separations)),
        };
    }
}
