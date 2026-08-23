// ------------------------------------------------
// 解析用クラス
// ------------------------------------------------

#pragma once

#include <memory>

#include "core/SimulationInput.hpp"
#include "core/SimulationObserver.hpp"
#include "dynamics/WindModel.hpp"
#include "env/Environment.hpp"
#include "rocket/Rocket.hpp"
#include "rocket/RocketSpec.hpp"

class Solver {
    // Setting
    const double m_dt;
    const size_t m_resultStepSaveInterval;
    const Environment m_environment;
    const SolverSettings m_solverSettings;
    const SimulationRunSettings m_runSettings;
    const bool m_isMultiple;
    RocketSpecification m_rocketSpec;

    // Simulation
    Rocket m_rocket;
    Body m_bodyDelta;
    std::unique_ptr<WindModel> m_windModel;
    size_t m_currentBodyIndex = 0;  // Index of the body being solved
    size_t m_detachCount      = 0;
    size_t m_steps            = 0;

    SimulationObserver& m_observer;

public:
    explicit Solver(const SimulationInput& input, SimulationObserver& observer);

    void solve();

private:
    void initializeRocket();

    void update();

    void updateParachute();

    bool updateDetachment();

    void updateAerodynamicParameters(const WindModel::AtmosphericConditions& air);

    void updateRocketProperties();

    void updateExternalForce(const WindModel::AtmosphericConditions& air);

    void updateRocketDelta();

    void applyDelta();

    void organizeResult(const WindModel::AtmosphericConditions& air);

    // Prepare the next rocket (multi rocket)
    void nextRocket();
};
