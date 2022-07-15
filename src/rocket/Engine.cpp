#include "Engine.hpp"

#include <fstream>
#include <iostream>

#include "math/Algorithm.hpp"

size_t getLowerIndex(const std::vector<ThrustData>& thrust, double time) {
    const auto it = std::lower_bound(
        thrust.begin() + 1, thrust.end() - 1, ThrustData{.time = time}, [](const ThrustData& t1, const ThrustData& t2) {
            return t1.time < t2.time;
        });
    return std::distance(thrust.begin(), it) - 1;
}

bool Engine::loadThrustData(const std::string& filename) {
    std::fstream fs("input/thrust/" + filename);

    if (!fs.is_open()) {
        return false;
    }

    char dummy[1024];

    while (1) {
        if (const char c = static_cast<char>(fs.get()); c < '0' || '9' < c) {
            fs.getline(dummy, 1024);
        } else {
            fs.unget();
            break;
        }
    }

    while (!fs.eof()) {
        ThrustData thrustdata;
        fs >> thrustdata.time >> thrustdata.thrust;
        m_thrustData.emplace_back(std::move(thrustdata));
    }

    if (m_thrustData[m_thrustData.size() - 1].time == 0.0) {
        m_thrustData.pop_back();
    }

    if (m_thrustData[0].time != 0.0) {
        m_thrustData.insert(m_thrustData.begin(), ThrustData{0.0, 0.0});
    }

    m_exist = true;
    return true;
}

double Engine::thrustAt(double time, double pressure) const {
    if (!m_exist || time < 0.0 || time > m_thrustData[m_thrustData.size() - 1].time) {
        return 0;
    }

    const size_t i = getLowerIndex(m_thrustData, time);

    const double time1   = m_thrustData[i].time;
    const double time2   = m_thrustData[i + 1].time;
    const double thrust1 = m_thrustData[i].thrust;
    const double thrust2 = m_thrustData[i + 1].thrust;

    const auto thrust = Algorithm::Lerp(time, time1, time2, thrust1, thrust2);

    return thrust + (m_thrustMeasuredPressure - pressure) * m_nozzleArea;
}
