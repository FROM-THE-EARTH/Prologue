// ------------------------------------------------
// 2DプロットのためのIPlotter実装
// ------------------------------------------------

#pragma once

#include "PlotterBase.hpp"
#include "env/Map.hpp"
#include "solver/Solver.hpp"

class Plotter2D : public PlotterBase<std::vector<SimuResultSummary>> {
private:
    struct PlotRange {
        double xMin, xMax;
        double yMin, yMax;
    };

    const MapData m_mapData;

    PlotRange m_range;

public:
    explicit Plotter2D(std::string_view resultDirectory, size_t bodyCount, const MapData& mapData);

    void saveResult(const std::vector<SimuResultSummary>& results) override;

    void savePlotAsPng();

protected:
    void initializePlot() const override;

    void plot() const override;

private:
    void calcPlotRange(const std::vector<SimuResultSummary>& results);
};
