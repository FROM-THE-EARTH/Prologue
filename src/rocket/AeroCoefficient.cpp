#include "AeroCoefficient.hpp"

#include <fstream>

#include "math/Algorithm.hpp"

size_t search(const std::vector<AeroCoefficient>& vs, double airspeed, size_t begin, size_t end) {
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

size_t getLowerIndex(const std::vector<AeroCoefficient>& vs, double airspeed) {
    size_t index = vs.size() / 2;
    if (vs[index].airspeed == airspeed) {
        return index;
    } else if (vs[index].airspeed > airspeed) {
        return search(vs, airspeed, 0, index);
    } else {
        return search(vs, airspeed, index, vs.size() - 1);
    }
}

void AeroCoefVsAirspeed::init(const std::string& filename) {
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
        fs >> m_aeroCoefs[i].airspeed >> c >> m_aeroCoefs[i].Cp >> c >> m_aeroCoefs[i].Cp_a >> c >> m_aeroCoefs[i].Cd
            >> c >> m_aeroCoefs[i].Cd_a2 >> c >> m_aeroCoefs[i].Cna;
        i++;
    }
    if (m_aeroCoefs[m_aeroCoefs.size() - 1] == AeroCoefficient()) {
        m_aeroCoefs.pop_back();
    }

    fs.close();

    m_exist = true;
}

AeroCoefficient AeroCoefVsAirspeed::valuesAt(double airspeed) const {
    if (m_aeroCoefs.size() == 1) {
        return {airspeed,
                m_aeroCoefs[0].Cp,
                m_aeroCoefs[0].Cp_a,
                m_aeroCoefs[0].Cd,
                m_aeroCoefs[0].Cd_a2,
                m_aeroCoefs[0].Cna};
    }

    const size_t i = getLowerIndex(m_aeroCoefs, airspeed);

    const double airspeed1 = m_aeroCoefs[i].airspeed;
    const double airspeed2 = m_aeroCoefs[i + 1].airspeed;

    if (airspeed < airspeed1) {
        return {airspeed,
                m_aeroCoefs[i].Cp,
                m_aeroCoefs[i].Cp_a,
                m_aeroCoefs[i].Cd,
                m_aeroCoefs[i].Cd_a2,
                m_aeroCoefs[i].Cna};
    }

    if (airspeed > airspeed2) {
        return {airspeed,
                m_aeroCoefs[i + 1].Cp,
                m_aeroCoefs[i + 1].Cp_a,
                m_aeroCoefs[i + 1].Cd,
                m_aeroCoefs[i + 1].Cd_a2,
                m_aeroCoefs[i + 1].Cna};
    }

    return {airspeed,
            Algorithm::Lerp(airspeed, airspeed1, airspeed2, m_aeroCoefs[i].Cp, m_aeroCoefs[i + 1].Cp),
            Algorithm::Lerp(airspeed, airspeed1, airspeed2, m_aeroCoefs[i].Cp_a, m_aeroCoefs[i + 1].Cp_a),
            Algorithm::Lerp(airspeed, airspeed1, airspeed2, m_aeroCoefs[i].Cd, m_aeroCoefs[i + 1].Cd),
            Algorithm::Lerp(airspeed, airspeed1, airspeed2, m_aeroCoefs[i].Cd_a2, m_aeroCoefs[i + 1].Cd_a2),
            Algorithm::Lerp(airspeed, airspeed1, airspeed2, m_aeroCoefs[i].Cna, m_aeroCoefs[i + 1].Cna)};
}
