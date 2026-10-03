#include "SimulationController.hpp"

#include <stdexcept>
#include <utility>

#include "io/FileProjectResourceProvider.hpp"
#include "io/MeasuredWindProfileReader.hpp"
#include "project/SimulationInputBuilder.hpp"
#include "gui/MapSeriesBuilder.hpp"
#include "result/ResultSaver.hpp"
#include "runner/SimulationRunner.hpp"

namespace gui {
    RunOutcome RunSimulation(const RunRequest& request) {
        try {
            const ProjectIO::FileProjectResourceProvider resources(request.projectFile.parent_path());
            auto input = project::BuildSimulationInput(request.document, resources);
            input.solver = request.settings.solver;
            input.run = request.run;

            // Match the CLI wind-reference convention and the measured-wind file root.
            input.run.windDirection.reference = DirectionReference::MagneticNorth;
            if (input.solver.wind.type == WindModelType::Real) {
                const auto utf8 = request.settings.measuredWindFilename;
                const std::filesystem::path filename(std::u8string(utf8.begin(), utf8.end()));
                input.solver.wind.measuredProfile = MeasuredWindProfileReader::Read(
                    request.settingsFile.parent_path() / "input/wind" / filename);
            }
            if (request.mode == SimulationMode::Scatter
                && (input.solver.wind.type == WindModelType::Real || input.solver.wind.type == WindModelType::NoWind)) {
                throw std::invalid_argument{"Measured wind and no-wind models support detail mode, as in the CLI."};
            }
            if (input.solver.wind.type == WindModelType::Real || input.solver.wind.type == WindModelType::NoWind) {
                input.run.windSpeed = 0.0;
                input.run.windDirection.degrees = 0.0;
            }

            auto result = std::make_shared<RunResults>();
            result->environment = input.environment;
            result->mode = request.mode;
            if (request.mode == SimulationMode::Detail) {
                result->simulations.emplace_back(SimulationRunner::RunDetail(input));
            } else {
                result->simulations = SimulationRunner::RunScatter(
                    input, request.settings.scatter, request.settings.execution);
            }
            auto mapSeries = BuildMapSeries(*result);
            return {.results = std::move(result), .mapSeries = std::move(mapSeries)};
        } catch (const std::exception& error) {
            return {.error = QString::fromUtf8(error.what())};
        }
    }

    void SaveResults(const RunResults& results, const std::filesystem::path& directory, int precision) {
        // Files retain the CLI names, columns, precision and KML generation.
        if (results.mode == SimulationMode::Detail) {
            if (results.simulations.size() != 1) {
                throw std::invalid_argument{"A detail result must contain exactly one simulation."};
            }
            ResultSaver::SaveDetail(directory, results.simulations.front(), results.environment, precision);
        } else {
            ResultSaver::SaveScatter(directory, results.simulations, results.environment, precision);
        }
    }
}
