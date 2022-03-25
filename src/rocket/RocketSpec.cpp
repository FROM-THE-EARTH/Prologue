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

    RocketParam& param = rocketParam[index];

    param.length     = JsonUtils::GetValueExc<double>(pt, key + ".ref_len");
    param.diameter   = JsonUtils::GetValueExc<double>(pt, key + ".diam");
    param.bottomArea = param.diameter * param.diameter * 0.25 * Constant::PI;

    param.CGLengthInitial = JsonUtils::GetValueExc<double>(pt, key + ".CGlen_i");
    param.CGLengthFinal   = JsonUtils::GetValueExc<double>(pt, key + ".CGlen_f");

    param.massInitial = JsonUtils::GetValueExc<double>(pt, key + ".mass_i");
    param.massFinal   = JsonUtils::GetValueExc<double>(pt, key + ".mass_f");

    param.rollingMomentInertiaInitial = JsonUtils::GetValueExc<double>(pt, key + ".Iyz_i");
    param.rollingMomentInertiaFinal   = JsonUtils::GetValueExc<double>(pt, key + ".Iyz_f");

    param.Cmq = JsonUtils::GetValueExc<double>(pt, key + ".Cmq");

    param.parachute.push_back(Parachute());
    param.parachute[0].terminalVelocity = JsonUtils::GetValue<double>(pt, key + ".vel_1st");
    param.parachute[0].openingType      = JsonUtils::GetValue<int>(pt, key + ".op_type_1st");
    param.parachute[0].openingTime      = JsonUtils::GetValue<double>(pt, key + ".op_time_1st");
    param.parachute[0].delayTime        = JsonUtils::GetValue<double>(pt, key + ".delay_time_1st");

    if (param.parachute[0].terminalVelocity == 0.0) {
        m_existInfCd = true;
        CommandLine::PrintInfo(PrintInfoType::Warning,
                               (std::string("Rocket: ") + key).c_str(),
                               "Terminal velocity is undefined.",
                               "Parachute Cd value is automatically calculated.");
    } else {
        param.parachute[0].Cd = CalcParachuteCd(param.massFinal, param.parachute[0].terminalVelocity);
    }

    param.engine.loadThrustData(JsonUtils::GetValue<std::string>(pt, key + ".motor_file"));
    param.airspeedParam.loadParam(JsonUtils::GetValue<std::string>(pt, key + ".airspeed_param_file"));
    if (!param.airspeedParam.exist()) {
        CommandLine::PrintInfo(
            PrintInfoType::Information, ("Rocket: " + key).c_str(), "Airspeed param is set from JSON");
        param.airspeedParam.setParam(JsonUtils::GetValueExc<double>(pt, key + ".CPlen"),
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
