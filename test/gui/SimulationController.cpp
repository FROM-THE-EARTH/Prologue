#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <utility>
#include <vector>

#include <QTemporaryDir>

#include "gui/FileActions.hpp"
#include "gui/SimulationController.hpp"
#include "io/FileProjectResourceProvider.hpp"
#include "io/MeasuredWindProfileReader.hpp"
#include "io/ProjectJsonSerializer.hpp"
#include "project/SimulationInputBuilder.hpp"
#include "result/ResultSaver.hpp"
#include "runner/SimulationRunner.hpp"

namespace {
    std::filesystem::path Utf8Path(const std::u8string& value) {
        return std::filesystem::path(value);
    }

    struct TemporaryProject {
        QTemporaryDir temporaryDirectory;
        std::filesystem::path root;
        std::filesystem::path input = root / "input";
        std::filesystem::path settingsFile = root / "prologue.settings.json";

        TemporaryProject() {
            REQUIRE(temporaryDirectory.isValid());
            root = gui::FilePath(temporaryDirectory.path());
            input = root / "input";
            settingsFile = root / "prologue.settings.json";
            std::filesystem::create_directories(root);
            const auto sourceInput = std::filesystem::current_path().parent_path() / "application/input";
            for (const auto& directory : {"spec", "thrust", "aero_coef", "wind"}) {
                std::filesystem::create_directories(input / directory);
            }
            for (const auto& [source, destination] : {
                     std::pair{"spec/spec_single.json", "spec/spec_single.json"},
                     std::pair{"spec/spec_multi.json", "spec/spec_multi.json"},
                     std::pair{"thrust/Sample_G40-4W.txt", "thrust/Sample_G40-4W.txt"},
                     std::pair{"thrust/Sample_K240.txt", "thrust/Sample_K240.txt"},
                     std::pair{"aero_coef/sample.csv", "aero_coef/sample.csv"},
                     std::pair{"wind/sample.csv", "wind/sample.csv"}}) {
                std::filesystem::copy_file(sourceInput / source, input / destination);
            }
            std::ofstream(settingsFile) << "{}";
        }

        std::filesystem::path spec(const std::string& name) const {
        return input / "spec" / name;
        }
    };

