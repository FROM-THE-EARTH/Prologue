#include "gui/plot/PlotPanel.hpp"

#include <algorithm>
#include <cmath>
#include <map>
#include <stdexcept>
#include <vector>

#include <QCheckBox>
#include <QComboBox>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QImage>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSaveFile>
#include <QSignalBlocker>
#include <QTemporaryFile>
#include <QVBoxLayout>

#include <jkqtplotter/graphs/jkqtpscatter.h>
#include <jkqtplotter/jkqtplotter.h>

#include "geography/GeographicResult.hpp"
#include "gui/RunResults.hpp"
#include "gui/plot/ResultSeries.hpp"

namespace gui {
    namespace {
        QString Label(const char* label, const char* unit) {
            return QString::fromStdString(plot::AxisLabel(label, unit));
        }

        QString PlotText(const QString& text) {
            // JKQtPlotter treats labels as LaTeX. One literal text node preserves
            // spaces and user-entered punctuation without changing numeric ticks.
            // Its verb command ends at the first closing brace, so emit those
            // braces separately using the documented escape.
            QString result;
            const auto parts = text.split('}');
            for (qsizetype i = 0; i < parts.size(); ++i) {
                if (i > 0) result += "\\}";
                if (!parts[i].isEmpty()) result += "\\verb{" + parts[i] + "}";
            }
            return result;
        }

        class ReportSize {
            JKQTBasePlotter* m_plot;
            int m_width;
            int m_height;
            JKQTBasePlotterStyle m_style;
            JKQTPCoordinateAxisStyle m_xStyle;
            JKQTPCoordinateAxisStyle m_yStyle;
        public:
            explicit ReportSize(JKQTBasePlotter* plot) :
                m_plot(plot), m_width(plot->getWidth()), m_height(plot->getHeight()),
                m_style(plot->getCurrentPlotterStyle()),
                m_xStyle(plot->getXAxis()->getCurrentAxisStyle()),
                m_yStyle(plot->getYAxis()->getCurrentAxisStyle()) {
                auto reportStyle = m_style;
                // Keep labels legible when the figure is reduced to report width.
                reportStyle.defaultFontSize *= 3.0;
                reportStyle.plotLabelFontSize *= 3.0;
                reportStyle.keyStyle.fontSize *= 3.0;
                reportStyle.xAxisStyle = m_xStyle;
                reportStyle.yAxisStyle = m_yStyle;
                for (auto* axis : {&reportStyle.xAxisStyle, &reportStyle.yAxisStyle}) {
                    axis->labelFontSize *= 3.0;
                    axis->tickLabelFontSize *= 3.0;
                    axis->minorTickLabelFontSize *= 3.0;
                }
                m_plot->setCurrentPlotterStyle(reportStyle);
                m_plot->setWidgetSize(2400, 1600);
            }
            ~ReportSize() {
                m_plot->setCurrentPlotterStyle(m_style);
                m_plot->getXAxis()->setCurrentAxisStyle(m_xStyle);
                m_plot->getYAxis()->setCurrentAxisStyle(m_yStyle);
                m_plot->setWidgetSize(m_width, m_height);
            }
        };

        void AddCurve(JKQTPlotter* plotter, const std::vector<double>& x,
                      const std::vector<double>& y, const QString& title, bool connectPoints) {
            if (x.empty()) return;
            auto* data = plotter->getDatastore();
            auto* graph = new JKQTPXYLineGraph(plotter);
            graph->setXColumn(data->addCopiedColumn(x, title + " x"));
            graph->setYColumn(data->addCopiedColumn(y, title + " y"));
            graph->setTitle(PlotText(title));
            // Time histories may turn back in x (for example downrange against
            // altitude). Sorting by x would change the flight path in the plot.
            graph->setDataSortOrder(JKQTPXYGraph::Unsorted);
            graph->setDrawLine(connectPoints);
            graph->setSymbolType(connectPoints ? JKQTPNoSymbol : JKQTPCircle);
            graph->setSymbolSize(5.0);
            graph->setLineWidth(1.5);
            plotter->addGraph(graph);
        }

