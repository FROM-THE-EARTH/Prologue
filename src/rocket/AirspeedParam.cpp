#include "AirspeedParam.hpp"

#include <fstream>

#include "math/Algorithm.hpp"

size_t search(const std::vector<VsAirspeed>& vs, double airSpeed, size_t begin, size_t end) {
    if (begin == end || begin == end - 1) {
        return begin;
    }

    size_t mid = begin + (end - begin) / 2;
    if (vs[mid].airSpeed == airSpeed) {
        return mid;
    } else if (vs[mid].airSpeed > airSpeed) {
        return search(vs, airSpeed, begin, mid);
    } else {
        return search(vs, airSpeed, mid, end - 1);
    }
}

size_t getLowerIndex(const std::vector<VsAirspeed>& vs, double airSpeed) {
    size_t index = vs.size() / 2;
    if (vs[index].airSpeed == airSpeed) {
        return index;
    } else if (vs[index].airSpeed > airSpeed) {
        return search(vs, airSpeed, 0, index);
    } else {
        return search(vs, airSpeed, index, vs.size() - 1);
    }
}

void AirspeedParam::loadParam(const std::string& filename) {
    std::fstream fs("input/airspeed_param/" + filename);

    if (!fs.is_open()) {
        return;
    }

    char header[1024];
    fs.getline(header, 1024);
    size_t i = 0;
    char c;
    std::string dummy;
    while (!fs.eof()) {
        m_vsAirspeed.push_back(VsAirspeed());
        fs >> m_vsAirspeed[i].airSpeed >> c >> m_vsAirspeed[i].Cp >> c >> m_vsAirspeed[i].Cp_a >> c
            >> m_vsAirspeed[i].Cd >> c >> m_vsAirspeed[i].Cd_a2 >> c >> m_vsAirspeed[i].Cna;
        i++;
    }
    if (m_vsAirspeed[m_vsAirspeed.size() - 1] == VsAirspeed()) {
        m_vsAirspeed.pop_back();
    }

    fs.close();

    m_exist = true;
}

void AirspeedParam::update(double airSpeed) {
    if (m_vsAirspeed.size() == 1) {
        m_param = {airSpeed,
                   m_vsAirspeed[0].Cp,
                   m_vsAirspeed[0].Cp_a,
                   m_vsAirspeed[0].Cd,
                   m_vsAirspeed[0].Cd_a2,
                   m_vsAirspeed[0].Cna};
        return;
    }

    const size_t i = getLowerIndex(m_vsAirspeed, airSpeed);

    const double airSpeed1 = m_vsAirspeed[i].airSpeed;
    const double airSpeed2 = m_vsAirspeed[i + 1].airSpeed;

    if (airSpeed < airSpeed1) {
        m_param = {airSpeed,
                   m_vsAirspeed[i].Cp,
                   m_vsAirspeed[i].Cp_a,
                   m_vsAirspeed[i].Cd,
                   m_vsAirspeed[i].Cd_a2,
                   m_vsAirspeed[i].Cna};
        return;
    }

    if (airSpeed > airSpeed2) {
        m_param = {airSpeed,
                   m_vsAirspeed[i + 1].Cp,
                   m_vsAirspeed[i + 1].Cp_a,
                   m_vsAirspeed[i + 1].Cd,
                   m_vsAirspeed[i + 1].Cd_a2,
                   m_vsAirspeed[i + 1].Cna};
        return;
    }

    m_param = {airSpeed,
               Algorithm::Lerp(airSpeed, airSpeed1, airSpeed2, m_vsAirspeed[i].Cp, m_vsAirspeed[i + 1].Cp),
               Algorithm::Lerp(airSpeed, airSpeed1, airSpeed2, m_vsAirspeed[i].Cp_a, m_vsAirspeed[i + 1].Cp_a),
               Algorithm::Lerp(airSpeed, airSpeed1, airSpeed2, m_vsAirspeed[i].Cd, m_vsAirspeed[i + 1].Cd),
               Algorithm::Lerp(airSpeed, airSpeed1, airSpeed2, m_vsAirspeed[i].Cd_a2, m_vsAirspeed[i + 1].Cd_a2),
               Algorithm::Lerp(airSpeed, airSpeed1, airSpeed2, m_vsAirspeed[i].Cna, m_vsAirspeed[i + 1].Cna)};
}
