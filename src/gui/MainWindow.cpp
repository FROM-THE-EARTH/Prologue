#include "MainWindow.hpp"

#include <algorithm>
#include <stdexcept>

#include <QtConcurrent/QtConcurrentRun>
#include <QAction>
#include <QCloseEvent>
#include <QComboBox>
#include <QDateTime>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QMenuBar>
#include <QMessageBox>
#include <QProgressBar>
#include <QPushButton>
#include <QScrollArea>
#include <QStatusBar>
#include <QTabWidget>
#include <QToolBar>
#include <QVBoxLayout>

#include "gui/FileActions.hpp"
#include "gui/input/NumericInput.hpp"
#include "gui/input/ProjectEditor.hpp"
#include "gui/input/SettingsEditor.hpp"
#include "gui/map/MapWidget.hpp"
#include "gui/plot/PlotPanel.hpp"
#include "io/ApplicationSettingsReader.hpp"
#include "io/ApplicationSettingsWriter.hpp"
#include "io/ProjectJsonSerializer.hpp"

namespace gui {
    namespace {
        QScrollArea* Scroll(QWidget* widget) {
            auto* scroll = new QScrollArea;
            scroll->setWidget(widget);
            scroll->setWidgetResizable(true);
            return scroll;
        }
        QString JsonFilename(QString file) {
            if (!file.isEmpty() && QFileInfo(file).suffix().isEmpty()) file += ".json";
            return file;
        }
    }

    MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
        setWindowTitle("Prologue GUI Demo");
        resize(1280, 880);
        m_worker = new QFutureWatcher<RunOutcome>(this);
        auto* toolbar = addToolBar(QStringLiteral("ファイルと実行"));
        toolbar->setMovable(false);
        const auto inputAction = [&](const QString& label, auto callback) {
            auto* action = toolbar->addAction(label);
            connect(action, &QAction::triggered, this, callback);
            m_inputActions.push_back(action);
            return action;
        };
        inputAction(QStringLiteral("諸元を開く"), [this] {
            if (!confirmDiscard(true, false)) return;
            const auto file = QFileDialog::getOpenFileName(this, QStringLiteral("諸元JSON"), PathText(m_projectFile), "JSON (*.json)");
            if (!file.isEmpty()) { try { loadProject(file); } catch (const std::exception& e) { showError(QString::fromUtf8(e.what())); } }
        });
        inputAction(QStringLiteral("諸元を保存"), [this] { saveProject(false); });
        inputAction(QStringLiteral("諸元を別名保存"), [this] { saveProject(true); });
        toolbar->addSeparator();
        inputAction(QStringLiteral("設定を開く"), [this] {
            if (!confirmDiscard(false, true)) return;
            const auto file = QFileDialog::getOpenFileName(this, QStringLiteral("prologue.settings.json"), PathText(m_settingsFile), "JSON (*.json)");
            if (!file.isEmpty()) { try { loadSettings(file); } catch (const std::exception& e) { showError(QString::fromUtf8(e.what())); } }
        });
        inputAction(QStringLiteral("設定を保存"), [this] { saveSettings(false); });
        inputAction(QStringLiteral("設定を別名保存"), [this] { saveSettings(true); });
        toolbar->addSeparator();
        m_runAction = toolbar->addAction(QStringLiteral("計算を実行"));
        connect(m_runAction, &QAction::triggered, this, [this] { runSimulation(); });
        m_exportAction = toolbar->addAction(QStringLiteral("CSV/KMLを保存"));
        m_exportAction->setEnabled(false);
        connect(m_exportAction, &QAction::triggered, this, [this] { exportResults(); });

