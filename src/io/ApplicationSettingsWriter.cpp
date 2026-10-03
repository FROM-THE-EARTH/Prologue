#include "ApplicationSettingsWriter.hpp"

#include <cmath>
#include <fstream>
#include <iomanip>
#include <limits>
#include <locale>
#include <sstream>
#include <stdexcept>

namespace {
    const char* WindName(WindModelType type) {
        switch (type) {
        case WindModelType::Real: return "real";
        case WindModelType::Original: return "original";
        case WindModelType::OnlyPowerLow: return "only_powerlow";
        case WindModelType::NoWind: return "no_wind";
        }
        throw std::invalid_argument{"Invalid wind model type."};
    }

    void WriteString(std::ostream& output, const std::string& value) {
        output << '"';
        for (const unsigned char character : value) {
            switch (character) {
            case '"': output << "\\\""; break;
            case '\\': output << "\\\\"; break;
            case '\n': output << "\\n"; break;
            case '\r': output << "\\r"; break;
            case '\t': output << "\\t"; break;
            default:
                if (character < 0x20) {
                    output << "\\u" << std::hex << std::setw(4) << std::setfill('0')
                           << static_cast<int>(character) << std::dec << std::setfill(' ');
                } else {
                    output << static_cast<char>(character);
                }
            }
        }
        output << '"';
    }

    void RequireFinite(const ApplicationSettings& settings) {
        const double numbers[] = {
            settings.solver.timeStep, settings.scatter.windSpeedMin, settings.scatter.windSpeedMax,
            settings.scatter.windDirectionInterval, settings.solver.wind.powerConstant,
            settings.solver.wind.powerLowBaseAltitude, settings.solver.atmosphere.basePressure,
            settings.solver.atmosphere.baseTemperature,
        };
        for (const double number : numbers) {
            if (!std::isfinite(number)) {
                throw std::invalid_argument{"Settings JSON cannot contain a non-finite number."};
            }
        }
    }
}

namespace ApplicationSettingsWriter {
    std::string Serialize(const ApplicationSettings& settings) {
        RequireFinite(settings);
        std::ostringstream output;
        output.imbue(std::locale::classic());
        output << std::setprecision(std::numeric_limits<double>::max_digits10) << std::boolalpha;
        output << "{\n  \"processing\": {\n    \"multi_thread\": " << settings.execution.multiThread
               << ",\n    \"multi_thread_count\": " << settings.execution.threadCount
               << "\n  },\n  \"simulation\": {\n    \"dt\": " << settings.solver.timeStep
               << ",\n    \"scatter\": {\n      \"wind_speed_min\": " << settings.scatter.windSpeedMin
               << ",\n      \"wind_speed_max\": " << settings.scatter.windSpeedMax
               << ",\n      \"wind_dir_interval\": " << settings.scatter.windDirectionInterval
               << "\n    }\n  },\n  \"result\": {\n    \"precision\": " << settings.result.precision
               << ",\n    \"step_save_interval\": " << settings.solver.resultStepSaveInterval
               << "\n  },\n  \"wind_model\": {\n    \"power_constant\": " << settings.solver.wind.powerConstant
               << ",\n    \"power_low_base_alt\": " << settings.solver.wind.powerLowBaseAltitude
               << ",\n    \"type\": ";
        WriteString(output, WindName(settings.solver.wind.type));
        output << ",\n    \"realdata_filename\": ";
        WriteString(output, settings.measuredWindFilename);
        output << "\n  },\n  \"atmosphere\": {\n    \"base_pressure_pascal\": "
               << settings.solver.atmosphere.basePressure
               << ",\n    \"base_temperature_celsius\": " << settings.solver.atmosphere.baseTemperature
               << "\n  }\n}\n";

        return output.str();
    }

    void Write(const std::filesystem::path& file, const ApplicationSettings& settings) {
        const std::string serialized = Serialize(settings);
        std::ofstream destination(file, std::ios::binary);
        if (!destination.is_open()) {
            throw std::runtime_error{"Failed to open settings file for writing: " + file.string()};
        }
        destination << serialized;
        destination.close();
        if (!destination) {
            throw std::runtime_error{"Failed to write settings file: " + file.string()};
        }
    }
}
