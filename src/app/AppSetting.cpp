#include "AppSetting.hpp"

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <iostream>

#include "CommandLine.hpp"
#include "utils/JsonUtils.hpp"

Setting AppSetting::setting_;

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

bool AppSetting::Initialize() {
    boost::property_tree::ptree pt;
    boost::property_tree::read_json("prologue.settings.json", pt);

    if (pt.empty()) {
        return false;
    }

    try {
        // processing
        setting_.processing.multiThread = GetValueExc<bool>(pt, "processing.multi_thread");

        // simulation
        setting_.simulation.dt                  = GetValueExc<double>(pt, "simulation.dt");
        setting_.simulation.detectPeakThreshold = GetValueExc<double>(pt, "simulation.detect_peak_threshold");
        // scatter
        setting_.simulation.windSpeedMin    = GetValueExc<double>(pt, "simulation.scatter.wind_speed_min");
        setting_.simulation.windSpeedMax    = GetValueExc<double>(pt, "simulation.scatter.wind_speed_max");
        setting_.simulation.windDirInterval = GetValueExc<double>(pt, "simulation.scatter.wind_dir_interval");

        // wind model
        setting_.windModel.powerConstant = GetValueExc<double>(pt, "wind_model.power_constant");
        const std::string windmodeltype  = GetValueExc<std::string>(pt, "wind_model.type");
        if (windmodeltype == "real") {
            setting_.windModel.type = WindModelType::Real;
        } else if (windmodeltype == "original") {
            setting_.windModel.type = WindModelType::Original;
        } else if (windmodeltype == "only_powerlow") {
            setting_.windModel.type = WindModelType::OnlyPowerLow;
        } else {
            throw 0;
        }
        setting_.windModel.realdataFilename = GetValueExc<std::string>(pt, "wind_model.realdata_filename");
    } catch (...) {
        return false;
    }

    return true;
}
