// ------------------------------------------------
// シミュレーションの型付き入力
// ------------------------------------------------

#pragma once

#include "core/SimulationSettings.hpp"
#include "dynamics/WindProfile.hpp"
#include "env/Environment.hpp"
#include "rocket/RocketSpec.hpp"

enum class TrajectoryMode : int { Trajectory = 1, Parachute };

enum class DetachType : int { BurningFinished = 1, Time, SyncPara, DoNotDeatch };

struct SimulationRunSettings {
    TrajectoryMode trajectoryMode = TrajectoryMode::Trajectory;
    DetachType detachType = DetachType::DoNotDeatch;
    double detachTime = 0.0;
    double windSpeed = 0.0;
    WindDirection windDirection;
    double magneticDeclination = 0.0;
};

struct SimulationInput {
    Environment environment;
    RocketSpecification rocket;
    SolverSettings solver;
    SimulationRunSettings run;
};

void ValidateSimulationInput(const SimulationInput& input);
