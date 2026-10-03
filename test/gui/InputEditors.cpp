#define CATCH_CONFIG_RUNNER
#include "catch2/catch.hpp"

#include <cmath>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include <QApplication>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QTemporaryDir>

#include "io/ProjectJsonSerializer.hpp"
#include "gui/input/NumericInput.hpp"
#include "gui/input/ProjectEditor.hpp"
#include "gui/input/SettingsEditor.hpp"

namespace {
    bool SameOptional(const std::optional<double>& a, const std::optional<double>& b) {
        return a.has_value() == b.has_value() && (!a || *a == *b);
    }

    bool SameBody(const project::Body& a, const project::Body& b) {
        if (a.length != b.length || a.diameter != b.diameter || a.massInitial != b.massInitial
            || a.massFinal != b.massFinal || a.centerOfGravityInitial != b.centerOfGravityInitial
            || a.centerOfGravityFinal != b.centerOfGravityFinal
            || a.pitchYawMomentOfInertiaInitial != b.pitchYawMomentOfInertiaInitial
            || a.pitchYawMomentOfInertiaFinal != b.pitchYawMomentOfInertiaFinal
            || a.pitchDampingMomentCoefficient != b.pitchDampingMomentCoefficient
            || a.engine.thrustFile != b.engine.thrustFile
            || a.engine.thrustMeasuredPressure != b.engine.thrustMeasuredPressure
            || a.engine.nozzleDiameter != b.engine.nozzleDiameter
            || a.aerodynamics.coefficientFile != b.aerodynamics.coefficientFile
            || a.aerodynamics.centerOfPressure != b.aerodynamics.centerOfPressure
            || a.aerodynamics.centerOfPressureAlpha != b.aerodynamics.centerOfPressureAlpha
            || a.aerodynamics.dragCoefficientInitial != b.aerodynamics.dragCoefficientInitial
            || a.aerodynamics.dragCoefficientFinal != b.aerodynamics.dragCoefficientFinal
            || a.aerodynamics.dragCoefficientAlphaSquared != b.aerodynamics.dragCoefficientAlphaSquared
            || a.aerodynamics.normalForceCoefficient != b.aerodynamics.normalForceCoefficient
            || a.parachutes.size() != b.parachutes.size() || a.transitions.size() != b.transitions.size()) {
            return false;
        }
        for (size_t i = 0; i < a.parachutes.size(); ++i) {
            const auto& x = a.parachutes[i];
            const auto& y = b.parachutes[i];
            if (!SameOptional(x.openingTimeFromLaunch, y.openingTimeFromLaunch)
                || !SameOptional(x.openingTimeFromPeak, y.openingTimeFromPeak)
                || !SameOptional(x.openingHeight, y.openingHeight) || x.CdS != y.CdS) {
                return false;
            }
        }
        for (size_t i = 0; i < a.transitions.size(); ++i) {
            if (a.transitions[i].time != b.transitions[i].time
                || a.transitions[i].mass != b.transitions[i].mass
                || a.transitions[i].Cd != b.transitions[i].Cd) {
                return false;
            }
        }
        return true;
    }

    bool SameDocument(const project::Document& a, const project::Document& b) {
        if (a.formatVersion != b.formatVersion || a.environment.place != b.environment.place
            || a.environment.latitude != b.environment.latitude
            || a.environment.longitude != b.environment.longitude
            || a.environment.coordinateZone != b.environment.coordinateZone
            || a.environment.magneticDeclination != b.environment.magneticDeclination
            || a.environment.railLength != b.environment.railLength
            || a.environment.railAzimuth != b.environment.railAzimuth
            || a.environment.railElevation != b.environment.railElevation
            || a.bodies.size() != b.bodies.size() || a.separations.size() != b.separations.size()) {
            return false;
        }
        for (size_t i = 0; i < a.bodies.size(); ++i) {
            if (!SameBody(a.bodies[i], b.bodies[i])) return false;
        }
        for (size_t i = 0; i < a.separations.size(); ++i) {
            if (a.separations[i].sourceBodyIndex != b.separations[i].sourceBodyIndex
                || a.separations[i].productBodyIndices != b.separations[i].productBodyIndices) {
                return false;
            }
        }
        return true;
    }

