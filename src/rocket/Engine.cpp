#include "Engine.hpp"

#include <fstream>
#include <iostream>

size_t search(const std::vector<ThrustData>& thrust, double time, size_t begin, size_t end) {
    if (begin == end || begin == end - 1) {
        return begin;
    }

    size_t mid = begin + (end - begin) / 2;
    if (thrust[mid].time == time) {
        return mid;
    } else if (thrust[mid].time > time) {
        return search(thrust, time, begin, mid);
    } else {
        return search(thrust, time, mid, end - 1);
    }
}

size_t getLowerIndex(const std::vector<ThrustData>& thrust, double time) {
    /*size_t index = thrust.size() / 2;
    if (thrust[index].time == time) {
            return index;
    }
    else if (thrust[index].time > time) {
            return search(thrust, time, 0, index);
    }
    else {
            return search(thrust, time, index, thrust.size() - 1);
    }*/
    for (size_t i = 1; i < thrust.size() - 1; i++) {
        if (thrust[i].time > time) {
            return i - 1;
        }
    }

    return thrust.size() - 2;
}

bool Engine::loadThrustData(const std::string& filename) {
    std::fstream fs("input/thrust/" + filename);

    if (!fs.is_open()) {
        return false;
    }

    char dummy[1024];

    while (1) {
        char c = static_cast<char>(fs.get());
        if (c < '0' || c > '9') {
            fs.getline(dummy, 1024);
        } else {
            fs.unget();
            break;
        }
    }

    while (!fs.eof()) {
        ThrustData thrustdata;
        fs >> thrustdata.time >> thrustdata.thrust;
        m_thrustData.push_back(thrustdata);
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

double Engine::thrustAt(double time) const {
    if (!m_exist || time <= 0.0 || time > m_thrustData[m_thrustData.size() - 1].time) {
        return 0;
    }

    const size_t i = getLowerIndex(m_thrustData, time);

    const double time1   = m_thrustData[i].time;
    const double time2   = m_thrustData[i + 1].time;
    const double thrust1 = m_thrustData[i].thrust;
    const double thrust2 = m_thrustData[i + 1].thrust;

    const double grad = (thrust2 - thrust1) / (time2 - time1);

    const double thrust = thrust1 + grad * (time - time1);

    return thrust;
}
