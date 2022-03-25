#include "RocketSpec.hpp"

#include <cmath>
#include <iostream>
#include <string>

#include "app/CommandLine.hpp"
#include "misc/Constant.hpp"
#include "utils/JsonUtils.hpp"

constexpr int multipleRocketNum                          = 3;
constexpr const char* RocketParamList[multipleRocketNum] = {"rocket1", "rocket2", "rocket3"};

double CalcParachuteCd(double massFinal, double terminalVelocity) {
    return massFinal * Constant::G / (0.5 * 1.25 * std::pow(terminalVelocity, 2) * 1.0);
}

void RocketSpec::setEnvironment(const boost::property_tree::ptree& pt) {
    env.place         = JsonUtils::GetValue<std::string>(pt, "environment.place");
    env.railLength    = JsonUtils::GetValueExc<double>(pt, "environment.rail_len");
    env.railAzimuth   = JsonUtils::GetValueExc<double>(pt, "environment.rail_azi");
    env.railElevation = JsonUtils::GetValueExc<double>(pt, "environment.rail_elev");
}

void RocketSpec::setExtraInfo(const boost::property_tree::ptree& pt) {
    info.teamName       = JsonUtils::GetValue<std::string>(pt, "info.TEAM");
    info.rocketName     = JsonUtils::GetValue<std::string>(pt, "info.NAME");
    info.experimentDate = JsonUtils::GetValue<std::string>(pt, "info.DATE");
    info.version        = JsonUtils::GetValue<std::string>(pt, "info.VERSION");
}

void RocketSpec::setRocketParam(const boost::property_tree::ptree& pt, size_t index) {
    const std::string key = RocketParamList[index];

    rocketParam.push_back({});

    rocketParam[index].length     = JsonUtils::GetValueExc<double>(pt, key + ".ref_len");
    rocketParam[index].diameter   = JsonUtils::GetValueExc<double>(pt, key + ".diam");
    rocketParam[index].bottomArea = rocketParam[index].diameter * rocketParam[index].diameter * 0.25 * Constant::PI;

    rocketParam[index].CGLengthInitial = JsonUtils::GetValueExc<double>(pt, key + ".CGlen_i");
    rocketParam[index].CGLengthFinal   = JsonUtils::GetValueExc<double>(pt, key + ".CGlen_f");

    rocketParam[index].massInitial = JsonUtils::GetValueExc<double>(pt, key + ".mass_i");
    rocketParam[index].massFinal   = JsonUtils::GetValueExc<double>(pt, key + ".mass_f");

    rocketParam[index].rollingMomentInertiaInitial = JsonUtils::GetValueExc<double>(pt, key + ".Iyz_i");
    rocketParam[index].rollingMomentInertiaFinal   = JsonUtils::GetValueExc<double>(pt, key + ".Iyz_f");

    rocketParam[index].Cmq = JsonUtils::GetValueExc<double>(pt, key + ".Cmq");

    rocketParam[index].parachute.push_back(Parachute());
    rocketParam[index].parachute[0].terminalVelocity = JsonUtils::GetValue<double>(pt, key + ".vel_1st");
    rocketParam[index].parachute[0].openingType      = JsonUtils::GetValue<int>(pt, key + ".op_type_1st");
    rocketParam[index].parachute[0].openingTime      = JsonUtils::GetValue<double>(pt, key + ".op_time_1st");
    rocketParam[index].parachute[0].delayTime        = JsonUtils::GetValue<double>(pt, key + ".delay_time_1st");

    if (rocketParam[index].parachute[0].terminalVelocity == 0.0) {
        m_existInfCd = true;
        CommandLine::PrintInfo(PrintInfoType::Warning,
                               (std::string("Rocket: ") + key).c_str(),
                               "Terminal velocity is undefined.",
                               "Parachute Cd value is automatically calculated.");
    } else {
        rocketParam[index].parachute[0].Cd =
            CalcParachuteCd(rocketParam[index].massFinal, rocketParam[index].parachute[0].terminalVelocity);
    }

    rocketParam[index].engine.loadThrustData(JsonUtils::GetValue<std::string>(pt, key + ".motor_file"));
    rocketParam[index].airspeedParam.loadParam(JsonUtils::GetValue<std::string>(pt, key + ".airspeed_param_file"));
    if (!rocketParam[index].airspeedParam.exist()) {
        CommandLine::PrintInfo(
            PrintInfoType::Information, ("Rocket: " + key).c_str(), "Airspeed param is set from JSON");
        rocketParam[index].airspeedParam.setParam(JsonUtils::GetValueExc<double>(pt, key + ".CPlen"),
                                                  JsonUtils::GetValue<double>(pt, key + ".CP_alpha"),
                                                  JsonUtils::GetValueExc<double>(pt, key + ".Cd"),
                                                  JsonUtils::GetValue<double>(pt, key + ".Cd_alpha2"),
                                                  JsonUtils::GetValueExc<double>(pt, key + ".Cna"));
    } else {
        CommandLine::PrintInfo(
            PrintInfoType::Information, ("Rocket: " + key).c_str(), "Airspeed param is set from CSV");
    }
}

void RocketSpec::setInfParachuteCd() {
    for (size_t i = 0; i < rocketParam.size(); i++) {
        if (rocketParam[i].parachute[0].Cd == 0) {
            for (int j = (int)rocketParam.size() - 1; j >= 0; j--) {
                rocketParam[i].parachute[0].Cd += rocketParam[j].parachute[0].Cd;
            }
        }
    }
}

void RocketSpec::initialize(const std::string& filename) {
    boost::property_tree::ptree pt;
    boost::property_tree::read_json("input/json/" + filename, pt);

    m_existInfCd = false;

    /* Read once */
    setExtraInfo(pt);
    setEnvironment(pt);

    /* Read once or more */
    {
        const bool isMultipleRocket = IsMultipleRocket(filename);
        size_t i                    = 0;
        do {
            setRocketParam(pt, i);
            i++;
        } while (isMultipleRocket && i < multipleRocketNum);
    }

    // Set parachute Cd (Multiple rocket)
    if (m_existInfCd) {
        setInfParachuteCd();
    }
}

bool RocketSpec::IsMultipleRocket(const std::string& filename) {
    boost::property_tree::ptree pt;
    boost::property_tree::read_json("input/json/" + filename, pt);

    return JsonUtils::Exist(pt, RocketParamList[1]);
}
