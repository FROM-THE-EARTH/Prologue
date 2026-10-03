#define CATCH_CONFIG_RUNNER
#include "catch2/catch.hpp"

#include <memory>
#include <stdexcept>

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QFile>
#include <QImage>
#include <QLineEdit>
#include <QTemporaryDir>
#include <QXmlStreamReader>

#include <jkqtplotter/graphs/jkqtpscatter.h>
#include <jkqtplotter/jkqtplotter.h>

#include "gui/RunResults.hpp"
#include "gui/plot/PlotPanel.hpp"
#include "gui/plot/ResultSeries.hpp"

namespace {
    template<class Widget>
    Widget* Control(gui::PlotPanel& panel, const char* name) {
        auto* control = panel.findChild<Widget*>(QString::fromLatin1(name));
        REQUIRE(control != nullptr);
        return control;
    }

    template<class Quantity>
    void Select(QComboBox* control, Quantity quantity) {
        const int index = control->findData(static_cast<int>(quantity));
        REQUIRE(index >= 0);
        control->setCurrentIndex(index);
    }

    JKQTPXYLineGraph* Curve(JKQTPlotter* plotter, size_t index) {
        auto* graph = dynamic_cast<JKQTPXYLineGraph*>(plotter->getPlotter()->getGraph(index));
        REQUIRE(graph != nullptr);
        return graph;
    }

    std::shared_ptr<gui::RunResults> DetailResults() {
        auto results = std::make_shared<gui::RunResults>();
        results->mode = SimulationMode::Detail;
        SimulationResult simulation;
        for (size_t body = 0; body < 2; ++body) {
            BodySimulationResult history;
            for (size_t row = 0; row < 3; ++row) {
                SimulationStep step;
                step.gen_timeFromLaunch = static_cast<double>(row);
                step.rocket_mass = 2.0;
                step.rocket_pos.z = row == 1 ? 5.0 : static_cast<double>(row);
                // This path turns back in x. Sorting would connect the wrong samples.
                constexpr double downrange[] = {3.0, 1.0, 2.0};
                step.downrange = downrange[row] * static_cast<double>(body + 1);
                step.dynamicPressure = 100.0 + 10.0 * static_cast<double>(row);
                step.Fst = 10.0 * static_cast<double>(body + 1) + static_cast<double>(row);
                history.steps.push_back(step);
            }
            simulation.bodyResults.push_back(history);
        }
        results->simulations.push_back(simulation);
        return results;
    }

    std::shared_ptr<gui::RunResults> ScatterResults() {
        auto results = std::make_shared<gui::RunResults>();
        results->mode = SimulationMode::Scatter;
        for (const double speed : {2.0, 4.0}) {
            for (const double direction : {90.0, 0.0}) {
                SimulationResult simulation;
                simulation.windSpeed = speed;
                simulation.windDirection.degrees = direction;
                simulation.maxAltitude = speed * 100.0 + direction;
                simulation.bodyFinalPositions = {{speed, direction, -0.1},
                                                 {speed + 10.0, direction + 20.0, -0.2}};
                results->simulations.push_back(simulation);
            }
        }
        return results;
    }

    QByteArray ReadFile(const QString& path) {
        QFile file(path);
        REQUIRE(file.open(QIODevice::ReadOnly));
        const auto bytes = file.readAll();
        REQUIRE(file.error() == QFileDevice::NoError);
        return bytes;
    }
}

TEST_CASE("Detail plots retain selected axes, body and flight sample order", "[gui][plot]") {
    gui::PlotPanel panel;
    panel.setResults(DetailResults());
    auto* plotter = Control<JKQTPlotter>(panel, "resultPlot");
    auto* x = Control<QComboBox>(panel, "plotXQuantity");
    auto* y = Control<QComboBox>(panel, "plotYQuantity");
    auto* body = Control<QComboBox>(panel, "plotBody");
    REQUIRE(x->count() == static_cast<int>(gui::plot::DetailQuantities().size()));
    REQUIRE(plotter->getGraphCount() == 2);

    Select(x, gui::plot::DetailQuantity::Downrange);
    Select(y, gui::plot::DetailQuantity::StaticMargin);
    Select(body, 1);
    REQUIRE(plotter->getGraphCount() == 1);
    auto* curve = Curve(plotter, 0);
    auto* data = plotter->getDatastore();
    REQUIRE(curve->getDataSortOrder() == JKQTPXYGraph::Unsorted);
    REQUIRE(curve->getDrawLine());
    REQUIRE(data->getRows(curve->getXColumn()) == 3);
    REQUIRE(data->get(curve->getXColumn(), 0) == 6.0);
    REQUIRE(data->get(curve->getXColumn(), 1) == 2.0);
    REQUIRE(data->get(curve->getXColumn(), 2) == 4.0);
    REQUIRE(data->get(curve->getYColumn(), 0) == 20.0);
    REQUIRE(data->get(curve->getYColumn(), 2) == 22.0);
    REQUIRE(Control<QLineEdit>(panel, "plotYLabel")->text() == "Static margin Fst [%]");

    Select(y, gui::plot::DetailQuantity::DynamicPressure);
    curve = Curve(plotter, 0);
    REQUIRE(data->get(curve->getYColumn(), 1) == 110.0);
    REQUIRE(Control<QLineEdit>(panel, "plotYLabel")->text() == "Dynamic pressure [Pa]");
    Control<QCheckBox>(panel, "plotConnectPoints")->setChecked(false);
    REQUIRE_FALSE(Curve(plotter, 0)->getDrawLine());
}