        // JKQtPlotter's native save functions return void. Render into a fresh
        // temporary file and then commit it to the selected destination so an
        // unsuccessful render cannot be mistaken for an old existing export.
        void SaveReport(JKQTPlotter* plotter, const QString& destination) {
            const QString extension = QFileInfo(destination).suffix().toLower();
            QTemporaryFile rendered(QFileInfo(destination).absolutePath()
                                    + "/.prologue-plot-XXXXXX." + extension);
            if (!rendered.open()) {
                throw std::runtime_error{"Cannot create an image in the selected folder."};
            }
            const QString renderPath = rendered.fileName();
            rendered.close();

            auto* backend = plotter->getPlotter();
            {
                // Resizing the backend normally asks the onscreen widget to
                // redraw, which restores its screen size. Block that notification
                // while the native exporter uses the report canvas.
                const QSignalBlocker blockRedraw(backend);
                ReportSize reportSize(backend);
                if (extension == "png") backend->saveAsPixelImage(renderPath, false, "PNG");
                else if (extension == "svg") backend->saveAsSVG(renderPath, false);
                else if (extension == "pdf") backend->saveAsPDF(renderPath, false);
                else throw std::invalid_argument{"Choose a PNG, SVG or PDF filename."};
            }
            plotter->redrawPlot();

            if (extension == "png") {
                // The pinned native PNG writer allocates an extra transparent
                // margin. The complete rendered report canvas is 2400 x 1600.
                QImage image(renderPath);
                if (image.width() < 2400 || image.height() < 1600
                    || !image.copy(0, 0, 2400, 1600).save(renderPath, "PNG")) {
                    throw std::runtime_error{"The report PNG could not be generated."};
                }
            }

            QFile source(renderPath);
            if (!source.open(QIODevice::ReadOnly) || source.size() == 0) {
                throw std::runtime_error{"The plot image could not be generated."};
            }
            QSaveFile output(destination);
            if (!output.open(QIODevice::WriteOnly)) {
                throw std::runtime_error{output.errorString().toStdString()};
            }
            const QByteArray contents = source.readAll();
            if (source.error() != QFileDevice::NoError) {
                throw std::runtime_error{source.errorString().toStdString()};
            }
            if (output.write(contents) != contents.size() || !output.commit()) {
                throw std::runtime_error{output.errorString().toStdString()};
            }
        }
    }

