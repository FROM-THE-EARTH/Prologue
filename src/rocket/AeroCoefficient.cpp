// ------------------------------------------------
// AeroCoefficient.hppの実装
// ------------------------------------------------

#include "AeroCoefficient.hpp"

#include <algorithm>
#include <utility>

#include "math/Interpolation.hpp"

size_t getLowerIndex(const std::vector<AeroCoefficientData>& spec, double airspeed) {
    const auto it = std::lower_bound(
        spec.begin() + 1,
        spec.end() - 1,
        AeroCoefficientData{.airspeed = airspeed},
        [](const AeroCoefficientData& s1, const AeroCoefficientData& s2) { return s1.airspeed < s2.airspeed; });
    return std::distance(spec.begin(), it) - 1;
}

AeroCoefficientStorage::AeroCoefficientStorage(std::vector<AeroCoefficientData> aeroCoefSpec, bool isTimeSeries) :
    m_aeroCoefSpec(std::move(aeroCoefSpec)), m_isTimeSeries(isTimeSeries) {}

AeroCoefficient AeroCoefficientStorage::valuesIn(double airspeed, double attackAngle, bool combustionEnded) const {
    AeroCoefficientData spec;

    // No csv file or csv has only one row
    if (m_aeroCoefSpec.size() == 1) {
        spec = m_aeroCoefSpec[0];
    } else {
        // lower than minimum airspeed
        if (const auto& minAeroCoef = m_aeroCoefSpec[0]; airspeed < minAeroCoef.airspeed) {
            spec = minAeroCoef;
        }
        // higher than maximum airspeed
        else if (const auto& maxAeroCoef = m_aeroCoefSpec[m_aeroCoefSpec.size() - 1]; airspeed > maxAeroCoef.airspeed) {
            spec = maxAeroCoef;
        }
        // lerp
        else {
            const size_t i = getLowerIndex(m_aeroCoefSpec, airspeed);

            const auto& refSpec1 = m_aeroCoefSpec[i];
            const auto& refSpec2 = m_aeroCoefSpec[i + 1];

            const double airspeed1 = refSpec1.airspeed;
            const double airspeed2 = refSpec2.airspeed;

            spec = {
                .airspeed = airspeed,
                .Cp       = Interpolation::Linear(airspeed, airspeed1, airspeed2, refSpec1.Cp, refSpec2.Cp),
                .Cp_a     = Interpolation::Linear(airspeed, airspeed1, airspeed2, refSpec1.Cp_a, refSpec2.Cp_a),
                .Cd_i     = Interpolation::Linear(airspeed, airspeed1, airspeed2, refSpec1.Cd_i, refSpec2.Cd_i),
                .Cd_f     = Interpolation::Linear(airspeed, airspeed1, airspeed2, refSpec1.Cd_f, refSpec2.Cd_f),
                .Cd_a2    = Interpolation::Linear(airspeed, airspeed1, airspeed2, refSpec1.Cd_a2, refSpec2.Cd_a2),
                .Cna      = Interpolation::Linear(airspeed, airspeed1, airspeed2, refSpec1.Cna, refSpec2.Cna),
            };
        };
    }

    return AeroCoefficient{
        .Cp  = m_constant.Cp + spec.Cp + spec.Cp_a * attackAngle,
        .Cd  = m_constant.Cd + (combustionEnded ? spec.Cd_f : spec.Cd_i) + spec.Cd_a2 * attackAngle * attackAngle,
        .Cna = m_constant.Cna + spec.Cna};
}
