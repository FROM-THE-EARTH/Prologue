// ------------------------------------------------
// 3DプロットのためのIPlotter実装
// ------------------------------------------------

#pragma once

#include "PlotterBase.hpp"
#include "solver/Solver.hpp"

class Plotter3D : public PlotterBase<SimuResultSummary> {
private:
    const double m_windSpeed, m_windDirection;

public:
    explicit Plotter3D(std::string_view resultDirectory, size_t bodyCount, double windSpeed, double windDirection) :
        PlotterBase(resultDirectory, bodyCount, bodyCount), m_windSpeed(windSpeed), m_windDirection(windDirection) {}

    void saveResult(const SimuResultSummary& result) override;

protected:
    void initializePlot() const override;

    void plot() const override;
};
