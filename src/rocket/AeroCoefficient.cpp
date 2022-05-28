#include "AeroCoefficient.hpp"

// disable all warnings of fast-cpp-csv-parser
#pragma warning(push, 0)
#include <fast-cpp-csv-parser/csv.h>
#pragma warning(pop)

#include "app/CommandLine.hpp"
#include "math/Algorithm.hpp"

size_t search(const std::vector<Internal::AeroCoefSpec>& vs, double airspeed, size_t begin, size_t end) {
    if (begin == end || begin == end - 1) {
        return begin;
    }

    size_t mid = begin + (end - begin) / 2;
    if (vs[mid].airspeed == airspeed) {
        return mid;
    } else if (vs[mid].airspeed > airspeed) {
        return search(vs, airspeed, begin, mid);
    } else {
        return search(vs, airspeed, mid, end - 1);
    }
}

size_t getLowerIndex(const std::vector<Internal::AeroCoefSpec>& vs, double airspeed) {
    size_t index = vs.size() / 2;
    if (vs[index].airspeed == airspeed) {
        return index;
    } else if (vs[index].airspeed > airspeed) {
        return search(vs, airspeed, 0, index);
    } else {
        return search(vs, airspeed, index, vs.size() - 1);
    }
}

void AeroCoefficientStorage::init(const std::string& filename) {
    if (filename.empty()) {
        return;
    }

    try {
        io::CSVReader<7> csv("input/aero_coef/" + filename);
        csv.read_header(io::ignore_extra_column,
                        "air_speed[m/s]",
                        "Cp_from_nose[m]",
                        "Cp_a[m/rad]",
                        "Cd_i",
                        "Cd_f",
                        "Cd_a2[/rad^2]",
                        "Cna");

        Internal::AeroCoefSpec spec;
        while (csv.read_row(spec.airspeed, spec.Cp, spec.Cp_a, spec.Cd_i, spec.Cd_f, spec.Cd_a2, spec.Cna)) {
            m_aeroCoefSpec.push_back(spec);
        }

        m_isTimeSeries = true;
    } catch (const std::exception& e) {
        CommandLine::PrintInfo(PrintInfoType::Error, "While reading aero coefficient file.", e.what());
    }
}

AeroCoefficient AeroCoefficientStorage::valuesIn(double airspeed, double attackAngle, bool combustionEnded) const {
    Internal::AeroCoefSpec spec;

    // No csv file or csv has only one row
    if (m_aeroCoefSpec.size() == 1) {
        spec = m_aeroCoefSpec[0];
    } else {
        const size_t i = getLowerIndex(m_aeroCoefSpec, airspeed);

        const auto& refSpec1 = m_aeroCoefSpec[i];
        const auto& refSpec2 = m_aeroCoefSpec[i + 1];

        const double airspeed1 = refSpec1.airspeed;
        const double airspeed2 = refSpec2.airspeed;

        // lower than minimum airspeed
        if (airspeed < airspeed1) {
            spec = refSpec1;
        }
        // higher than maximum airspeed
        else if (airspeed > airspeed2) {
            spec = refSpec2;
        }
        // lerp
        else {
            spec = {
                .airspeed = airspeed,
                .Cp       = Algorithm::Lerp(airspeed, airspeed1, airspeed2, refSpec1.Cp, refSpec2.Cp),
                .Cp_a     = Algorithm::Lerp(airspeed, airspeed1, airspeed2, refSpec1.Cp_a, refSpec2.Cp_a),
                .Cd_i     = Algorithm::Lerp(airspeed, airspeed1, airspeed2, refSpec1.Cd_i, refSpec2.Cd_i),
                .Cd_f     = Algorithm::Lerp(airspeed, airspeed1, airspeed2, refSpec1.Cd_f, refSpec2.Cd_f),
                .Cd_a2    = Algorithm::Lerp(airspeed, airspeed1, airspeed2, refSpec1.Cd_a2, refSpec2.Cd_a2),
                .Cna      = Algorithm::Lerp(airspeed, airspeed1, airspeed2, refSpec1.Cna, refSpec2.Cna),
            };
        };
    }

    return AeroCoefficient{.Cp  = spec.Cp + spec.Cp_a * attackAngle,
                           .Cd  = (combustionEnded ? spec.Cd_f : spec.Cd_i) + spec.Cd_a2 * attackAngle * attackAngle,
                           .Cna = spec.Cna};
}