    project::Body MakeBody(size_t index) {
        const double k = static_cast<double>(index + 1);
        project::Body body;
        body.length = 2.01 * k;
        body.diameter = 0.091 * k;
        body.massInitial = 8.243 * k;
        body.massFinal = 7.867 * k;
        body.centerOfGravityInitial = 1.074 * k;
        body.centerOfGravityFinal = 1.051 * k;
        body.pitchYawMomentOfInertiaInitial = 1.971 * k;
        body.pitchYawMomentOfInertiaFinal = 1.863 * k;
        body.pitchDampingMomentCoefficient = -3.125 * k;
        body.engine.thrustFile = std::filesystem::path("../thrust/engine " + std::to_string(index) + ".txt");
        body.engine.thrustMeasuredPressure = 100123.45678901235 * k;
        body.engine.nozzleDiameter = 0.012345678901234567 * k;
        body.aerodynamics.coefficientFile = std::filesystem::path("../aero/aero.csv");
        body.aerodynamics.centerOfPressure = 1.3812345678901234 * k;
        body.aerodynamics.centerOfPressureAlpha = 0.125 * k;
        body.aerodynamics.dragCoefficientInitial = 0.4567890123456789 * k;
        body.aerodynamics.dragCoefficientFinal = 0.12345678901234568 * k;
        body.aerodynamics.dragCoefficientAlphaSquared = 0.0009876543210123456 * k;
        body.aerodynamics.normalForceCoefficient = 11.234567890123456 * k;
        body.parachutes.push_back({.openingTimeFromLaunch = 10.123456789012345 * k,
                                   .openingTimeFromPeak = std::nullopt,
                                   .openingHeight = std::nullopt,
                                   .CdS = 5.987654321012345 * k});
        body.parachutes.push_back({.openingTimeFromLaunch = std::nullopt,
                                   .openingTimeFromPeak = 1.2345678901234567 * k,
                                   .openingHeight = 123.45678901234567 * k,
                                   .CdS = 2.3456789012345678 * k});
        body.transitions.push_back({.time = 5.123456789012345 * k,
                                    .mass = -0.23456789012345678 * k,
                                    .Cd = -0.012345678901234567 * k});
        body.transitions.push_back({.time = 10.987654321098765 * k,
                                    .mass = -0.34567890123456789 * k,
                                    .Cd = 0.0012345678901234567 * k});
        return body;
    }

    project::Document MakeDocument(size_t bodyCount) {
        project::Document document;
        document.formatVersion = project::CurrentFormatVersion;
        document.environment = {
            .place = "Noshirō test site",
            .latitude = 40.24286512345678,
            .longitude = 140.01045098765432,
            .coordinateZone = 10,
            .magneticDeclination = 8.941234567890123,
            .railLength = 5.012345678901234,
            .railAzimuth = -60.12345678901234,
            .railElevation = 70.98765432109876,
        };
        for (size_t i = 0; i < bodyCount; ++i) document.bodies.push_back(MakeBody(i));
        if (bodyCount == 3) document.separations.push_back({0, {1, 2}});
        return document;
    }

