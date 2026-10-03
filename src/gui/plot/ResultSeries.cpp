#include "gui/plot/ResultSeries.hpp"

#include <cmath>
#include <stdexcept>

#include "geography/GeographicResult.hpp"
#include "misc/Constant.hpp"

namespace gui::plot {
    namespace {
        constexpr DetailQuantityInfo DetailFields[] = {
            {DetailQuantity::TimeFromLaunch, "Time from launch", "s"},
            {DetailQuantity::ElapsedTime, "Body elapsed time", "s"},
            {DetailQuantity::AirDensity, "Air density", "kg/m^3"},
            {DetailQuantity::Gravity, "Gravity", "m/s^2"},
            {DetailQuantity::Pressure, "Pressure", "Pa"},
            {DetailQuantity::Temperature, "Temperature", "deg C"},
            {DetailQuantity::WindEast, "Wind east", "m/s"},
            {DetailQuantity::WindNorth, "Wind north", "m/s"},
            {DetailQuantity::WindUp, "Wind up", "m/s"},
            {DetailQuantity::WindSpeed, "Wind speed magnitude", "m/s"},
            {DetailQuantity::Mass, "Mass", "kg"},
            {DetailQuantity::CenterOfGravity, "Center of gravity from nose", "m"},
            {DetailQuantity::PitchYawInertia, "Pitch/yaw moment of inertia", "kg m^2"},
            {DetailQuantity::RollInertia, "Roll moment of inertia", "kg m^2"},
            {DetailQuantity::AttackAngleRadians, "Angle of attack", "rad"},
            {DetailQuantity::AttackAngleDegrees, "Angle of attack", "deg"},
            {DetailQuantity::Altitude, "Altitude", "m"},
            {DetailQuantity::GroundSpeed, "Ground speed", "m/s"},
            {DetailQuantity::Airspeed, "Airspeed", "m/s"},
            {DetailQuantity::Acceleration, "Acceleration magnitude", "m/s^2"},
            {DetailQuantity::LongitudinalAcceleration, "Longitudinal acceleration", "m/s^2"},
            {DetailQuantity::NormalForce, "Normal force", "N"},
            {DetailQuantity::Cnp, "Cnp", ""},
            {DetailQuantity::Cny, "Cny", ""},
            {DetailQuantity::Cmqp, "Cmqp", ""},
            {DetailQuantity::Cmqy, "Cmqy", ""},
            {DetailQuantity::CenterOfPressure, "Center of pressure from nose", "m"},
            {DetailQuantity::DragCoefficient, "Drag coefficient", ""},
            {DetailQuantity::NormalForceCoefficient, "Normal force coefficient", ""},
            {DetailQuantity::Latitude, "Latitude", "deg"},
            {DetailQuantity::Longitude, "Longitude", "deg"},
            {DetailQuantity::Downrange, "Downrange", "m"},
            {DetailQuantity::StaticMargin, "Static margin Fst", "%"},
            {DetailQuantity::DynamicPressure, "Dynamic pressure", "Pa"},
            {DetailQuantity::PositionEast, "Position east", "m"},
            {DetailQuantity::PositionNorth, "Position north", "m"},
            {DetailQuantity::VelocityEast, "Velocity east", "m/s"},
            {DetailQuantity::VelocityNorth, "Velocity north", "m/s"},
            {DetailQuantity::VelocityUp, "Velocity up", "m/s"},
            {DetailQuantity::BodyAirspeedX, "Body airspeed x", "m/s"},
            {DetailQuantity::BodyAirspeedY, "Body airspeed y", "m/s"},
            {DetailQuantity::BodyAirspeedZ, "Body airspeed z", "m/s"},
            {DetailQuantity::BodyForceX, "Body force x", "N"},
            {DetailQuantity::BodyForceY, "Body force y", "N"},
            {DetailQuantity::BodyForceZ, "Body force z", "N"},
        };

