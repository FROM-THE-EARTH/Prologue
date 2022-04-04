#include "AeroCoefficient.hpp"

#include <fstream>

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
    std::fstream fs("input/aero_coef/" + filename);

    if (!fs.is_open()) {
        return;
    }

    char header[1024];
    fs.getline(header, 1024);
    size_t i = 0;
    char c;
    std::string dummy;
    while (!fs.eof()) {
        m_aeroCoefs.push_back(AeroCoefficient());
        fs >> m_aeroCoefs[i].internalVars.airspeed >> c >> m_aeroCoefs[i].Cp >> c >> m_aeroCoefs[i].internalVars.Cp_a
            >> c >> m_aeroCoefs[i].Cd >> c >> m_aeroCoefs[i].internalVars.Cd_a2 >> c >> m_aeroCoefs[i].Cna;
        i++;
    }
    if (m_aeroCoefs[m_aeroCoefs.size() - 1] == AeroCoefficient()) {
        m_aeroCoefs.pop_back();
    }

    fs.close();

    m_exist = true;
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
                        m_aeroCoefs[i].Cd,
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
                        m_aeroCoefs[i + 1].Cd,
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
            aeroCoef = {Algorithm::Lerp(airspeed, airspeed1, airspeed2, m_aeroCoefs[i].Cp, m_aeroCoefs[i + 1].Cp),
                        Algorithm::Lerp(airspeed, airspeed1, airspeed2, m_aeroCoefs[i].Cd, m_aeroCoefs[i + 1].Cd),
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
