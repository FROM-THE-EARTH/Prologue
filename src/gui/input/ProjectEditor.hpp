#pragma once

#include <filesystem>
#include <memory>
#include <vector>

#include <QWidget>

#include "gui/input/NumericInput.hpp"
#include "project/ProjectDocument.hpp"

class QLineEdit;
class QTabWidget;
class QTableWidget;

class ProjectEditor final : public QWidget {
    struct BodyWidgets;

    std::filesystem::path m_projectFile;
    int m_formatVersion = project::CurrentFormatVersion;
    QLineEdit* m_place = nullptr;
    NumericInput* m_latitude = nullptr;
    NumericInput* m_longitude = nullptr;
    QLineEdit* m_coordinateZone = nullptr;
    NumericInput* m_magneticDeclination = nullptr;
    NumericInput* m_railLength = nullptr;
    NumericInput* m_railAzimuth = nullptr;
    NumericInput* m_railElevation = nullptr;
    QTabWidget* m_bodyTabs = nullptr;
    QTableWidget* m_separations = nullptr;
    std::vector<std::unique_ptr<BodyWidgets>> m_bodies;

    void rebuildBodyTabs(const std::vector<project::Body>& bodies);
    void addBody(const project::Body& body = {});
    void removeBody();
    void setSingleTopology();
    void setLegacyTripleTopology();
    void addSeparation();
    void removeSeparation();

public:
    explicit ProjectEditor(QWidget* parent = nullptr);
    ~ProjectEditor() override;

    void setDocument(const project::Document& document, std::filesystem::path projectFile);
    [[nodiscard]] project::Document document() const;
};
