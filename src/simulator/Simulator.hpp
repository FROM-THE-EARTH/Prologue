// ------------------------------------------------
// シミュレータークラス
// ファイル読み取り、初期化を行い結果を元にSolverクラスで解析を実行する
// class Simulatorは抽象クラスとして定義しているためそのままでは使えない
// Detail/Scatterモードに対して、Simulatorクラスを継承したDetailSimulator/ScatterSimulatorを定義している
// 抽象クラスでは、virtual void func() { ... } とすることで継承したクラスでその内部実装を変更することできる
// virtual void func() = 0;とした関数は継承先で必ず実装しなければならない
// ------------------------------------------------

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "env/Environment.hpp"
#include "env/Map.hpp"
#include "rocket/RocketSpec.hpp"
#include "solver/Solver.hpp"

enum class SimulationMode : int { Scatter = 1, Detail };

class Simulator {
protected:
    // settings
    const std::string m_jsonFile;
    const SimulationMode m_simulationMode;
    RocketType m_rocketType;
    TrajectoryMode m_trajectoryMode;
    DetachType m_detachType;
    double m_detachTime;

    // simulate
    const double m_dt;
    double m_windSpeed     = 0.0;
    double m_windDirection = 0.0;  // direction is clockwise from the north

    // from json
    Environment m_environment;
    std::unique_ptr<RocketSpecification> m_rocketSpec = nullptr;
    MapData m_mapData;

    // result
    std::string m_outputDirName;

public:
    // 抽象クラスのデストラクタはvirtualで定義し直さなければいけない
    virtual ~Simulator() {}

    // Gnuplotへ結果をプロット
    virtual void plotToGnuplot() = 0;

    // DetailSimulatorまたはScatterSimulatorの生成
    static std::unique_ptr<Simulator> New(double dt);

    // シミュレーション実行
    bool run(bool output);

protected:
    Simulator(const std::string& jsonFile, SimulationMode simulationMode, double dt) :
        m_jsonFile(jsonFile), m_simulationMode(simulationMode), m_dt(dt) {}

    virtual bool simulate() = 0;

    virtual void saveResult() = 0;

private:
    static std::string SetJSONFile();

    static SimulationMode SetSimulationMode();

    bool initialize();

    void setTrajectoryMode();

    void setWindCondition();

    void setDetachType();

    void setDetachTime();

    void createResultDirectory();
};
