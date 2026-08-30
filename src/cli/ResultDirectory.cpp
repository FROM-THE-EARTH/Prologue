// ------------------------------------------------
// ResultDirectory.hppの実装
// ------------------------------------------------

#include "ResultDirectory.hpp"

#include <filesystem>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace cli::ResultDirectory {
    std::string BuildName(const std::string& specificationName,
                          SimulationMode mode,
                          const SimulationInput& input,
                          const std::string& measuredWindFilename) {
        std::string name = specificationName + "[";
        switch (input.solver.wind.type) {
        case WindModelType::Real:
            name += "(" + std::filesystem::path(measuredWindFilename).stem().string() + ")";
            break;
        case WindModelType::Original:
            name += "original";
            break;
        case WindModelType::OnlyPowerLow:
            name += "powerlow";
            break;
        case WindModelType::NoWind:
            name += "nowind";
            break;
        }

        if (input.solver.wind.type != WindModelType::Real) {
            name += mode == SimulationMode::Scatter ? "_scatter" : "_detail";
        }
        name += input.run.trajectoryMode == TrajectoryMode::Parachute ? "_para" : "_traj";
        name += "]";

        if (mode == SimulationMode::Detail && input.solver.wind.type != WindModelType::Real
            && input.solver.wind.type != WindModelType::NoWind) {
            std::ostringstream condition;
            condition << std::fixed << std::setprecision(2)
                      << input.run.windSpeed << "ms, " << input.run.windDirection.degrees << "deg";
            name += "[" + condition.str() + "]";
        }
        return name;
    }

    std::filesystem::path Create(const std::string& directoryName) {
        const std::filesystem::path directory = std::filesystem::path("result") / directoryName;
        std::error_code error;
        std::filesystem::create_directories(directory, error);
        if (error) {
            throw std::runtime_error{"Failed to create result directory: " + directory.string()};
        }
        return directory;
    }
}