    PlotPanel::PlotPanel(QWidget* parent) : QWidget(parent) {
        auto* layout = new QVBoxLayout(this);
        auto* selectors = new QHBoxLayout;
        m_preset = new QComboBox(this);
        m_preset->setObjectName("plotPreset");
        m_xQuantity = new QComboBox(this);
        m_xQuantity->setObjectName("plotXQuantity");
        m_yQuantity = new QComboBox(this);
        m_yQuantity->setObjectName("plotYQuantity");
        for (auto* quantity : {m_xQuantity, m_yQuantity}) {
            quantity->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
            quantity->setMinimumContentsLength(18);
        }
        m_body = new QComboBox(this);
        m_body->setObjectName("plotBody");
        m_connectPoints = new QCheckBox("Connect points", this);
        m_connectPoints->setObjectName("plotConnectPoints");
        selectors->addWidget(new QLabel("Preset", this));
        selectors->addWidget(m_preset);
        selectors->addWidget(new QLabel("X", this));
        selectors->addWidget(m_xQuantity, 1);
        selectors->addWidget(new QLabel("Y", this));
        selectors->addWidget(m_yQuantity, 1);
        selectors->addWidget(new QLabel("Body", this));
        selectors->addWidget(m_body);
        selectors->addWidget(m_connectPoints);
        layout->addLayout(selectors);

        auto* labels = new QHBoxLayout;
        m_title = new QLineEdit(this);
        m_title->setObjectName("plotTitle");
        m_xLabel = new QLineEdit(this);
        m_xLabel->setObjectName("plotXLabel");
        m_yLabel = new QLineEdit(this);
        m_yLabel->setObjectName("plotYLabel");
        labels->addWidget(new QLabel("Title", this));
        labels->addWidget(m_title, 1);
        labels->addWidget(new QLabel("X label", this));
        labels->addWidget(m_xLabel, 1);
        labels->addWidget(new QLabel("Y label", this));
        labels->addWidget(m_yLabel, 1);
        layout->addLayout(labels);

        m_plot = new JKQTPlotter(this);
        m_plot->setObjectName("resultPlot");
        m_plot->setToolbarEnabled(true);
        m_plot->setToolbarAlwaysOn(true);
        m_plot->setMousePositionShown(true);
        m_plot->setMinimumHeight(300);
        m_plot->setGrid(true);
        m_plot->getPlotter()->setBackgroundColor(Qt::white);
        m_plot->getPlotter()->setExportBackgroundColor(Qt::white);
        m_plot->getPlotter()->setUseAntiAliasingForGraphs(true);
        layout->addWidget(m_plot, 1);

        auto* footer = new QHBoxLayout;
        m_status = new QLabel("Run a simulation to plot its results.", this);
        m_status->setWordWrap(true);
        footer->addWidget(m_status, 1);
        auto* fit = new QPushButton("Fit axes", this);
        fit->setObjectName("plotFitAxes");
        m_export = new QPushButton("Export PNG / SVG / PDF", this);
        m_export->setObjectName("plotExport");
        m_export->setEnabled(false);
        footer->addWidget(fit);
        footer->addWidget(m_export);
        layout->addLayout(footer);

        connect(m_xQuantity, &QComboBox::currentIndexChanged, this, [this] {
            updateDefaultLabels();
            rebuildPlot();
        });
        connect(m_yQuantity, &QComboBox::currentIndexChanged, this, [this] {
            updateDefaultLabels();
            rebuildPlot();
        });
        connect(m_body, &QComboBox::currentIndexChanged, this, [this] { rebuildPlot(); });
        connect(m_preset, &QComboBox::activated, this, [this] { applyPreset(); });
        connect(m_connectPoints, &QCheckBox::toggled, this, [this] { rebuildPlot(); });
        for (auto* label : {m_title, m_xLabel, m_yLabel}) {
            connect(label, &QLineEdit::editingFinished, this, [this] {
                m_plot->getPlotter()->setPlotLabel(PlotText(m_title->text()));
                m_plot->getXAxis()->setAxisLabel(PlotText(m_xLabel->text()));
                m_plot->getYAxis()->setAxisLabel(PlotText(m_yLabel->text()));
                m_plot->redrawPlot();
            });
        }
        connect(fit, &QPushButton::clicked, this, [this] { m_plot->zoomToFit(); });
        connect(m_export, &QPushButton::clicked, this, [this] { exportPlot(); });
    }

    bool PlotPanel::isSummary() const {
        return m_results && m_results->mode == SimulationMode::Scatter;
    }

    void PlotPanel::setResults(std::shared_ptr<const RunResults> results) {
        m_results = std::move(results);
        populateSelectors();
        applyPreset();
    }

    void PlotPanel::populateSelectors() {
        const QSignalBlocker blockX(m_xQuantity), blockY(m_yQuantity), blockBody(m_body);
        const QSignalBlocker blockPreset(m_preset), blockConnection(m_connectPoints);
        m_xQuantity->clear();
        m_yQuantity->clear();
        m_body->clear();
        m_body->addItem("All bodies", -1);
        m_preset->clear();
        if (isSummary()) {
            for (const auto& field : plot::SummaryQuantities()) {
                m_xQuantity->addItem(Label(field.label, field.unit), static_cast<int>(field.quantity));
                m_yQuantity->addItem(Label(field.label, field.unit), static_cast<int>(field.quantity));
            }
            m_preset->addItems({"Wind speed / peak altitude", "Wind direction / peak altitude",
                               "Final east / final north", "Wind speed / maximum dynamic pressure"});
        } else {
            for (const auto& field : plot::DetailQuantities()) {
                m_xQuantity->addItem(Label(field.label, field.unit), static_cast<int>(field.quantity));
                m_yQuantity->addItem(Label(field.label, field.unit), static_cast<int>(field.quantity));
            }
            m_preset->addItems({"Time / altitude", "Time / dynamic pressure", "Time / Fst",
                               "Time / airspeed", "Time / angle of attack", "Downrange / altitude"});
        }
        size_t bodyCount = 0;
        if (m_results) {
            for (const auto& simulation : m_results->simulations) {
                bodyCount = std::max(bodyCount, isSummary()
                    ? simulation.bodyFinalPositions.size() : simulation.bodyResults.size());
            }
        }
        for (size_t body = 0; body < bodyCount; ++body) {
            m_body->addItem(QString("Body %1").arg(body + 1), static_cast<int>(body));
        }
        m_connectPoints->setChecked(!isSummary());
        m_title->setText(isSummary() ? "Scatter run summary" : "Flight history");
    }

