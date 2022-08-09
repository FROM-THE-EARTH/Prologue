// ------------------------------------------------
// シミュレータークラス
// ファイル読み取り、初期化を行い結果を元にSolverクラスで解析を実行する
// class SimulatorBaseは抽象クラスとして定義しているためそのままでは使えない
// Detail/Scatterモードに対して、SimulatorBaseクラスを継承したDetailSimulator/ScatterSimulatorを定義している
// 抽象クラスでは、virtual void func() { ... } とすることで継承したクラスでその内部実装を変更することできる
// virtual void func() = 0;とした関数は継承先で必ず実装しなければならない
// ------------------------------------------------

#pragma once

#include <boost/property_tree/ptree.hpp>
#include <string>

#include "solver/Solver.hpp"

struct Environment;
struct MapData;
class RocketSpecification;

enum class SimulationMode : int { Scatter = 1, Detail };

class SimulatorBase {
public:
    // Settings from command line
    struct SimulationSetting {
        SimulationMode simulationMode;
        TrajectoryMode trajectoryMode;
        DetachType detachType;
        double detachTime    = 0.0;
        double windSpeed     = 0.0;
        double windDirection = 0.0;  // Direction is clockwise from the north
    };

protected:
    const std::string m_specName;
    const SimulationSetting m_setting;
    const RocketType m_rocketType;
    const RocketSpecification m_rocketSpec;
    const Environment m_environment;
    const MapData m_mapData;
    const std::string m_outputDirName;

public:
    explicit SimulatorBase(const std::string specName,
                           const boost::property_tree::ptree& specJson,
                           const SimulationSetting& setting);

    // 抽象クラスのデストラクタはvirtualで定義し直さなければいけない
    virtual ~SimulatorBase() {}

    // Gnuplotへ結果をプロット
    virtual void plotToGnuplot() = 0;

    // シミュレーション実行
    bool run(bool output);

protected:
    virtual bool simulate() = 0;

    virtual void saveResult() = 0;

private:
    void createResultDirectory();

    std::string getOutputDirectoryName();

    MapData getMapData();
};
