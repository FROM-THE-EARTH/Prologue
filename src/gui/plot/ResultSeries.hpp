#pragma once

#include <cstddef>
#include <span>
#include <string>

#include "core/SimulationResult.hpp"

struct GeographicPosition;

namespace gui::plot {
    enum class DetailQuantity {
        TimeFromLaunch, ElapsedTime, AirDensity, Gravity, Pressure, Temperature,
        WindEast, WindNorth, WindUp, WindSpeed, Mass, CenterOfGravity, PitchYawInertia, RollInertia,
        AttackAngleRadians, AttackAngleDegrees, Altitude, GroundSpeed, Airspeed,
        Acceleration, LongitudinalAcceleration, NormalForce,
        Cnp, Cny, Cmqp, Cmqy, CenterOfPressure, DragCoefficient, NormalForceCoefficient,
        Latitude, Longitude, Downrange, StaticMargin, DynamicPressure,
        PositionEast, PositionNorth, VelocityEast, VelocityNorth, VelocityUp,
        BodyAirspeedX, BodyAirspeedY, BodyAirspeedZ, BodyForceX, BodyForceY, BodyForceZ,
    };

    enum class SummaryQuantity {
        WindSpeed, WindDirection, LaunchClearTime, LaunchClearSpeed,
        PeakAltitude, PeakTime, AirspeedAtPeak,
        MaxDynamicPressure, MaxDynamicPressureTime, MaxDynamicPressureAltitude,
        MaxAirspeed, MaxAirspeedTime, MaxLongitudinalAcceleration, MaxLongitudinalAccelerationTime,
        ParachuteOpenTime, ParachuteOpenAltitude, ParachuteOpenAirspeed,
        FinalEast, FinalNorth, FinalAltitude, FinalDownrange, FinalLatitude, FinalLongitude,
    };

    struct DetailQuantityInfo {
        DetailQuantity quantity;
        const char* label;
        const char* unit;
    };

    struct SummaryQuantityInfo {
        SummaryQuantity quantity;
        const char* label;
        const char* unit;
    };

    std::span<const DetailQuantityInfo> DetailQuantities();
    std::span<const SummaryQuantityInfo> SummaryQuantities();
    std::string AxisLabel(const char* label, const char* unit);

    bool NeedsGeography(DetailQuantity quantity);
    bool NeedsGeography(SummaryQuantity quantity);
    bool NeedsBody(SummaryQuantity quantity);

    // Geographic coordinates are supplied by the geography adapter, never inferred
    // from a CSV file. They are only required for latitude/longitude selections.
    double DetailValue(const SimulationStep& step, DetailQuantity quantity,
                       const GeographicPosition* position = nullptr);
    double SummaryValue(const SimulationResult& result, SummaryQuantity quantity,
                        size_t bodyIndex = 0, const GeographicPosition* finalPosition = nullptr);
}
