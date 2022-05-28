#include "AeroCoefficient.hpp"

// disable all warnings of fast-cpp-csv-parser
#pragma warning(push, 0)
#include <fast-cpp-csv-parser/csv.h>
#pragma warning(pop)

#include "app/CommandLine.hpp"
#include "math/Algorithm.hpp"

size_t search(const std::vector<AeroCoefficient>& vs, double airspeed, size_t begin, size_t end) {
    if (begin == end || begin == end - 1) {
        return begin;
    }

    size_t mid = begin + (end - begin) / 2;
    if (vs[mid].internalVars.airspeed == airspeed) {
        return mid;
    } else if (vs[mid].internalVars.airspeed > airspeed) {
        return search(vs, airspeed, begin, mid);
    } else {
        return search(vs, airspeed, mid, end - 1);
    }
}

size_t getLowerIndex(const std::vector<AeroCoefficient>& vs, double airspeed) {
    size_t index = vs.size() / 2;
    if (vs[index].internalVars.airspeed == airspeed) {
        return index;
    } else if (vs[index].internalVars.airspeed > airspeed) {
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

        AeroCoefficient coef;
        while (csv.read_row(coef.internalVars.airspeed,
                            coef.Cp,
                            coef.internalVars.Cp_a,
                            coef.internalVars.Cd_i,
                            coef.internalVars.Cd_f,
                            coef.internalVars.Cd_a2,
                            coef.Cna)) {
            m_aeroCoefs.push_back(coef);
        }

        m_exist = true;
    } catch (const std::exception& e) {
        CommandLine::PrintInfo(PrintInfoType::Error, "While reading aero coefficient file.", e.what());
    }
}

AeroCoefficient AeroCoefficientStorage::valuesIn(double airspeed, double attackAngle, bool combustionEnded) const {
    AeroCoefficient aeroCoef;

    // No csv file or csv has only one row
    if (m_aeroCoefs.size() == 1) {
        aeroCoef = {m_aeroCoefs[0].Cp,
                    combustionEnded ? m_aeroCoefs[0].internalVars.Cd_f : m_aeroCoefs[0].internalVars.Cd_i,
                    m_aeroCoefs[0].Cna,
                    {
                        airspeed,
                        m_aeroCoefs[0].internalVars.Cd_i,
                        m_aeroCoefs[0].internalVars.Cd_f,
                        m_aeroCoefs[0].internalVars.Cp_a,
                        m_aeroCoefs[0].internalVars.Cd_a2,
                    }};
    } else {
        const size_t i = getLowerIndex(m_aeroCoefs, airspeed);

        const double airspeed1 = m_aeroCoefs[i].internalVars.airspeed;
        const double airspeed2 = m_aeroCoefs[i + 1].internalVars.airspeed;

        // lower than minimum airspeed
        if (airspeed < airspeed1) {
            aeroCoef = {m_aeroCoefs[i].Cp,
                        combustionEnded ? m_aeroCoefs[i].internalVars.Cd_f : m_aeroCoefs[i].internalVars.Cd_i,
                        m_aeroCoefs[i].Cna,
                        {
                            airspeed,
                            m_aeroCoefs[i].internalVars.Cd_i,
                            m_aeroCoefs[i].internalVars.Cd_f,
                            m_aeroCoefs[i].internalVars.Cp_a,
                            m_aeroCoefs[i].internalVars.Cd_a2,
                        }};
        }
        // higher than maximum airspeed
        else if (airspeed > airspeed2) {
            aeroCoef = {m_aeroCoefs[i + 1].Cp,
                        combustionEnded ? m_aeroCoefs[i + 1].internalVars.Cd_f : m_aeroCoefs[i + 1].internalVars.Cd_i,
                        m_aeroCoefs[i + 1].Cna,
                        {
                            airspeed,
                            m_aeroCoefs[i + 1].internalVars.Cd_i,
                            m_aeroCoefs[i + 1].internalVars.Cd_f,
                            m_aeroCoefs[i + 1].internalVars.Cp_a,
                            m_aeroCoefs[i + 1].internalVars.Cd_a2,
                        }};
        }
        // lerp
        else {
            const auto cd1 = combustionEnded ? m_aeroCoefs[i].internalVars.Cd_f : m_aeroCoefs[i].internalVars.Cd_i;
            const auto cd2 =
                combustionEnded ? m_aeroCoefs[i + 1].internalVars.Cd_f : m_aeroCoefs[i + 1].internalVars.Cd_i;
            aeroCoef = {Algorithm::Lerp(airspeed, airspeed1, airspeed2, m_aeroCoefs[i].Cp, m_aeroCoefs[i + 1].Cp),
                        Algorithm::Lerp(airspeed, airspeed1, airspeed2, cd1, cd2),
                        Algorithm::Lerp(airspeed, airspeed1, airspeed2, m_aeroCoefs[i].Cna, m_aeroCoefs[i + 1].Cna),
                        {
                            airspeed,
                            Algorithm::Lerp(airspeed,
                                            airspeed1,
                                            airspeed2,
                                            m_aeroCoefs[i].internalVars.Cd_i,
                                            m_aeroCoefs[i + 1].internalVars.Cd_i),
                            Algorithm::Lerp(airspeed,
                                            airspeed1,
                                            airspeed2,
                                            m_aeroCoefs[i].internalVars.Cd_f,
                                            m_aeroCoefs[i + 1].internalVars.Cd_f),
                            Algorithm::Lerp(airspeed,
                                            airspeed1,
                                            airspeed2,
                                            m_aeroCoefs[i].internalVars.Cp_a,
                                            m_aeroCoefs[i + 1].internalVars.Cp_a),
                            Algorithm::Lerp(airspeed,
                                            airspeed1,
                                            airspeed2,
                                            m_aeroCoefs[i].internalVars.Cd_a2,
                                            m_aeroCoefs[i + 1].internalVars.Cd_a2),
                        }};
        }
    }

    aeroCoef.Cd = aeroCoef.Cd + aeroCoef.internalVars.Cd_a2 * attackAngle * attackAngle;
    aeroCoef.Cp = aeroCoef.Cp + aeroCoef.internalVars.Cp_a * attackAngle;

    return aeroCoef;
}
