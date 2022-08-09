// ------------------------------------------------
// 2D/3Dプロットのための抽象クラス
// ------------------------------------------------

#pragma once

#include <string>

#include "gnuplot/GnuplotController.hpp"

#if PLATFORM_WINDOWS
#define GNUPLOT_TERMINAL "windows"
#define PAUSE_COMMAND "pause"
#elif PLATFORM_MACOS
#define GNUPLOT_TERMINAL "qt"
#define PAUSE_COMMAND "read -n1 -r -p \"Press any key to continue...\" key"
#else
#define GNUPLOT_TERMINAL "x11"
#define PAUSE_COMMAND "read -rsp $'Press any key to continue...\n'"
#endif

class IPlotter {
protected:
    const std::string m_resultDirectory;
    const size_t m_bodyCount;
    const size_t m_plotCount;

    GnuplotController m_gnuplot;

public:
    explicit IPlotter(std::string_view resultDirectory, size_t bodyCount, size_t plotCount);

    // 抽象クラスなのでデストラクタはvirtual指定
    virtual ~IPlotter() = default;

    void savePlot();

protected:
    virtual void plot() const = 0;

    virtual void initializePlot() const;

    void saveLaunchPoint() const;
};
