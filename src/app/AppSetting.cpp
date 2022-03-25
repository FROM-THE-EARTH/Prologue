#include "AppSetting.hpp"

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <iostream>

#include "CommandLine.hpp"
#include "utils/JsonUtils.hpp"

template <typename T>
T GetValueExc(const boost::property_tree::ptree& pt, const std::string& key) {
    if (!JsonUtils::HasValue<T>(pt, key)) {
        const std::string k = "key: " + key;
        CommandLine::PrintInfo(
            PrintInfoType::Error, "In prologue.settings.json", k.c_str(), "This key does not have value");
        throw 0;
    }

    return JsonUtils::GetValue<T>(pt, key);
}

namespace AppSetting {
    namespace Internal {
        bool initialized = false;
        boost::property_tree::ptree pt;

        template <typename T>
        T InitValue(const std::string& key) {
            if (!initialized) {
                boost::property_tree::read_json("prologue.settings.json", pt);
                initialized = true;
            }

            return GetValueExc<T>(pt, key);
        }

        WindModelType InitWindModelType() {
            const std::string windmodeltype = InitValue<std::string>("wind_model.type");

            if (windmodeltype == "real") {
                return WindModelType::Real;
            } else if (windmodeltype == "original") {
                return WindModelType::Original;
            } else if (windmodeltype == "only_powerlow") {
                return WindModelType::OnlyPowerLow;
            } else {
                CommandLine::PrintInfo(PrintInfoType::Error,
                                       "In prologue.settings.json",
                                       "wind_model.type",
                                       ("\"" + windmodeltype + "\" is invalid string.").c_str(),
                                       "Set \"real\", \"original\" or \"only_powerlow\"");
                throw 0;
            }
        }
    }

    const bool Processing::multiThread = Internal::InitValue<bool>("processing.multi_thread");

    const double Simulation::dt                  = Internal::InitValue<double>("simulation.dt");
    const double Simulation::detectPeakThreshold = Internal::InitValue<double>("simulation.detect_peak_threshold");
    const double Simulation::windSpeedMin        = Internal::InitValue<double>("simulation.scatter.wind_speed_min");
    const double Simulation::windSpeedMax        = Internal::InitValue<double>("simulation.scatter.wind_speed_max");
    const double Simulation::windDirInterval     = Internal::InitValue<double>("simulation.scatter.wind_dir_interval");

    const double WindModel::powerConstant         = Internal::InitValue<double>("wind_model.power_constant");
    const WindModelType WindModel::type           = Internal::InitWindModelType();
    const std::string WindModel::realdataFilename = Internal::InitValue<std::string>("wind_model.realdata_filename");
}
