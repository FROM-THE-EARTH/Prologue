#include "RocketSpec.hpp"

#include <cmath>
#include <iostream>
#include <string>

#include "app/CommandLine.hpp"
#include "misc/Constant.hpp"
#include "utils/JsonUtils.hpp"

constexpr size_t AvailableBodyCount                = 3;
constexpr const char* BodyList[AvailableBodyCount] = {"rocket1", "rocket2", "rocket3"};

double CalcParachuteCd(double massFinal, double terminalVelocity) {
    return massFinal * Constant::G / (0.5 * 1.25 * std::pow(terminalVelocity, 2) * 1.0);
}

void RocketSpecification::setBodySpecification(const boost::property_tree::ptree& pt, size_t index) {
    const std::string key = BodyList[index];

    bodySpec.emplace_back();

    BodySpecification& spec = bodySpec[index];

    spec.length     = JsonUtils::GetValueExc<double>(pt, key + ".ref_len");
    spec.diameter   = JsonUtils::GetValueExc<double>(pt, key + ".diam");
    spec.bottomArea = spec.diameter * spec.diameter * 0.25 * Constant::PI;

    spec.CGLengthInitial = JsonUtils::GetValueExc<double>(pt, key + ".CGlen_i");
    spec.CGLengthFinal   = JsonUtils::GetValueExc<double>(pt, key + ".CGlen_f");

    spec.massInitial = JsonUtils::GetValueExc<double>(pt, key + ".mass_i");
    spec.massFinal   = JsonUtils::GetValueExc<double>(pt, key + ".mass_f");

    spec.rollingMomentInertiaInitial = JsonUtils::GetValueExc<double>(pt, key + ".Iyz_i");
    spec.rollingMomentInertiaFinal   = JsonUtils::GetValueExc<double>(pt, key + ".Iyz_f");

    spec.Cmq = JsonUtils::GetValueExc<double>(pt, key + ".Cmq");

    spec.parachutes.emplace_back(Parachute());
    spec.parachutes[0].terminalVelocity = JsonUtils::GetValue<double>(pt, key + ".vel_1st");
    spec.parachutes[0].openingType =
        static_cast<ParachuteOpeningType>(JsonUtils::GetValue<int>(pt, key + ".op_type_1st"));
    spec.parachutes[0].openingTime = JsonUtils::GetValue<double>(pt, key + ".op_time_1st");
    spec.parachutes[0].delayTime   = JsonUtils::GetValue<double>(pt, key + ".delay_time_1st");

    if (spec.parachutes[0].terminalVelocity == 0.0) {
        m_existInfCd = true;
        CommandLine::PrintInfo(PrintInfoType::Warning,
                               "Rocket: " + key,
                               "Terminal velocity is undefined.",
                               "Parachute Cd value is automatically calculated.");
    } else {
        spec.parachutes[0].Cd = CalcParachuteCd(spec.massFinal, spec.parachutes[0].terminalVelocity);
    }

    // Initialize Engine
    spec.engine.loadThrustData(JsonUtils::GetValue<std::string>(pt, key + ".motor_file"));
    if (const auto result = JsonUtils::GetValueOpt<double>(pt, key + ".thrust_measured_pressure"); result.has_value()) {
        spec.engine.setThrustMeasuredPressure(result.value());
    }
    if (const auto result = JsonUtils::GetValueOpt<double>(pt, key + ".engine_nozzle_diameter"); result.has_value()) {
        spec.engine.setNozzleDiameter(result.value());
    }

    spec.aeroCoefStorage.init(JsonUtils::GetValue<std::string>(pt, key + ".aero_coef_file"));
    if (spec.aeroCoefStorage.isTimeSeriesSpec()) {
        CommandLine::PrintInfo(PrintInfoType::Information, "Rocket: " + key, "Aero coefficients are set from CSV");
    } else {
        CommandLine::PrintInfo(PrintInfoType::Information, "Rocket: " + key, "Aero coefficients are set from JSON");
        spec.aeroCoefStorage.init(JsonUtils::GetValueExc<double>(pt, key + ".CPlen"),
                                  JsonUtils::GetValue<double>(pt, key + ".CP_alpha"),
                                  JsonUtils::GetValueExc<double>(pt, key + ".Cd_i"),
                                  JsonUtils::GetValueExc<double>(pt, key + ".Cd_f"),
                                  JsonUtils::GetValue<double>(pt, key + ".Cd_alpha2"),
                                  JsonUtils::GetValueExc<double>(pt, key + ".Cna"));
    }
}

void RocketSpecification::setInfParachuteCd() {
    for (size_t i = 0; i < bodySpec.size(); i++) {
        if (bodySpec[i].parachutes[0].Cd == 0) {
            for (int j = (int)bodySpec.size() - 1; j >= 0; j--) {
                bodySpec[i].parachutes[0].Cd += bodySpec[j].parachutes[0].Cd;
            }
        }
    }
}

void RocketSpecification::initialize(const std::string& filename) {
    boost::property_tree::ptree pt;
    boost::property_tree::read_json("input/json/" + filename, pt);

    m_existInfCd = false;

    {
        const bool isMultipleRocket = IsMultipleRocket(filename);
        size_t i                    = 0;
        do {
            setBodySpecification(pt, i);
            i++;
        } while (isMultipleRocket && i < AvailableBodyCount);
    }

    // Set parachute Cd (Multiple rocket)
    if (m_existInfCd) {
        setInfParachuteCd();
    }
}

bool RocketSpecification::IsMultipleRocket(const std::string& filename) {
    boost::property_tree::ptree pt;
    boost::property_tree::read_json("input/json/" + filename, pt);
    return JsonUtils::Exist(pt, BodyList[1]);
}
