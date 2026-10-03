#include "ApplicationSettingsReader.hpp"

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <fstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "utils/JsonUtils.hpp"

namespace ApplicationSettingsReader {
    namespace {
        size_t ReadThreadCount(const boost::property_tree::ptree& json,
                               std::vector<InputIO::Diagnostic>& diagnostics) {
            const int count = JsonUtils::GetValueExc<int>(json, "processing.multi_thread_count");

            if (count < 1) {
                diagnostics.push_back(
                    {.level = InputIO::DiagnosticLevel::Warning,
                     .lines = {"Specified thread count is too low",
                               "Thread count is automatically set to 1."}});
                return 1;
            }

            const size_t limit = std::thread::hardware_concurrency();

            if (limit == 0) {
                diagnostics.push_back(
                    {.level = InputIO::DiagnosticLevel::Error,
                     .lines = {"Could not get hardware concurrency", "Thread count is automatically set to 1."}});
                return 1;
            }
            if (static_cast<size_t>(count) > limit) {
                diagnostics.push_back(
                    {.level = InputIO::DiagnosticLevel::Warning,
                     .lines = {"Specified thread count exceeds the number that the machine can run.", "Thread count is automatically set to " + std::to_string(limit) + "."}});
                return limit;
            }
            return static_cast<size_t>(count);
        }

        WindModelType ReadWindModelType(const boost::property_tree::ptree& json) {
            const std::string type = JsonUtils::GetValueExc<std::string>(json, "wind_model.type");

            if (type == "real")
				return WindModelType::Real;
            if (type == "original")
				return WindModelType::Original;
            if (type == "only_powerlow")
				return WindModelType::OnlyPowerLow;
            if (type == "no_wind")
				return WindModelType::NoWind;

            throw std::runtime_error{"In prologue.settings.json: wind_model.type: \"" + type
                                     + "\" is invalid. Set \"real\", \"original\", \"only_powerlow\" or \"no_wind\"."};
        }

        int ReadPrecision(const boost::property_tree::ptree& json,
                          std::vector<InputIO::Diagnostic>& diagnostics) {
            const int precision = JsonUtils::GetValueExc<int>(json, "result.precision");
            if (precision >= 0) return precision;
            diagnostics.push_back(
                {.level = InputIO::DiagnosticLevel::Warning,
                 .lines = {"Result precision is set to the default value of 8."}});
            return 8;
        }

        unsigned int ReadStepSaveInterval(const boost::property_tree::ptree& json,
                                          std::vector<InputIO::Diagnostic>& diagnostics) {
            const int interval = JsonUtils::GetValueExc<int>(json, "result.step_save_interval");
            if (interval >= 1) return static_cast<unsigned int>(interval);
            diagnostics.push_back(
                {.level = InputIO::DiagnosticLevel::Warning,
                 .lines = {"Step save interval is set to the default value of 10."}});
            return 10;
        }
    }

    InputIO::ReadResult<ApplicationSettings> Read(const std::filesystem::path& settingsFile) {
        boost::property_tree::ptree json;
        std::ifstream input(settingsFile, std::ios::binary);
        if (!input.is_open()) {
            throw std::runtime_error{"Failed to open settings file: " + settingsFile.string()};
        }
        boost::property_tree::read_json(input, json);

        ApplicationSettings settings;
        std::vector<InputIO::Diagnostic> diagnostics;

        settings.execution.multiThread = JsonUtils::GetValueExc<bool>(json, "processing.multi_thread");
        settings.execution.threadCount = ReadThreadCount(json, diagnostics);
        settings.solver.timeStep = JsonUtils::GetValueExc<double>(json, "simulation.dt");
        settings.scatter.windSpeedMin = JsonUtils::GetValueExc<double>(json, "simulation.scatter.wind_speed_min");
        settings.scatter.windSpeedMax = JsonUtils::GetValueExc<double>(json, "simulation.scatter.wind_speed_max");
        settings.scatter.windDirectionInterval =
            JsonUtils::GetValueExc<double>(json, "simulation.scatter.wind_dir_interval");
        settings.result.precision = ReadPrecision(json, diagnostics);
        settings.solver.resultStepSaveInterval = ReadStepSaveInterval(json, diagnostics);
        settings.solver.wind.powerConstant = JsonUtils::GetValueExc<double>(json, "wind_model.power_constant");
        settings.solver.wind.powerLowBaseAltitude =
            JsonUtils::GetValueExc<double>(json, "wind_model.power_low_base_alt");
        settings.solver.wind.type = ReadWindModelType(json);
        settings.measuredWindFilename = JsonUtils::GetValueExc<std::string>(json, "wind_model.realdata_filename");
        settings.solver.atmosphere.basePressure =
            JsonUtils::GetValueExc<double>(json, "atmosphere.base_pressure_pascal");
        settings.solver.atmosphere.baseTemperature =
            JsonUtils::GetValueExc<double>(json, "atmosphere.base_temperature_celsius");

        return {.value = std::move(settings), .diagnostics = std::move(diagnostics)};
    }
}