    bool SameSettings(const ApplicationSettings& a, const ApplicationSettings& b) {
        if (a.execution.multiThread != b.execution.multiThread
            || a.execution.threadCount != b.execution.threadCount
            || a.solver.timeStep != b.solver.timeStep
            || a.scatter.windSpeedMin != b.scatter.windSpeedMin
            || a.scatter.windSpeedMax != b.scatter.windSpeedMax
            || a.scatter.windDirectionInterval != b.scatter.windDirectionInterval
            || a.result.precision != b.result.precision
            || a.solver.resultStepSaveInterval != b.solver.resultStepSaveInterval
            || a.solver.wind.type != b.solver.wind.type
            || a.solver.wind.powerConstant != b.solver.wind.powerConstant
            || a.solver.wind.powerLowBaseAltitude != b.solver.wind.powerLowBaseAltitude
            || a.solver.atmosphere.basePressure != b.solver.atmosphere.basePressure
            || a.solver.atmosphere.baseTemperature != b.solver.atmosphere.baseTemperature
            || a.measuredWindFilename != b.measuredWindFilename
            || a.solver.wind.measuredProfile.has_value() != b.solver.wind.measuredProfile.has_value()) {
            return false;
        }
        if (a.solver.wind.measuredProfile) {
            const auto before = a.solver.wind.measuredProfile->windAt(75.0, 3.0);
            const auto after = b.solver.wind.measuredProfile->windAt(75.0, 3.0);
            if (!(before == after)) return false;
        }
        return true;
    }

    std::string Utf8Path(const std::filesystem::path& path) {
        const auto utf8 = path.u8string();
        return {reinterpret_cast<const char*>(utf8.data()), utf8.size()};
    }
}