    void PlotPanel::applyPreset() {
        const QSignalBlocker blockX(m_xQuantity), blockY(m_yQuantity);
        int x = 0, y = 0;
        if (isSummary()) {
            using Q = plot::SummaryQuantity;
            constexpr Q X[] = {Q::WindSpeed, Q::WindDirection, Q::FinalEast, Q::WindSpeed};
            constexpr Q Y[] = {Q::PeakAltitude, Q::PeakAltitude, Q::FinalNorth, Q::MaxDynamicPressure};
            const int index = std::clamp(m_preset->currentIndex(), 0, 3);
            x = static_cast<int>(X[index]);
            y = static_cast<int>(Y[index]);
        } else {
            using Q = plot::DetailQuantity;
            constexpr Q Y[] = {Q::Altitude, Q::DynamicPressure, Q::StaticMargin,
                               Q::Airspeed, Q::AttackAngleDegrees, Q::Altitude};
            const int index = std::clamp(m_preset->currentIndex(), 0, 5);
            x = static_cast<int>(index == 5 ? Q::Downrange : Q::TimeFromLaunch);
            y = static_cast<int>(Y[index]);
        }
        m_xQuantity->setCurrentIndex(m_xQuantity->findData(x));
        m_yQuantity->setCurrentIndex(m_yQuantity->findData(y));
        updateDefaultLabels();
        rebuildPlot();
    }

    void PlotPanel::updateDefaultLabels() {
        m_xLabel->setText(m_xQuantity->currentText());
        m_yLabel->setText(m_yQuantity->currentText());
    }

    void PlotPanel::rebuildPlot() {
        m_plot->clearGraphs();
        m_plot->getDatastore()->clear();
        m_export->setEnabled(false);
        if (!m_results || m_results->simulations.empty()) {
            m_status->setText("Run a simulation to plot its results.");
            m_plot->redrawPlot();
            return;
        }
        try {
            if (isSummary()) addSummaryGraphs();
            else addDetailGraphs();
            m_plot->getPlotter()->setPlotLabel(PlotText(m_title->text()));
            m_plot->getXAxis()->setAxisLabel(PlotText(m_xLabel->text()));
            m_plot->getYAxis()->setAxisLabel(PlotText(m_yLabel->text()));
            m_plot->zoomToFit();
            m_plot->redrawPlot();
            m_export->setEnabled(m_plot->getPlotter()->getGraphCount() > 0);
            m_status->setText(isSummary()
                ? "One point per run. Input wind direction retains its stored north reference."
                : "Samples follow simulation order. Drag to zoom; use Fit axes to restore the full range.");
        } catch (const std::exception& error) {
            m_plot->clearGraphs();
            m_plot->getDatastore()->clear();
            m_plot->redrawPlot();
            m_status->setText(QString::fromUtf8(error.what()));
        }
    }

