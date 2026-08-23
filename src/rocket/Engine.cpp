// ------------------------------------------------
// Engine.hppの実装
// ------------------------------------------------

#include "Engine.hpp"

#include <algorithm>
#include <utility>

#include "math/Interpolation.hpp"

size_t getLowerIndex(const std::vector<ThrustData>& thrust, double time) {
    const auto it = std::lower_bound(
        thrust.begin() + 1, thrust.end() - 1, ThrustData{.time = time}, [](const ThrustData& t1, const ThrustData& t2) {
            return t1.time < t2.time;
        });
    return std::distance(thrust.begin(), it) - 1;
}

Engine::Engine(std::vector<ThrustData> thrustData) :
    m_thrustData(std::move(thrustData)), m_exist(!m_thrustData.empty()) {}

double Engine::thrustAt(double time, double pressure) const {
    if (!m_exist || time < 0.0 || time > m_thrustData[m_thrustData.size() - 1].time) {
        return 0;
    }

    const size_t i = getLowerIndex(m_thrustData, time);

    const double time1   = m_thrustData[i].time;
    const double time2   = m_thrustData[i + 1].time;
    const double thrust1 = m_thrustData[i].thrust;
    const double thrust2 = m_thrustData[i + 1].thrust;

    const auto thrust = Interpolation::Linear(time, time1, time2, thrust1, thrust2);

    return thrust + (m_thrustMeasuredPressure - pressure) * m_nozzleArea;
}
