#pragma once

#include <memory>

#include <QWidget>

class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QPushButton;
class JKQTPlotter;

namespace gui {
    struct RunResults;

    class PlotPanel final : public QWidget {
    public:
        explicit PlotPanel(QWidget* parent = nullptr);
        void setResults(std::shared_ptr<const RunResults> results);
        void exportFigure(const QString& destination);

    private:
        std::shared_ptr<const RunResults> m_results;
        JKQTPlotter* m_plot;
        QComboBox* m_xQuantity;
        QComboBox* m_yQuantity;
        QComboBox* m_body;
        QComboBox* m_preset;
        QCheckBox* m_connectPoints;
        QLineEdit* m_title;
        QLineEdit* m_xLabel;
        QLineEdit* m_yLabel;
        QLabel* m_status;
        QPushButton* m_export;

        bool isSummary() const;
        void populateSelectors();
        void applyPreset();
        void updateDefaultLabels();
        void rebuildPlot();
        void addDetailGraphs();
        void addSummaryGraphs();
        void exportPlot();
    };
}
