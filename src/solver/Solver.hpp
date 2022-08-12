// ------------------------------------------------
// 解析用クラス
// ------------------------------------------------

#pragma once

#include "dynamics/WindModel.hpp"
#include "env/Environment.hpp"
#include "env/Map.hpp"
#include "result/SimuResult.hpp"
#include "rocket/Rocket.hpp"
#include "rocket/RocketSpec.hpp"

enum class TrajectoryMode : int { Trajectory = 1, Parachute };

enum class RocketType : int { Single = 1, Multi };

enum class DetachType : int { BurningFinished = 1, Time, SyncPara, DoNotDeatch };

class Solver {
    // Setting
    const double m_dt;
    const RocketSpecification m_rocketSpec;
    const Environment m_environment;
    const MapData m_mapData;
    const RocketType m_rocketType;
    const TrajectoryMode m_trajectoryMode;
    const DetachType m_detachType;
    const double m_detachTime;

    // Simulation
    Rocket m_rocket;
    Body m_bodyDelta;
    std::unique_ptr<WindModel> m_windModel;
    size_t m_currentBodyIndex = 0;  // Index of the body being solved
    size_t m_detachCount      = 0;
    size_t m_steps            = 0;

    // Result
    std::shared_ptr<SimuResultLogger> m_resultLogger = nullptr;

public:
    explicit Solver(MapData mapData,
                    RocketType rocketType,
                    TrajectoryMode mode,
                    DetachType detachType,
                    double detachTime,
                    const Environment& env,
                    const RocketSpecification& spec);

    std::shared_ptr<SimuResultLogger> solve(double windSpeed, double windDirection);

private:
    void initializeRocket();

    void update();

    void updateParachute();

    bool updateDetachment();

    void updateAerodynamicParameters();

    void updateRocketProperties();

    void updateExternalForce();

    void updateRocketDelta();

    void applyDelta();

    void organizeResult();

    // Prepare the next rocket (multi rocket)
    void nextRocket();
};