TEST_CASE("Scatter plots use summary values and selected final bodies without histories", "[gui][plot]") {
    gui::PlotPanel panel;
    panel.setResults(ScatterResults());
    auto* plotter = Control<JKQTPlotter>(panel, "resultPlot");
    auto* x = Control<QComboBox>(panel, "plotXQuantity");
    auto* y = Control<QComboBox>(panel, "plotYQuantity");
    auto* body = Control<QComboBox>(panel, "plotBody");
    REQUIRE(x->count() == static_cast<int>(gui::plot::SummaryQuantities().size()));
    REQUIRE_FALSE(body->isEnabled());
    REQUIRE(plotter->getGraphCount() == 2);
    Select(x, gui::plot::SummaryQuantity::WindDirection);
    auto* curve = Curve(plotter, 0);
    auto* data = plotter->getDatastore();
    REQUIRE_FALSE(curve->getDrawLine());
    REQUIRE(data->getRows(curve->getXColumn()) == 2);
    REQUIRE(data->get(curve->getXColumn(), 0) == 90.0);
    REQUIRE(data->get(curve->getXColumn(), 1) == 0.0);
    REQUIRE(data->get(curve->getYColumn(), 0) == 290.0);
    REQUIRE(data->get(curve->getYColumn(), 1) == 200.0);

    Select(x, gui::plot::SummaryQuantity::FinalEast);
    Select(y, gui::plot::SummaryQuantity::FinalNorth);
    REQUIRE(body->isEnabled());
    REQUIRE(plotter->getGraphCount() == 4);
    Select(body, 1);
    REQUIRE(plotter->getGraphCount() == 2);
    curve = Curve(plotter, 0);
    REQUIRE(data->get(curve->getXColumn(), 0) == 12.0);
    REQUIRE(data->get(curve->getYColumn(), 0) == 110.0);
    REQUIRE(data->get(curve->getYColumn(), 1) == 20.0);
}

TEST_CASE("Report exports create PNG, SVG and PDF and restore the onscreen plot", "[gui][plot][export]") {
    QTemporaryDir directory;
    REQUIRE(directory.isValid());
    gui::PlotPanel panel;
    REQUIRE_THROWS_AS(panel.exportFigure(directory.filePath("empty.png")), std::invalid_argument);
    panel.resize(1100, 700);
    panel.show();
    QApplication::processEvents();
    panel.setResults(DetailResults());
    Select(Control<QComboBox>(panel, "plotYQuantity"), gui::plot::DetailQuantity::StaticMargin);
    auto* title = Control<QLineEdit>(panel, "plotTitle");
    title->setText("Report figure {test}_50% & $5 \\ demo");
    title->editingFinished();

    auto* plotter = Control<JKQTPlotter>(panel, "resultPlot");
    auto* backend = plotter->getPlotter();
    const QSize screenSize(backend->getWidth(), backend->getHeight());
    const auto screenStyle = backend->getCurrentPlotterStyle();
    const double xLabelSize = backend->getXAxis()->getLabelFontSize();
    const QString screenTitle = backend->getPlotLabel();
    const QString yLabel = backend->getYAxis()->getAxisLabel();
    for (const auto* extension : {"png", "svg", "pdf"}) {
        panel.exportFigure(directory.filePath(QString("figure.") + extension));
        REQUIRE(QSize(backend->getWidth(), backend->getHeight()) == screenSize);
        REQUIRE(backend->getCurrentPlotterStyle().plotLabelFontSize == screenStyle.plotLabelFontSize);
        REQUIRE(backend->getXAxis()->getLabelFontSize() == xLabelSize);
        REQUIRE(backend->getPlotLabel() == screenTitle);
        REQUIRE(backend->getYAxis()->getAxisLabel() == yLabel);
    }

    const QImage png(directory.filePath("figure.png"));
    REQUIRE_FALSE(png.isNull());
    REQUIRE(png.size() == QSize(2400, 1600));
    REQUIRE(png.pixelColor(0, 0).alpha() == 255);
    REQUIRE(png.pixelColor(png.width() - 1, png.height() - 1).alpha() == 255);
    REQUIRE(ReadFile(directory.filePath("figure.png")).size() > 10000);

    QXmlStreamReader svg(ReadFile(directory.filePath("figure.svg")));
    bool hasRoot = false, hasVectorShape = false;
    QString svgText;
    while (!svg.atEnd()) {
        svg.readNext();
        if (svg.isCharacters()) svgText += svg.text();
        if (!svg.isStartElement()) continue;
        hasRoot = hasRoot || svg.name() == u"svg";
        hasVectorShape = hasVectorShape || svg.name() == u"path" || svg.name() == u"polyline";
    }
    REQUIRE_FALSE(svg.hasError());
    REQUIRE(hasRoot);
    REQUIRE(hasVectorShape);
    REQUIRE(svgText.contains("Static margin Fst [%]"));
    REQUIRE(svgText.contains("Report figure {test"));
    REQUIRE(svgText.contains("_50% & $5 \\ demo"));
    const auto pdf = ReadFile(directory.filePath("figure.pdf"));
    REQUIRE(pdf.startsWith("%PDF-"));
    REQUIRE(pdf.size() > 1000);

    REQUIRE_THROWS_AS(panel.exportFigure(directory.filePath("figure.txt")), std::invalid_argument);
    REQUIRE_FALSE(QFile::exists(directory.filePath("figure.txt")));
    REQUIRE_THROWS_AS(panel.exportFigure(directory.filePath("missing/figure.png")), std::runtime_error);
    REQUIRE(QSize(backend->getWidth(), backend->getHeight()) == screenSize);
}

int main(int argc, char** argv) {
    if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM")) qputenv("QT_QPA_PLATFORM", "offscreen");
    QApplication application(argc, argv);
    return Catch::Session().run(argc, argv);
}