    void PlotPanel::addDetailGraphs() {
        const auto& result = m_results->simulations.front();
        const auto xQuantity = static_cast<plot::DetailQuantity>(m_xQuantity->currentData().toInt());
        const auto yQuantity = static_cast<plot::DetailQuantity>(m_yQuantity->currentData().toInt());
        m_body->setEnabled(true);
        const int selectedBody = m_body->currentData().toInt();
        GeographicResult geographic;
        const bool needsGeography = plot::NeedsGeography(xQuantity) || plot::NeedsGeography(yQuantity);
        if (needsGeography) {
            const auto& site = m_results->environment.launchSite;
            geographic = GeographicResultAdapter::Convert(
                result, GeoCoordinate(site.latitude, site.longitude, site.coordinateZone));
        }
        for (size_t body = 0; body < result.bodyResults.size(); ++body) {
            if (selectedBody >= 0 && body != static_cast<size_t>(selectedBody)) continue;
            std::vector<double> x, y;
            const auto& steps = result.bodyResults[body].steps;
            x.reserve(steps.size());
            y.reserve(steps.size());
            for (size_t row = 0; row < steps.size(); ++row) {
                const auto* position = needsGeography ? &geographic.bodyStepPositions[body][row] : nullptr;
                x.push_back(plot::DetailValue(steps[row], xQuantity, position));
                y.push_back(plot::DetailValue(steps[row], yQuantity, position));
            }
            AddCurve(m_plot, x, y, QString("Body %1").arg(body + 1), m_connectPoints->isChecked());
        }
    }

    void PlotPanel::addSummaryGraphs() {
        const auto xQuantity = static_cast<plot::SummaryQuantity>(m_xQuantity->currentData().toInt());
        const auto yQuantity = static_cast<plot::SummaryQuantity>(m_yQuantity->currentData().toInt());
        const bool needsBody = plot::NeedsBody(xQuantity) || plot::NeedsBody(yQuantity);
        const bool needsGeography = plot::NeedsGeography(xQuantity) || plot::NeedsGeography(yQuantity);
        m_body->setEnabled(needsBody);
        const int selectedBody = m_body->currentData().toInt();
        std::map<double, std::vector<const SimulationResult*>> windGroups;
        for (const auto& result : m_results->simulations) windGroups[result.windSpeed].push_back(&result);
        for (const auto& [windSpeed, group] : windGroups) {
            const size_t bodyCount = needsBody ? group.front()->bodyFinalPositions.size() : 1;
            for (size_t body = 0; body < bodyCount; ++body) {
                if (needsBody && selectedBody >= 0 && body != static_cast<size_t>(selectedBody)) continue;
                std::vector<double> x, y;
                for (const auto* result : group) {
                    if (needsBody && body >= result->bodyFinalPositions.size()) continue;
                    GeographicPosition finalPosition;
                    if (needsGeography) {
                        const auto& site = m_results->environment.launchSite;
                        const auto& final = result->bodyFinalPositions[body];
                        const auto [latitude, longitude] =
                            GeoCoordinate(site.latitude, site.longitude, site.coordinateZone).LatLonAt(final.x, final.y);
                        finalPosition = {latitude, longitude};
                    }
                    const auto* position = needsGeography ? &finalPosition : nullptr;
                    x.push_back(plot::SummaryValue(*result, xQuantity, body, position));
                    y.push_back(plot::SummaryValue(*result, yQuantity, body, position));
                }
                QString title = QString("Wind %1 m/s").arg(windSpeed);
                if (needsBody) title += QString(", body %1").arg(body + 1);
                AddCurve(m_plot, x, y, title, m_connectPoints->isChecked());
            }
        }
    }

    void PlotPanel::exportFigure(const QString& destination) {
        if (!m_results || m_plot->getGraphCount() == 0) {
            throw std::invalid_argument{"Run a simulation and select quantities before exporting."};
        }
        SaveReport(m_plot, destination);
    }

    void PlotPanel::exportPlot() {
        QString selectedFilter;
        QString destination = QFileDialog::getSaveFileName(this, "Export report figure", "flight-plot.png",
            "PNG image (*.png);;SVG vector image (*.svg);;PDF figure (*.pdf)", &selectedFilter);
        if (destination.isEmpty()) return;
        if (QFileInfo(destination).suffix().isEmpty()) {
            destination += selectedFilter.startsWith("SVG") ? ".svg"
                : selectedFilter.startsWith("PDF") ? ".pdf" : ".png";
        }
        try {
            exportFigure(destination);
            m_status->setText("Exported report figure: " + destination);
        } catch (const std::exception& error) {
            QMessageBox::warning(this, "Plot export failed", QString::fromUtf8(error.what()));
        }
    }
}
