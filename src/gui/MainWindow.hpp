#pragma once

#include <filesystem>
#include <memory>
#include <vector>

#include <QMainWindow>
#include <QFutureWatcher>

#include "gui/SimulationController.hpp"

class QAction;
class QComboBox;
class QLabel;
class QListWidget;
class QProgressBar;
class QTabWidget;
class NumericInput;
class ProjectEditor;
class SettingsEditor;

namespace gui {
    class PlotPanel;
    class MapWidget;

    class MainWindow final : public QMainWindow {
    public:
        explicit MainWindow(QWidget* parent = nullptr);
        void loadProject(const QString& file);
        void loadSettings(const QString& file);

    protected:
        void closeEvent(QCloseEvent* event) override;

    private:
        std::filesystem::path m_projectFile;
        std::filesystem::path m_settingsFile;
        ProjectEditor* m_project;
        SettingsEditor* m_settings;
        PlotPanel* m_plot;
        MapWidget* m_map;
        QTabWidget* m_tabs;
        QComboBox* m_mode;
        QComboBox* m_falling;
        QComboBox* m_detachment;
        NumericInput* m_windSpeed;
        NumericInput* m_windDirection;
        NumericInput* m_detachTime;
        QListWidget* m_overlays;
        QLabel* m_summary;
        QProgressBar* m_progress;
        QFutureWatcher<RunOutcome>* m_worker;
        std::shared_ptr<const RunResults> m_results;
        std::vector<QAction*> m_inputActions;
        QAction* m_runAction;
        QAction* m_exportAction;
        int m_resultPrecision = 8;
        std::string m_savedProject;
        std::string m_savedSettings;

        void saveProject(bool chooseFile);
        void saveSettings(bool chooseFile);
        void runSimulation();
        void runFinished();
        void exportResults();
        void loadOverlay();
        void exportMap();
        void setBusy(bool busy);
        void showError(const QString& message);
        bool confirmDiscard(bool project, bool settings);
    };
}
