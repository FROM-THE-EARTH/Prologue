#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "result/ResultSaver.hpp"

namespace {
    class ConsoleCapture {
        std::streambuf* m_original;
    public:
        explicit ConsoleCapture(std::ostream& replacement) : m_original(std::cout.rdbuf(replacement.rdbuf())) {}
        ~ConsoleCapture() { std::cout.rdbuf(m_original); }
    };

    class RecordingObserver final : public ResultSaveObserver {
    public:
        std::vector<std::pair<size_t, size_t>> startedBodies;
        std::vector<std::tuple<size_t, size_t, size_t>> progress;

        void onBodyStarted(size_t bodyIndex, size_t rowCount) override {
            startedBodies.emplace_back(bodyIndex, rowCount);
        }

        void onBodyProgress(size_t bodyIndex, size_t completedRowCount, size_t totalRowCount) override {
            progress.emplace_back(bodyIndex, completedRowCount, totalRowCount);
        }
    };

    std::string ReadAll(const std::filesystem::path& path) {
        std::ifstream file(path, std::ios::binary);
        return {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
    }

    std::string NativeNewlines(std::string text) {
#ifdef _WIN32
        for (size_t position = 0; (position = text.find('\n', position)) != std::string::npos; position += 2) {
            text.replace(position, 1, "\r\n");
        }
#endif
        return text;
    }
}

TEST_CASE("Result export is silent and observer callbacks preserve CSV output", "[result][interface]") {
    const auto outputDirectory = std::filesystem::temp_directory_path() / "prologue-result-saver-test";
    std::filesystem::remove_all(outputDirectory);
    std::filesystem::create_directories(outputDirectory);
    const std::string prefix = outputDirectory.string() + "/";

    Environment environment;
    environment.launchSite.latitude = 33.0;
    environment.launchSite.longitude = 129.5;
    environment.launchSite.coordinateZone = 1;

    SimulationResult result;
    result.bodyResults.resize(1);
    SimulationStep step;
    step.gen_timeFromLaunch = 1.25;
    step.gen_elapsedTime = 0.5;
    step.rocket_mass = 1.0;
    result.bodyResults[0].steps.push_back(step);

    std::ostringstream consoleOutput;
    {
        ConsoleCapture capture(consoleOutput);
        ResultSaver::SaveDetail(prefix, result, environment, 3);
    }

    REQUIRE(consoleOutput.str().empty());
    const std::string expectedHeader = NativeNewlines(
        "time_from_launch[s],elapsed_time[s],launch_clear?,combusting?,para_opened?,air_density[kg/m3],"
        "gravity[m/s2],pressure[Pa],temperature[C],wind_x[m/s],wind_y[m/s],wind_z[m/s],mass[kg],"
        "Cg_from_nose[m],inertia moment pitch & yaw[kg*m2],inertia moment roll[kg*m2],attack angle[rad],"
        "altitude[m],velocity[m/s],airspeed[m/s],accel[m/s2],longitudinal accel[m/s2],normal_force[N],"
        "Cnp,Cny,Cmqp,Cmqy,Cp_from_nose[m],Cd,Cna,latitude,longitude,downrange[m],Fst[%],dynamic_pressure[Pa],\n");
    const std::string expectedRow = NativeNewlines(
        "1.250,0.500,0,0,\"\",0.000,0.000,0.000,0.000,0.000,0.000,0.000,1.000,0.000,0.000,0.000,"
        "0.000,0.000,0.000,0.000,0.000,0.000,0.000,0.000,0.000,0.000,0.000,0.000,0.000,0.000,33.000,129.500,"
        "0.000,0.000,0.000,\n");
    const auto detailContents = ReadAll(outputDirectory / "detail_body1.csv");
    REQUIRE(detailContents == expectedHeader + expectedRow);

    RecordingObserver observer;
    ResultSaver::SaveDetail(prefix, result, environment, 3, &observer);
    REQUIRE((observer.startedBodies == std::vector<std::pair<size_t, size_t>>{{0, 1}}));
    REQUIRE((observer.progress == std::vector<std::tuple<size_t, size_t, size_t>>{{0, 1, 1}}));
    REQUIRE(ReadAll(outputDirectory / "detail_body1.csv") == expectedHeader + expectedRow);

    REQUIRE_THROWS_WITH(ResultSaver::SaveDetail(prefix + "missing/", result, environment, 3),
                        Catch::Contains("Failed to open result CSV"));

    const auto asciiDirectory = outputDirectory / "ascii";
    const auto japaneseDirectory = outputDirectory / std::filesystem::path(std::u8string(u8"日本語 結果"));
    std::filesystem::create_directories(asciiDirectory);
    std::filesystem::create_directories(japaneseDirectory);

    ResultSaver::SaveDetail(asciiDirectory, result, environment, 3);
    ResultSaver::SaveDetail(japaneseDirectory, result, environment, 3);
    REQUIRE(ReadAll(asciiDirectory / "summary.csv") == ReadAll(japaneseDirectory / "summary.csv"));
    REQUIRE(ReadAll(asciiDirectory / "detail_body1.csv") == ReadAll(japaneseDirectory / "detail_body1.csv"));

    std::vector<SimulationResult> scatterResults(3);
    scatterResults[0].windSpeed = 1.0;
    scatterResults[0].windDirection.degrees = 0.0;
    scatterResults[0].bodyFinalPositions.emplace_back(0.0, 0.0, 0.0);
    scatterResults[1].windSpeed = 1.0;
    scatterResults[1].windDirection.degrees = 90.0;
    scatterResults[1].bodyFinalPositions.emplace_back(100.0, 0.0, 0.0);
    scatterResults[2].windSpeed = 1.0;
    scatterResults[2].windDirection.degrees = 180.0;
    scatterResults[2].bodyFinalPositions.emplace_back(0.0, 100.0, 0.0);
    ResultSaver::SaveScatter(asciiDirectory, scatterResults, environment, 3);
    ResultSaver::SaveScatter(japaneseDirectory, scatterResults, environment, 3);
    REQUIRE(ReadAll(asciiDirectory / "summary.csv") == ReadAll(japaneseDirectory / "summary.csv"));
    REQUIRE(std::filesystem::exists(asciiDirectory / "scatter_body1.kml"));
    REQUIRE(std::filesystem::exists(japaneseDirectory / "scatter_body1.kml"));
    REQUIRE(ReadAll(asciiDirectory / "scatter_body1.kml") == ReadAll(japaneseDirectory / "scatter_body1.kml"));

    std::filesystem::remove_all(outputDirectory);
}