        constexpr SummaryQuantityInfo SummaryFields[] = {
            {SummaryQuantity::WindSpeed, "Ground wind speed", "m/s"},
            {SummaryQuantity::WindDirection, "Input wind direction", "deg"},
            {SummaryQuantity::LaunchClearTime, "Launch clear time", "s"},
            {SummaryQuantity::LaunchClearSpeed, "Launch clear speed", "m/s"},
            {SummaryQuantity::PeakAltitude, "Peak altitude", "m"},
            {SummaryQuantity::PeakTime, "Time at peak altitude", "s"},
            {SummaryQuantity::AirspeedAtPeak, "Airspeed at peak altitude", "m/s"},
            {SummaryQuantity::MaxDynamicPressure, "Maximum dynamic pressure during ascent", "Pa"},
            {SummaryQuantity::MaxDynamicPressureTime, "Time at maximum dynamic pressure", "s"},
            {SummaryQuantity::MaxDynamicPressureAltitude, "Altitude at maximum dynamic pressure", "m"},
            {SummaryQuantity::MaxAirspeed, "Maximum airspeed", "m/s"},
            {SummaryQuantity::MaxAirspeedTime, "Time at maximum airspeed", "s"},
            {SummaryQuantity::MaxLongitudinalAcceleration, "Maximum longitudinal acceleration", "m/s^2"},
            {SummaryQuantity::MaxLongitudinalAccelerationTime, "Time at maximum longitudinal acceleration", "s"},
            {SummaryQuantity::ParachuteOpenTime, "First parachute opening time", "s"},
            {SummaryQuantity::ParachuteOpenAltitude, "First parachute opening altitude", "m"},
            {SummaryQuantity::ParachuteOpenAirspeed, "First parachute opening airspeed", "m/s"},
            {SummaryQuantity::FinalEast, "Body final position east", "m"},
            {SummaryQuantity::FinalNorth, "Body final position north", "m"},
            {SummaryQuantity::FinalAltitude, "Body final position up", "m"},
            {SummaryQuantity::FinalDownrange, "Body final downrange", "m"},
            {SummaryQuantity::FinalLatitude, "Body final latitude", "deg"},
            {SummaryQuantity::FinalLongitude, "Body final longitude", "deg"},
        };

        const GeographicPosition& RequirePosition(const GeographicPosition* position) {
            if (position == nullptr) {
                throw std::invalid_argument{"Geographic coordinates are required for this plot quantity."};
            }
            return *position;
        }
    }

    std::span<const DetailQuantityInfo> DetailQuantities() { return DetailFields; }
    std::span<const SummaryQuantityInfo> SummaryQuantities() { return SummaryFields; }

    std::string AxisLabel(const char* label, const char* unit) {
        return *unit == '\0' ? std::string(label) : std::string(label) + " [" + unit + "]";
    }

    bool NeedsGeography(DetailQuantity quantity) {
        return quantity == DetailQuantity::Latitude || quantity == DetailQuantity::Longitude;
    }

    bool NeedsGeography(SummaryQuantity quantity) {
        return quantity == SummaryQuantity::FinalLatitude || quantity == SummaryQuantity::FinalLongitude;
    }

    bool NeedsBody(SummaryQuantity quantity) {
        switch (quantity) {
        case SummaryQuantity::FinalEast:
        case SummaryQuantity::FinalNorth:
        case SummaryQuantity::FinalAltitude:
        case SummaryQuantity::FinalDownrange:
        case SummaryQuantity::FinalLatitude:
        case SummaryQuantity::FinalLongitude:
            return true;
        default:
            return false;
        }
    }