    std::string ReadBytes(const std::filesystem::path& path) {
        std::ifstream file(path, std::ios::binary);
        return {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
    }

    std::vector<std::filesystem::path> RelativeFiles(const std::filesystem::path& directory) {
        std::vector<std::filesystem::path> result;
        for (const auto& entry : std::filesystem::recursive_directory_iterator(directory)) {
            if (entry.is_regular_file()) result.push_back(entry.path().lexically_relative(directory));
        }
        std::sort(result.begin(), result.end());
        return result;
    }

    void RequireSameFiles(const std::filesystem::path& first, const std::filesystem::path& second) {
        const auto firstFiles = RelativeFiles(first);
        const auto secondFiles = RelativeFiles(second);
        REQUIRE(firstFiles == secondFiles);
        for (const auto& relative : firstFiles) {
            INFO("Comparing output file " << relative.string());
            REQUIRE(ReadBytes(first / relative) == ReadBytes(second / relative));
        }
    }

    ApplicationSettings BaseSettings(WindModelType windType = WindModelType::NoWind) {
        ApplicationSettings settings;
        settings.execution = {.multiThread = false, .threadCount = 2};
        settings.scatter = {.windSpeedMin = 1.0, .windSpeedMax = 2.0, .windDirectionInterval = 90.0};
        settings.result.precision = 8;
        settings.solver.timeStep = 0.001;
        settings.solver.resultStepSaveInterval = 10;
        settings.solver.wind.type = windType;
        settings.solver.wind.powerConstant = 6.0;
        settings.solver.wind.powerLowBaseAltitude = 2.0;
        settings.measuredWindFilename = "sample.csv";
        return settings;
    }

    project::Document LoadDocument(const std::filesystem::path& projectFile) {
        return ProjectJsonSerializer::Load(projectFile);
    }

    SimulationInput BuildControllerInput(const gui::RunRequest& request) {
        const ProjectIO::FileProjectResourceProvider resources(request.projectFile.parent_path());
        auto input = project::BuildSimulationInput(request.document, resources);
        input.solver = request.settings.solver;
        input.run = request.run;
        input.run.windDirection.reference = DirectionReference::MagneticNorth;
        if (input.solver.wind.type == WindModelType::Real) {
            const auto utf8 = request.settings.measuredWindFilename;
            const std::filesystem::path filename(std::u8string(utf8.begin(), utf8.end()));
            input.solver.wind.measuredProfile = MeasuredWindProfileReader::Read(
                request.settingsFile.parent_path() / "input/wind" / filename);
        }
        if (input.solver.wind.type == WindModelType::Real || input.solver.wind.type == WindModelType::NoWind) {
            input.run.windSpeed = 0.0;
            input.run.windDirection.degrees = 0.0;
        }
        return input;
    }

    std::filesystem::path CompareDetail(const gui::RunRequest& request,
                                        const std::filesystem::path& outputRoot) {
        const auto outcome = gui::RunSimulation(request);
        REQUIRE(outcome.error.isEmpty());
        REQUIRE(outcome.results != nullptr);
        REQUIRE(outcome.results->mode == SimulationMode::Detail);
        REQUIRE(outcome.results->simulations.size() == 1);

        const auto input = BuildControllerInput(request);
        const auto directResult = SimulationRunner::RunDetail(input);
        gui::RunResults direct;
        direct.environment = input.environment;
        direct.mode = SimulationMode::Detail;
        direct.simulations.push_back(directResult);

        const auto controllerDirectory = outputRoot / "controller";
        const auto directDirectory = outputRoot / "direct";
        std::filesystem::create_directories(controllerDirectory);
        std::filesystem::create_directories(directDirectory);
        gui::SaveResults(*outcome.results, controllerDirectory, request.settings.result.precision);
        gui::SaveResults(direct, directDirectory, request.settings.result.precision);
        RequireSameFiles(controllerDirectory, directDirectory);
        return controllerDirectory;
    }

    std::filesystem::path CompareScatter(const gui::RunRequest& request,
                                         const std::filesystem::path& outputRoot) {
        const auto outcome = gui::RunSimulation(request);
        REQUIRE(outcome.error.isEmpty());
        REQUIRE(outcome.results != nullptr);
        REQUIRE(outcome.results->mode == SimulationMode::Scatter);
        REQUIRE(outcome.results->simulations.size() == 8);

        const auto input = BuildControllerInput(request);
        const auto directResults = SimulationRunner::RunScatter(
            input, request.settings.scatter, request.settings.execution);
        REQUIRE(directResults.size() == outcome.results->simulations.size());
        gui::RunResults direct;
        direct.environment = input.environment;
        direct.mode = SimulationMode::Scatter;
        direct.simulations = directResults;

        const auto controllerDirectory = outputRoot / "controller";
        const auto directDirectory = outputRoot / "direct";
        std::filesystem::create_directories(controllerDirectory);
        std::filesystem::create_directories(directDirectory);
        gui::SaveResults(*outcome.results, controllerDirectory, request.settings.result.precision);
        gui::SaveResults(direct, directDirectory, request.settings.result.precision);
        RequireSameFiles(controllerDirectory, directDirectory);
        return controllerDirectory;
    }

    gui::RunRequest MakeRequest(const TemporaryProject& temporary,
                                const std::filesystem::path& projectFile,
                                const project::Document& document,
                                SimulationMode mode,
                                ApplicationSettings settings,
                                TrajectoryMode trajectory = TrajectoryMode::Trajectory,
                                DetachType detach = DetachType::DoNotDeatch,
                                double detachTime = 0.0) {
        return {
            .document = document,
            .settings = std::move(settings),
            .run = {
                .trajectoryMode = trajectory,
                .detachType = detach,
                .detachTime = detachTime,
                .windSpeed = 5.0,
                .windDirection = {.degrees = 90.0, .reference = DirectionReference::TrueNorth},
            },
            .mode = mode,
            .projectFile = projectFile,
            .settingsFile = temporary.settingsFile,
        };
    }
}

TEST_CASE("GUI detail controller matches direct CLI-core runs for single and multi specs", "[gui][controller]") {
    TemporaryProject temporary;
    for (const auto& specName : {std::string("spec_single.json"), std::string("spec_multi.json")}) {
        const auto projectFile = temporary.spec(specName);
        const auto document = LoadDocument(projectFile);
        const auto isMulti = document.bodies.size() > 1;
        auto request = MakeRequest(temporary,
                                  projectFile,
                                  document,
                                  SimulationMode::Detail,
                                  BaseSettings(),
                                  isMulti ? TrajectoryMode::Parachute : TrajectoryMode::Trajectory,
                                  isMulti ? DetachType::Time : DetachType::DoNotDeatch,
                                  isMulti ? 1.5 : 0.0);
        const auto output = CompareDetail(request, temporary.root / (isMulti ? "multi" : "single"));
        REQUIRE(std::filesystem::exists(output / "summary.csv"));
        REQUIRE(std::filesystem::exists(output / "detail_body1.csv"));
    }
}

TEST_CASE("Japanese Save As rebases resources and atomically reloads a project with Japanese aero CSV", "[gui][files][unicode]") {
    TemporaryProject temporary;
    const auto sourceProject = temporary.spec("spec_single.json");
    auto document = LoadDocument(sourceProject);
    const auto sourceAero = std::filesystem::current_path().parent_path()
        / "application/input/aero_coef/sample.csv";
    const auto japaneseAero = temporary.input / "aero_coef" / Utf8Path(u8"性能 曲線.csv");
    std::filesystem::copy_file(sourceAero, japaneseAero, std::filesystem::copy_options::overwrite_existing);
    document.bodies[0].aerodynamics.coefficientFile = std::filesystem::path(
        std::u8string(u8"../aero_coef/性能 曲線.csv"));

    const auto request = MakeRequest(temporary,
                                     sourceProject,
                                     document,
                                     SimulationMode::Detail,
                                     BaseSettings());
    const auto sourceOutput = CompareDetail(request, temporary.root / "before-save-as");
    REQUIRE(std::filesystem::exists(sourceOutput / "detail_body1.csv"));

    const auto savedProject = temporary.root / Utf8Path(u8"保存先") / "project.json";
    std::filesystem::create_directories(savedProject.parent_path());
    auto rebased = document;
    gui::RebaseResources(rebased, sourceProject, savedProject);
    gui::WriteAtomically(gui::PathText(savedProject), ProjectJsonSerializer::Serialize(rebased));
    REQUIRE(std::filesystem::exists(savedProject));
    REQUIRE(gui::FilePath(gui::PathText(savedProject)) == savedProject);

    const auto restored = LoadDocument(savedProject);
    REQUIRE(restored.bodies[0].aerodynamics.coefficientFile
            == std::filesystem::path(std::u8string(u8"../input/aero_coef/性能 曲線.csv")));
    const auto savedRequest = MakeRequest(temporary,
                                          savedProject,
                                          restored,
                                          SimulationMode::Detail,
                                          BaseSettings());
    const auto savedOutput = CompareDetail(savedRequest, temporary.root / "after-save-as");
    RequireSameFiles(sourceOutput, savedOutput);
}

TEST_CASE("Scatter serial and parallel controller output matches the direct runner", "[gui][controller][scatter]") {
    TemporaryProject temporary;
    const auto projectFile = temporary.spec("spec_multi.json");
    const auto document = LoadDocument(projectFile);
    auto settings = BaseSettings(WindModelType::OnlyPowerLow);
    auto serialRequest = MakeRequest(temporary,
                                    projectFile,
                                    document,
                                    SimulationMode::Scatter,
                                    settings,
                                    TrajectoryMode::Parachute,
                                    DetachType::Time,
                                    1.5);
    serialRequest.settings.execution.multiThread = false;
    const auto serialOutput = CompareScatter(serialRequest, temporary.root / "scatter-serial");

    auto parallelRequest = serialRequest;
    parallelRequest.settings.execution.multiThread = true;
    parallelRequest.settings.execution.threadCount = 2;
    const auto parallelOutput = CompareScatter(parallelRequest, temporary.root / "scatter-parallel");
    RequireSameFiles(serialOutput, parallelOutput);
    REQUIRE(std::filesystem::exists(serialOutput / "summary.csv"));
    REQUIRE(std::filesystem::exists(serialOutput / "scatter_body2.kml"));
    REQUIRE(std::filesystem::exists(serialOutput / "scatter_body3.kml"));
}

TEST_CASE("Japanese measured wind filenames can be loaded for detail simulation", "[gui][controller][unicode]") {
    TemporaryProject temporary;
    const auto projectFile = temporary.spec("spec_single.json");
    const auto filename = std::u8string(u8"測定 風.csv");
    std::filesystem::copy_file(temporary.input / "wind/sample.csv", temporary.input / "wind" / std::filesystem::path(filename));
    auto settings = BaseSettings(WindModelType::Real);
    settings.measuredWindFilename = {filename.begin(), filename.end()};
    const auto request = MakeRequest(temporary, projectFile, LoadDocument(projectFile), SimulationMode::Detail, settings);
    CompareDetail(request, temporary.root / "measured-wind");
}

TEST_CASE("Controller reports invalid input and real-wind scatter mode as errors", "[gui][controller][validation]") {
    TemporaryProject temporary;
    const auto projectFile = temporary.spec("spec_single.json");
    auto document = LoadDocument(projectFile);
    auto invalidRequest = MakeRequest(temporary,
                                     projectFile,
                                     document,
                                     SimulationMode::Detail,
                                     BaseSettings());
    invalidRequest.document.environment.coordinateZone = 0;
    const auto invalid = gui::RunSimulation(invalidRequest);
    REQUIRE_FALSE(invalid.error.isEmpty());
    REQUIRE(invalid.results == nullptr);

    auto realSettings = BaseSettings(WindModelType::Real);
    auto scatterRequest = MakeRequest(temporary,
                                      projectFile,
                                      document,
                                      SimulationMode::Scatter,
                                      realSettings);
    const auto invalidMode = gui::RunSimulation(scatterRequest);
    REQUIRE_FALSE(invalidMode.error.isEmpty());
    REQUIRE(invalidMode.results == nullptr);
    REQUIRE(invalidMode.error.contains(QStringLiteral("support detail mode")));
}
