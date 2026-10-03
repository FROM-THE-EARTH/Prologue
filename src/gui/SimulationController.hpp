#pragma once

#include <filesystem>
#include <memory>
#include <vector>

#include <QString>

#include "config/ApplicationSettings.hpp"
#include "core/SimulationInput.hpp"
#include "gui/RunResults.hpp"
#include "gui/map/MapWidget.hpp"
#include "project/ProjectDocument.hpp"

namespace gui {
    // A snapshot copied from the editor before starting a worker thread.
    struct RunRequest {
        project::Document document;
        ApplicationSettings settings;
        SimulationRunSettings run;
        SimulationMode mode = SimulationMode::Detail;
        std::filesystem::path projectFile;
        std::filesystem::path settingsFile;
    };

    struct RunOutcome {
        std::shared_ptr<const RunResults> results;
        QString error;
        std::vector<MapSeries> mapSeries;
    };

    // Runs the shared C++ APIs directly; no CLI process or intermediate input file.
    RunOutcome RunSimulation(const RunRequest& request);
    void SaveResults(const RunResults& results, const std::filesystem::path& directory, int precision);
}