    double DetailValue(const SimulationStep& step, DetailQuantity quantity,
                       const GeographicPosition* position) {
        switch (quantity) {
        case DetailQuantity::TimeFromLaunch: return step.gen_timeFromLaunch;
        case DetailQuantity::ElapsedTime: return step.gen_elapsedTime;
        case DetailQuantity::AirDensity: return step.air_density;
        case DetailQuantity::Gravity: return step.air_gravity;
        case DetailQuantity::Pressure: return step.air_pressure;
        case DetailQuantity::Temperature: return step.air_temperature;
        case DetailQuantity::WindEast: return step.air_wind.x;
        case DetailQuantity::WindNorth: return step.air_wind.y;
        case DetailQuantity::WindUp: return step.air_wind.z;
        case DetailQuantity::WindSpeed: return step.air_wind.length();
        case DetailQuantity::Mass: return step.rocket_mass;
        case DetailQuantity::CenterOfGravity: return step.rocket_cgLength;
        case DetailQuantity::PitchYawInertia: return step.rocket_iyz;
        case DetailQuantity::RollInertia: return step.rocket_ix;
        case DetailQuantity::AttackAngleRadians: return step.rocket_attackAngle;
        case DetailQuantity::AttackAngleDegrees: return step.rocket_attackAngle * 180.0 / Constant::PI;
        case DetailQuantity::Altitude: return step.rocket_pos.z;
        case DetailQuantity::GroundSpeed: return step.rocket_velocity.length();
        case DetailQuantity::Airspeed: return step.rocket_airspeed_b.length();
        case DetailQuantity::Acceleration: return step.rocket_force_b.length() / step.rocket_mass;
        case DetailQuantity::LongitudinalAcceleration: return step.rocket_force_b.x / step.rocket_mass;
        case DetailQuantity::NormalForce:
            return std::sqrt(step.rocket_force_b.y * step.rocket_force_b.y
                             + step.rocket_force_b.z * step.rocket_force_b.z);
        case DetailQuantity::Cnp: return step.Cnp;
        case DetailQuantity::Cny: return step.Cny;
        case DetailQuantity::Cmqp: return step.Cmqp;
        case DetailQuantity::Cmqy: return step.Cmqy;
        case DetailQuantity::CenterOfPressure: return step.Cp;
        case DetailQuantity::DragCoefficient: return step.Cd;
        case DetailQuantity::NormalForceCoefficient: return step.Cna;
        case DetailQuantity::Latitude: return RequirePosition(position).latitude;
        case DetailQuantity::Longitude: return RequirePosition(position).longitude;
        case DetailQuantity::Downrange: return step.downrange;
        case DetailQuantity::StaticMargin: return step.Fst;
        case DetailQuantity::DynamicPressure: return step.dynamicPressure;
        case DetailQuantity::PositionEast: return step.rocket_pos.x;
        case DetailQuantity::PositionNorth: return step.rocket_pos.y;
        case DetailQuantity::VelocityEast: return step.rocket_velocity.x;
        case DetailQuantity::VelocityNorth: return step.rocket_velocity.y;
        case DetailQuantity::VelocityUp: return step.rocket_velocity.z;
        case DetailQuantity::BodyAirspeedX: return step.rocket_airspeed_b.x;
        case DetailQuantity::BodyAirspeedY: return step.rocket_airspeed_b.y;
        case DetailQuantity::BodyAirspeedZ: return step.rocket_airspeed_b.z;
        case DetailQuantity::BodyForceX: return step.rocket_force_b.x;
        case DetailQuantity::BodyForceY: return step.rocket_force_b.y;
        case DetailQuantity::BodyForceZ: return step.rocket_force_b.z;
        }
        throw std::invalid_argument{"Unknown detail plot quantity."};
    }

    double SummaryValue(const SimulationResult& result, SummaryQuantity quantity,
                        size_t bodyIndex, const GeographicPosition* finalPosition) {
        switch (quantity) {
        case SummaryQuantity::WindSpeed: return result.windSpeed;
        case SummaryQuantity::WindDirection: return result.windDirection.degrees;
        case SummaryQuantity::LaunchClearTime: return result.launchClearTime;
        case SummaryQuantity::LaunchClearSpeed: return result.launchClearVelocity.length();
        case SummaryQuantity::PeakAltitude: return result.maxAltitude;
        case SummaryQuantity::PeakTime: return result.detectPeakTime;
        case SummaryQuantity::AirspeedAtPeak: return result.airspeedAtPeak;
        case SummaryQuantity::MaxDynamicPressure: return result.maxDynamicPressureDuringRising;
        case SummaryQuantity::MaxDynamicPressureTime: return result.maxDynamicPressureTime;
        case SummaryQuantity::MaxDynamicPressureAltitude: return result.maxDynamicPressureAltitude;
        case SummaryQuantity::MaxAirspeed: return result.maxAirspeed;
        case SummaryQuantity::MaxAirspeedTime: return result.maxAirspeedTime;
        case SummaryQuantity::MaxLongitudinalAcceleration: return result.maxLongitudinalAccel;
        case SummaryQuantity::MaxLongitudinalAccelerationTime: return result.maxLongitudinalAccelTime;
        case SummaryQuantity::ParachuteOpenTime: return result.firstParachuteOpenTime;
        case SummaryQuantity::ParachuteOpenAltitude: return result.firstParachuteOpenAltitude;
        case SummaryQuantity::ParachuteOpenAirspeed: return result.firstParachuteOpenAirspeed;
        case SummaryQuantity::FinalEast: return result.bodyFinalPositions.at(bodyIndex).x;
        case SummaryQuantity::FinalNorth: return result.bodyFinalPositions.at(bodyIndex).y;
        case SummaryQuantity::FinalAltitude: return result.bodyFinalPositions.at(bodyIndex).z;
        case SummaryQuantity::FinalDownrange: {
            const auto& final = result.bodyFinalPositions.at(bodyIndex);
            return std::sqrt(final.x * final.x + final.y * final.y);
        }
        case SummaryQuantity::FinalLatitude: return RequirePosition(finalPosition).latitude;
        case SummaryQuantity::FinalLongitude: return RequirePosition(finalPosition).longitude;
        }
        throw std::invalid_argument{"Unknown summary plot quantity."};
    }
}