TEST_CASE("Project and settings editors preserve full precision fields", "[gui][input]") {

    ProjectEditor projectEditor;
    const auto projectPath = std::filesystem::path("projects/demo/project.json");
    for (const size_t bodyCount : {size_t{1}, size_t{3}}) {
        const auto source = MakeDocument(bodyCount);
        projectEditor.setDocument(source, projectPath);
        REQUIRE(SameDocument(source, projectEditor.document()));
    }
    auto emptyDocument = MakeDocument(0);
    projectEditor.setDocument(emptyDocument, projectPath);
    REQUIRE(SameDocument(emptyDocument, projectEditor.document()));

    projectEditor.setDocument(MakeDocument(1), projectPath);
    auto* latitude = dynamic_cast<NumericInput*>(projectEditor.findChild<QLineEdit*>("latitude"));
    REQUIRE(latitude != nullptr);
    latitude->setText(QStringLiteral("4.024286512345678e+1"));
    const auto edited = projectEditor.document();
    REQUIRE(edited.environment.latitude == 40.24286512345678);

    auto parachuteDocument = MakeDocument(1);
    parachuteDocument.bodies[0].parachutes[0].openingTimeFromPeak.reset();
    parachuteDocument.bodies[0].parachutes[0].openingHeight.reset();
    projectEditor.setDocument(parachuteDocument, projectPath);
    auto* parachutes = projectEditor.findChild<QTableWidget*>("body0_parachutes");
    REQUIRE(parachutes != nullptr);
    auto* openAtPeak = dynamic_cast<NumericInput*>(parachutes->cellWidget(0, 1));
    auto* openAtHeight = dynamic_cast<NumericInput*>(parachutes->cellWidget(0, 2));
    REQUIRE(openAtPeak != nullptr);
    REQUIRE(openAtHeight != nullptr);
    REQUIRE(openAtPeak->text().isEmpty());
    REQUIRE(openAtHeight->text().isEmpty());
    openAtPeak->setText(QStringLiteral("3.141592653589793"));
    openAtHeight->setText(QStringLiteral("1234.5678901234567"));

    auto* addParachute = projectEditor.findChild<QPushButton*>("body0_addParachuteButton");
    REQUIRE(addParachute != nullptr);
    addParachute->click();
    const int addedRow = parachutes->rowCount() - 1;
    auto* addedOpenAtLaunch = dynamic_cast<NumericInput*>(parachutes->cellWidget(addedRow, 0));
    REQUIRE(addedOpenAtLaunch != nullptr);
    addedOpenAtLaunch->setText(QStringLiteral("0.012345678901234567"));
    auto* addedOpenAtPeak = dynamic_cast<NumericInput*>(parachutes->cellWidget(addedRow, 1));
    REQUIRE(addedOpenAtPeak != nullptr);
    addedOpenAtPeak->setText(QStringLiteral("6.789012345678901"));
    auto* addedOpenAtHeight = dynamic_cast<NumericInput*>(parachutes->cellWidget(addedRow, 2));
    REQUIRE(addedOpenAtHeight != nullptr);
    addedOpenAtHeight->setText(QStringLiteral("9876.543210987654"));

    const auto parachuteEdited = projectEditor.document();
    REQUIRE(parachuteEdited.bodies[0].parachutes[0].openingTimeFromPeak == 3.141592653589793);
    REQUIRE(parachuteEdited.bodies[0].parachutes[0].openingHeight == 1234.5678901234567);
    REQUIRE(parachuteEdited.bodies[0].parachutes.back().openingTimeFromLaunch == 0.012345678901234567);
    REQUIRE(parachuteEdited.bodies[0].parachutes.back().openingTimeFromPeak == 6.789012345678901);
    REQUIRE(parachuteEdited.bodies[0].parachutes.back().openingHeight == 9876.543210987654);
    QTemporaryDir temporaryDirectory;
    REQUIRE(temporaryDirectory.isValid());
    const auto serializedFile = std::filesystem::path(temporaryDirectory.filePath("project.json").toStdU16String());
    ProjectJsonSerializer::Save(serializedFile, parachuteEdited);
    const auto reloaded = ProjectJsonSerializer::Load(serializedFile);
    REQUIRE(SameDocument(parachuteEdited, reloaded));

    NumericInput precisionInput;
    precisionInput.setValue(1.2345678901234567e-100);
    REQUIRE(precisionInput.value(QStringLiteral("test number")) == 1.2345678901234567e-100);
    precisionInput.setText(QStringLiteral("invalid"));
    bool rejectedInvalid = false;
    try {
        static_cast<void>(precisionInput.value(QStringLiteral("test number")));
    } catch (const std::invalid_argument&) {
        rejectedInvalid = true;
    }
    REQUIRE(rejectedInvalid);

    ApplicationSettings settings;
    settings.execution = {.multiThread = true, .threadCount = 13};
    settings.solver.timeStep = 0.00012345678901234568;
    settings.scatter = {.windSpeedMin = 1.25, .windSpeedMax = 19.75, .windDirectionInterval = 22.5};
    settings.result.precision = 17;
    settings.solver.resultStepSaveInterval = 37;
    settings.solver.wind.type = WindModelType::Real;
    settings.solver.wind.powerConstant = 6.125;
    settings.solver.wind.powerLowBaseAltitude = 2.75;
    settings.solver.wind.measuredProfile = WindProfile({{100.0, 3.25, 87.5}, {200.0, 4.75, 123.5}},
                                                        DirectionReference::MagneticNorth);
    settings.measuredWindFilename = "data/wind " + std::string("測定.csv");
    settings.solver.atmosphere.basePressure = 100123.45678901235;
    settings.solver.atmosphere.baseTemperature = -2.125;

    SettingsEditor settingsEditor;
    settingsEditor.setSettings(settings, std::filesystem::path("projects/demo/prologue.settings.json"));
    const auto roundTripSettings = settingsEditor.settings();
    REQUIRE(SameSettings(settings, roundTripSettings));

    settings.measuredWindFilename = Utf8Path(
        std::filesystem::absolute(std::filesystem::temp_directory_path()
                                  / std::filesystem::path(std::u8string(u8"入力/wind/測定.csv"))));
    settingsEditor.setSettings(settings, std::filesystem::path("projects/demo/prologue.settings.json"));
    REQUIRE(SameSettings(settings, settingsEditor.settings()));
}

int main(int argc, char** argv) {
    QApplication application(argc, argv);
    return Catch::Session().run(argc, argv);
}