        auto* central = new QWidget;
        auto* layout = new QVBoxLayout(central);
        auto* runBox = new QGroupBox(QStringLiteral("実行条件（CLIと同じ角度・単位）"));
        auto* runLayout = new QHBoxLayout(runBox);
        auto* first = new QFormLayout;
        auto* second = new QFormLayout;
        m_mode = new QComboBox;
        m_mode->setObjectName("runMode");
        m_mode->addItem("Detail", static_cast<int>(SimulationMode::Detail));
        m_mode->addItem("Scatter", static_cast<int>(SimulationMode::Scatter));
        m_falling = new QComboBox;
        m_falling->addItem(QStringLiteral("弾道"), static_cast<int>(TrajectoryMode::Trajectory));
        m_falling->addItem(QStringLiteral("パラシュート"), static_cast<int>(TrajectoryMode::Parachute));
        m_falling->setObjectName("fallingMode");
        m_detachment = new QComboBox;
        m_detachment->addItem(QStringLiteral("燃焼終了時"), static_cast<int>(DetachType::BurningFinished));
        m_detachment->addItem(QStringLiteral("時刻指定"), static_cast<int>(DetachType::Time));
        m_detachment->addItem(QStringLiteral("パラシュートと同時"), static_cast<int>(DetachType::SyncPara));
        m_detachment->addItem(QStringLiteral("分離しない"), static_cast<int>(DetachType::DoNotDeatch));
        m_windSpeed = new NumericInput;
        m_windSpeed->setObjectName("groundWindSpeed");
        m_windSpeed->setValue(3.0);
        m_windDirection = new NumericInput;
        m_windDirection->setValue(0.0);
        m_detachTime = new NumericInput;
        m_detachTime->setValue(7.0);
        first->addRow(QStringLiteral("計算モード"), m_mode);
        first->addRow(QStringLiteral("落下方法"), m_falling);
        first->addRow(QStringLiteral("分離方法"), m_detachment);
        second->addRow(QStringLiteral("地上風速 [m/s]"), m_windSpeed);
        second->addRow(QStringLiteral("風向（磁北0・東90）[deg]"), m_windDirection);
        second->addRow(QStringLiteral("分離時刻 [s]"), m_detachTime);
        runLayout->addLayout(first);
        runLayout->addLayout(second);
        layout->addWidget(runBox);
        m_project = new ProjectEditor;
        m_settings = new SettingsEditor;
        m_plot = new PlotPanel;
        m_map = new MapWidget;
        m_tabs = new QTabWidget;
        m_tabs->addTab(Scroll(m_project), QStringLiteral("諸元"));
        m_tabs->addTab(Scroll(m_settings), QStringLiteral("計算設定"));
        m_tabs->addTab(m_plot, QStringLiteral("グラフ"));
        auto* mapPage = new QWidget;
        auto* mapLayout = new QVBoxLayout(mapPage);
        auto* mapButtons = new QHBoxLayout;
        auto* overlayButton = new QPushButton(QStringLiteral("KML/KMZを重ねる"));
        auto* clearButton = new QPushButton(QStringLiteral("重畳をクリア"));
        auto* fitButton = new QPushButton(QStringLiteral("全体表示"));
        auto* saveMapButton = new QPushButton(QStringLiteral("地図をPNG/SVG/PDF保存"));
        mapButtons->addWidget(overlayButton);
        mapButtons->addWidget(clearButton);
        mapButtons->addWidget(fitButton);
        mapButtons->addStretch();
        mapButtons->addWidget(saveMapButton);
        mapLayout->addLayout(mapButtons);
        auto* mapContent = new QHBoxLayout;
        m_overlays = new QListWidget;
        m_overlays->setMaximumWidth(260);
        mapContent->addWidget(m_overlays);
        mapContent->addWidget(m_map, 1);
        mapLayout->addLayout(mapContent, 1);
        auto* attribution = new QLabel(QStringLiteral("出典: <a href=\"https://maps.gsi.go.jp/development/ichiran.html\">地理院タイル</a> ｜ ドラッグで移動・ホイールで拡大縮小"));
        attribution->setOpenExternalLinks(true);
        mapLayout->addWidget(attribution);
        m_tabs->addTab(mapPage, QStringLiteral("地図"));
        layout->addWidget(m_tabs, 1);
        m_summary = new QLabel(QStringLiteral("諸元と計算設定を確認して、計算を実行してください。"));
        m_summary->setWordWrap(true);
        layout->addWidget(m_summary);
        m_progress = new QProgressBar;
        m_progress->hide();
        layout->addWidget(m_progress);
        setCentralWidget(central);

