#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"

#include <filesystem>
#include <limits>
#include <locale>

#include "io/ApplicationSettingsReader.hpp"
#include "io/ApplicationSettingsWriter.hpp"

TEST_CASE("Saved application settings use the CLI format without losing precision", "[io][gui]") {
    auto settings = ApplicationSettingsReader::Read("data/wind_model/prologue.settings.json").value;
    settings.solver.timeStep = 0.0012345678901234567;
    settings.scatter.windSpeedMin = 1.2345678901234567;
    settings.solver.atmosphere.baseTemperature = 14.987654321098765;
    settings.measuredWindFilename = "wind\\\"sample.csv";
    const auto file = std::filesystem::temp_directory_path() / std::filesystem::path(u8"prologue-設定.json");
    ApplicationSettingsWriter::Write(file, settings);
    const auto restored = ApplicationSettingsReader::Read(file).value;
    std::filesystem::remove(file);

    REQUIRE(restored.solver.timeStep == settings.solver.timeStep);
    REQUIRE(restored.scatter.windSpeedMin == settings.scatter.windSpeedMin);
    REQUIRE(restored.solver.atmosphere.baseTemperature == settings.solver.atmosphere.baseTemperature);
    REQUIRE(restored.measuredWindFilename == settings.measuredWindFilename);
    REQUIRE(restored.execution.multiThread == settings.execution.multiThread);
    REQUIRE(restored.result.precision == settings.result.precision);
    REQUIRE(restored.solver.wind.type == settings.solver.wind.type);
}

TEST_CASE("Invalid settings serialization cannot truncate an existing file", "[io][gui]") {
    ApplicationSettings settings;
    settings.solver.timeStep = std::numeric_limits<double>::quiet_NaN();
    REQUIRE_THROWS_AS(ApplicationSettingsWriter::Serialize(settings), std::invalid_argument);
    settings.solver.timeStep = 0.001;
    settings.solver.wind.type = static_cast<WindModelType>(99);
    REQUIRE_THROWS_AS(ApplicationSettingsWriter::Serialize(settings), std::invalid_argument);
}
