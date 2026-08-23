// ------------------------------------------------
// シミュレータークラス
// ファイル読み取り、初期化を行い結果を元にSolverクラスで解析を実行する
// class SimulatorBaseは抽象クラスとして定義しているためそのままでは使えない
// Detail/Scatterモードに対して、SimulatorBaseクラスを継承したDetailSimulator/ScatterSimulatorを定義している
// 抽象クラスでは、virtual void func() { ... } とすることで継承したクラスでその内部実装を変更することできる
// virtual void func() = 0;とした関数は継承先で必ず実装しなければならない
// ------------------------------------------------

#pragma once

#include <string>

#include "config/ApplicationSettings.hpp"
#include "core/SimulationInput.hpp"
#include "env/Map.hpp"

enum class SimulationMode : int { Scatter = 1, Detail };

class SimulatorBase {
protected:
    const std::string m_specName;
    const ApplicationSettings m_applicationSettings;
    const SimulationMode m_simulationMode;
    SimulationInput m_input;
    const MapData m_mapData;
    std::string m_outputDirName;

public:
    explicit SimulatorBase(std::string specificationName,
                           SimulationInput input,
                           SimulationMode simulationMode,
                           ApplicationSettings applicationSettings);

    // 抽象クラスのデストラクタはvirtualで定義し直さなければいけない
    virtual ~SimulatorBase() {}

    // シミュレーション実行
    bool run(bool output);

    std::string getOutputDirectory() const {
        return m_outputDirName;
    }

protected:
    virtual bool simulate() = 0;

    virtual void saveResult() = 0;

private:
    void createResultDirectory();

    std::string getOutputDirectoryName() const;

    MapData getMapData() const;

    SolverSettings getSolverSettings() const;
};