        connect(overlayButton, &QPushButton::clicked, this, [this] { loadOverlay(); });
        connect(clearButton, &QPushButton::clicked, this, [this] { m_map->clearOverlays(); m_overlays->clear(); });
        connect(fitButton, &QPushButton::clicked, this, [this] { m_map->fitAll(); });
        connect(saveMapButton, &QPushButton::clicked, this, [this] { exportMap(); });
        connect(m_overlays, &QListWidget::itemChanged, this, [this](QListWidgetItem* item) {
            m_map->setOverlayVisible(static_cast<size_t>(m_overlays->row(item)), item->checkState() == Qt::Checked);
        });
        m_map->setStatusCallback([this](const QString& message) { statusBar()->showMessage(message); });
        connect(m_worker, &QFutureWatcher<RunOutcome>::finished, this, [this] { runFinished(); });
        auto* help = menuBar()->addMenu(QStringLiteral("ヘルプ"));
        auto* license = help->addAction(QStringLiteral("ライセンスと出典"));
        connect(license, &QAction::triggered, this, [this] {
            QMessageBox::about(this, "Prologue GUI Demo", QStringLiteral(
                "Prologue: MIT<br>Qt 6: LGPL-3.0（動的リンク）<br>JKQTPlotter: LGPL-2.1-or-later（動的リンク）<br>"
                "miniz: MIT<br>出典: <a href='https://maps.gsi.go.jp/development/ichiran.html'>地理院タイル</a><br>"
                "第三者ライセンスの全文は実行ファイル横の licenses フォルダを参照してください。"));
        });
    }

    void MainWindow::loadProject(const QString& file) {
        const auto path = FilePath(QFileInfo(file).absoluteFilePath());
        const auto document = ProjectJsonSerializer::Load(path);
        m_project->setDocument(document, path);
        m_savedProject = ProjectJsonSerializer::Serialize(m_project->document());
        m_projectFile = path;
        m_map->setCenter(document.environment.latitude, document.environment.longitude);
        setWindowTitle(QString("Prologue GUI Demo — %1").arg(QFileInfo(file).fileName()));
        statusBar()->showMessage(QStringLiteral("諸元を読み込みました: ") + file);
    }

    void MainWindow::loadSettings(const QString& file) {
        const auto path = FilePath(QFileInfo(file).absoluteFilePath());
        const auto read = ApplicationSettingsReader::Read(path);
        m_settings->setSettings(read.value, path);
        m_savedSettings = ApplicationSettingsWriter::Serialize(m_settings->settings());
        m_settingsFile = path;
        if (read.value.solver.wind.type == WindModelType::Real || read.value.solver.wind.type == WindModelType::NoWind) m_mode->setCurrentIndex(0);
        QStringList messages;
        for (const auto& diagnostic : read.diagnostics) {
            for (const auto& line : diagnostic.lines) messages.append(QString::fromStdString(line));
        }
        statusBar()->showMessage(messages.isEmpty() ? QStringLiteral("計算設定を読み込みました。") : messages.join(" "));
    }

    void MainWindow::saveProject(bool chooseFile) {
        try {
            QString file = PathText(m_projectFile);
            if (chooseFile || file.isEmpty()) file = JsonFilename(QFileDialog::getSaveFileName(this, QStringLiteral("諸元を保存"), file, "JSON (*.json)"));
            if (file.isEmpty()) return;
            auto document = m_project->document();
            const auto destination = FilePath(QFileInfo(file).absoluteFilePath());
            if (!m_projectFile.empty()) RebaseResources(document, m_projectFile, destination);
            WriteAtomically(file, ProjectJsonSerializer::Serialize(document));
            m_projectFile = destination;
            m_project->setDocument(document, destination);
            m_savedProject = ProjectJsonSerializer::Serialize(document);
            setWindowTitle(QString("Prologue GUI Demo — %1").arg(QFileInfo(file).fileName()));
            statusBar()->showMessage(QStringLiteral("諸元を保存しました。"));
        } catch (const std::exception& error) { showError(QString::fromUtf8(error.what())); }
    }

    void MainWindow::saveSettings(bool chooseFile) {
        try {
            QString file = PathText(m_settingsFile);
            if (chooseFile || file.isEmpty()) file = JsonFilename(QFileDialog::getSaveFileName(this, QStringLiteral("計算設定を保存"), file, "JSON (*.json)"));
            if (file.isEmpty()) return;
            auto settings = m_settings->settings();
            const auto destination = FilePath(QFileInfo(file).absoluteFilePath());
            if (!m_settingsFile.empty() && destination.parent_path() != m_settingsFile.parent_path()) {
                const auto filename = FilePath(QString::fromStdString(settings.measuredWindFilename));
                if (!filename.empty() && filename.is_relative()) {
                    settings.measuredWindFilename = PathText(m_settingsFile.parent_path() / "input/wind" / filename).toStdString();
                }
            }
            WriteAtomically(file, ApplicationSettingsWriter::Serialize(settings));
            m_settingsFile = destination;
            m_settings->setSettings(settings, destination);
            m_savedSettings = ApplicationSettingsWriter::Serialize(settings);
            statusBar()->showMessage(QStringLiteral("計算設定を保存しました。"));
        } catch (const std::exception& error) { showError(QString::fromUtf8(error.what())); }
    }

    void MainWindow::runSimulation() {
        if (m_worker->isRunning()) return;
        try {
            RunRequest request{.document = m_project->document(), .settings = m_settings->settings()};
            request.projectFile = m_projectFile;
            request.settingsFile = m_settingsFile;
            request.mode = static_cast<SimulationMode>(m_mode->currentData().toInt());
            request.run.trajectoryMode = static_cast<TrajectoryMode>(m_falling->currentData().toInt());
            request.run.detachType = static_cast<DetachType>(m_detachment->currentData().toInt());
            request.run.detachTime = m_detachTime->value("Detachment time [s]");
            request.run.windSpeed = m_windSpeed->value("Ground wind speed [m/s]");
            request.run.windDirection = {m_windDirection->value("Wind direction [deg]"), DirectionReference::MagneticNorth};
            m_resultPrecision = request.settings.result.precision;
            m_results.reset();
            m_plot->setResults(nullptr);
            m_map->setSeries({});
            setBusy(true);
            m_summary->setText(QStringLiteral("計算中… 編集中の値はこの実行には反映されません。"));
            m_worker->setFuture(QtConcurrent::run([request] { return RunSimulation(request); }));
        } catch (const std::exception& error) { showError(QString::fromUtf8(error.what())); }
    }

    void MainWindow::runFinished() {
        setBusy(false);
        try {
            const auto outcome = m_worker->result();
            if (!outcome.error.isEmpty()) {
                m_summary->setText(QStringLiteral("計算に失敗しました。入力を確認してください。"));
                showError(outcome.error);
                return;
            }
            m_results = outcome.results;
            m_plot->setResults(m_results);
            m_map->setSeries(outcome.mapSeries);
            m_map->fitAll();
            m_exportAction->setEnabled(true);
            double maxAltitude = 0.0, maxAirspeed = 0.0, maxPressure = 0.0;
            for (const auto& result : m_results->simulations) {
                maxAltitude = std::max(maxAltitude, result.maxAltitude);
                maxAirspeed = std::max(maxAirspeed, result.maxAirspeed);
                maxPressure = std::max(maxPressure, result.maxDynamicPressureDuringRising);
            }
            m_summary->setText(QStringLiteral("完了: %1条件 ｜ 最高高度 %2 m ｜ 最大対気速度 %3 m/s ｜ 最大動圧 %4 Pa")
                .arg(m_results->simulations.size()).arg(maxAltitude, 0, 'g', 7)
                .arg(maxAirspeed, 0, 'g', 7).arg(maxPressure, 0, 'g', 7));
            m_tabs->setCurrentIndex(2);
        } catch (const std::exception& error) {
            m_results.reset();
            m_exportAction->setEnabled(false);
            m_summary->setText(QStringLiteral("結果の表示に失敗しました。"));
            showError(QString::fromUtf8(error.what()));
        }
    }

    void MainWindow::exportResults() {
        if (!m_results) return;
        try {
            const auto parent = QFileDialog::getExistingDirectory(this, QStringLiteral("結果フォルダを作成する場所"), PathText(m_projectFile.parent_path()));
            if (parent.isEmpty()) return;
            const auto name = QString("Prologue-result-%1").arg(QDateTime::currentDateTime().toString("yyyyMMdd-HHmmss-zzz"));
            const auto directory = FilePath(QDir(parent).filePath(name));
            std::filesystem::create_directories(directory);
            SaveResults(*m_results, directory, m_resultPrecision);
            statusBar()->showMessage(QStringLiteral("CSV/KMLを保存しました: ") + PathText(directory));
        } catch (const std::exception& error) { showError(QString::fromUtf8(error.what())); }
    }

    void MainWindow::loadOverlay() {
        const auto files = QFileDialog::getOpenFileNames(this, QStringLiteral("KML/KMZを重ねる"), PathText(m_projectFile.parent_path()), "KML/KMZ (*.kml *.kmz)");
        for (const auto& file : files) {
            QString error;
            if (!m_map->loadOverlay(file, &error)) { showError(error); continue; }
            auto* item = new QListWidgetItem(m_map->overlayNames().last(), m_overlays);
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            item->setCheckState(Qt::Checked);
        }
        m_map->fitAll();
    }

    void MainWindow::exportMap() {
        QString selectedFilter;
        QString file = QFileDialog::getSaveFileName(this, QStringLiteral("地図を保存"), "flight-map.png", "PNG (*.png);;SVG (*.svg);;PDF (*.pdf)", &selectedFilter);
        if (file.isEmpty()) return;
        if (QFileInfo(file).suffix().isEmpty()) {
            file += selectedFilter.startsWith("SVG") ? ".svg" : selectedFilter.startsWith("PDF") ? ".pdf" : ".png";
        }
        QString error;
        const int height = static_cast<int>(2400.0 * m_map->height() / std::max(1, m_map->width()));
        if (!m_map->exportFigure(file, QSize(2400, height), &error)) showError(error);
        else statusBar()->showMessage(QStringLiteral("地図を保存しました。"));
    }

    void MainWindow::setBusy(bool busy) {
        for (auto* action : m_inputActions) action->setEnabled(!busy);
        m_runAction->setEnabled(!busy);
        m_exportAction->setEnabled(!busy && static_cast<bool>(m_results));
        m_project->setEnabled(!busy);
        m_settings->setEnabled(!busy);
        m_progress->setRange(0, busy ? 0 : 100);
        m_progress->setVisible(busy);
    }

    void MainWindow::showError(const QString& message) {
        statusBar()->showMessage(message);
        QMessageBox::critical(this, QStringLiteral("Prologue"), message);
    }

    bool MainWindow::confirmDiscard(bool project, bool settings) {
        bool changed = false;
        try {
            if (project && !m_savedProject.empty()) changed = ProjectJsonSerializer::Serialize(m_project->document()) != m_savedProject;
            if (settings && !m_savedSettings.empty()) changed = changed || ApplicationSettingsWriter::Serialize(m_settings->settings()) != m_savedSettings;
        } catch (const std::exception&) {
            changed = true; // Incomplete numeric edits must also be protected.
        }
        if (!changed) return true;
        return QMessageBox::question(this, QStringLiteral("未保存の変更"),
            QStringLiteral("諸元または計算設定に未保存の変更があります。変更を破棄して続けますか？\n保存する場合はキャンセルして、保存ボタンを使ってください。"),
            QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Cancel) == QMessageBox::Discard;
    }

    void MainWindow::closeEvent(QCloseEvent* event) {
        if (m_worker->isRunning()) {
            statusBar()->showMessage(QStringLiteral("計算完了後にウィンドウを閉じてください。"));
            event->ignore();
        } else if (confirmDiscard(true, true)) event->accept();
        else event->ignore();
    }
}
