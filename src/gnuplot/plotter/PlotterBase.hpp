// ------------------------------------------------
// IPlotterを継承し、結果の型を指定するtemplate class
// ------------------------------------------------

#pragma once

#include "IPlotter.hpp"

template <class T>
class PlotterBase : public IPlotter {
public:
    using IPlotter::IPlotter;

    virtual ~PlotterBase() = default;

    virtual void saveResult(const T& results) = 0;
};
