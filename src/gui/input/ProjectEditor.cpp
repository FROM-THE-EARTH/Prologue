#include "ProjectEditor.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>

#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QIntValidator>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QStringList>
#include <QTableWidget>
#include <QTabWidget>
#include <QVBoxLayout>

namespace {
    std::filesystem::path ProjectBase(const std::filesystem::path& projectFile);

    NumericInput* AddNumber(QFormLayout* form,
                            const QString& label,
                            double value,
                            QWidget* parent,
                            const QString& objectName = {}) {
        auto* input = new NumericInput(parent);
        input->setValue(value);
        if (!objectName.isEmpty()) input->setObjectName(objectName);
        form->addRow(label, input);
        return input;
    }

    QLineEdit* AddText(QFormLayout* form,
                       const QString& label,
                       const QString& value,
                       QWidget* parent,
                       const QString& objectName = {}) {
        auto* input = new QLineEdit(value, parent);
        if (!objectName.isEmpty()) input->setObjectName(objectName);
        form->addRow(label, input);
        return input;
    }

    QString PathText(const std::optional<std::filesystem::path>& path,
                     const std::filesystem::path& projectFile) {
        if (!path) return {};
        auto displayPath = *path;
        if (displayPath.is_absolute()) {
            const auto relative = displayPath.lexically_relative(ProjectBase(projectFile));
            if (!relative.empty()) displayPath = relative;
        }
        return QString::fromStdU16String(displayPath.generic_u16string());
    }

    std::optional<std::filesystem::path> PathValue(const QLineEdit* input) {
        const auto value = input->text().trimmed();
        if (value.isEmpty()) return std::nullopt;
        return std::filesystem::path(value.toStdU16String());
    }

    std::filesystem::path ProjectBase(const std::filesystem::path& projectFile) {
        const auto parent = projectFile.parent_path();
        return projectFile.empty() || parent.empty() ? std::filesystem::current_path() : parent;
    }

    void BrowseReferencePath(QLineEdit* input,
                             const std::filesystem::path& projectFile,
                             QWidget* parent,
                             const QString& filter) {
        const auto currentText = input->text().trimmed();
        std::filesystem::path start = ProjectBase(projectFile);
        if (!currentText.isEmpty()) {
            auto current = std::filesystem::path(currentText.toStdU16String());
            if (current.is_relative()) current = start / current;
            start = current;
        }
        const auto selected = QFileDialog::getOpenFileName(parent,
                                                           QObject::tr("Select referenced file"),
                                                           QString::fromStdU16String(start.generic_u16string()),
                                                           filter);
        if (selected.isEmpty()) return;

        const std::filesystem::path selectedPath(selected.toStdU16String());
        std::error_code error;
        auto relative = std::filesystem::relative(selectedPath, ProjectBase(projectFile), error);
        if (error) relative = selectedPath;
        input->setText(QString::fromStdU16String(relative.generic_u16string()));
    }

    QString CellText(const QTableWidget* table, int row, int column) {
        const auto* item = table->item(row, column);
        return item == nullptr ? QString{} : item->text().trimmed();
    }

    NumericInput* TableNumber(QTableWidget* table, int row, int column) {
        auto* input = new NumericInput(table);
        table->setCellWidget(row, column, input);
        return input;
    }

    double TableNumberValue(const QTableWidget* table,
                            int row,
                            int column,
                            const QString& label) {
        if (auto* input = dynamic_cast<NumericInput*>(table->cellWidget(row, column))) {
            return input->value(label);
        }
        bool ok = false;
        const auto text = CellText(table, row, column);
        const double value = text.toDouble(&ok);
        if (!ok || !std::isfinite(value)) {
            throw std::invalid_argument((label + QStringLiteral(" must be a finite number.")).toStdString());
        }
        return value;
    }

    std::optional<double> OptionalTableNumber(const QTableWidget* table,
                                              int row,
                                              int column,
                                              const QString& label) {
        auto* input = dynamic_cast<NumericInput*>(table->cellWidget(row, column));
        if (input != nullptr) return input->optionalValue(label);
        const auto text = CellText(table, row, column);
        if (text.isEmpty()) return std::nullopt;
        bool ok = false;
        const double value = text.toDouble(&ok);
        if (!ok || !std::isfinite(value)) {
            throw std::invalid_argument((label + QStringLiteral(" must be empty or a finite number.")).toStdString());
        }
        return value;
    }

    void SetTableNumber(QTableWidget* table, int row, int column, double value) {
        TableNumber(table, row, column)->setValue(value);
    }

    NumericInput* OptionalTableInput(QTableWidget* table, int row, int column) {
        auto* input = new NumericInput(table);
        table->setCellWidget(row, column, input);
        return input;
    }

    void SetOptionalTableNumber(QTableWidget* table,
                                int row,
                                int column,
                                const std::optional<double>& value) {
        auto* input = OptionalTableInput(table, row, column);
        if (value) input->setValue(*value);
    }

    QTableWidget* NewTable(QWidget* parent,
                           const QStringList& headers,
                           int rows = 0) {
        auto* table = new QTableWidget(rows, headers.size(), parent);
        table->setHorizontalHeaderLabels(headers);
        table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        table->verticalHeader()->setVisible(false);
        table->setSelectionBehavior(QAbstractItemView::SelectRows);
        table->setSelectionMode(QAbstractItemView::SingleSelection);
        return table;
    }

    void AddTableRow(QTableWidget* table) {
        const int row = table->rowCount();
        table->insertRow(row);
        for (int column = 0; column < table->columnCount(); ++column) {
            if (table->cellWidget(row, column) == nullptr) {
                table->setItem(row, column, new QTableWidgetItem{});
            }
        }
    }

    void RemoveSelectedTableRow(QTableWidget* table) {
        if (table->currentRow() >= 0) table->removeRow(table->currentRow());
    }

    QString ProductsText(const std::vector<size_t>& indices) {
        QStringList values;
        for (const auto index : indices) values.push_back(QString::number(static_cast<qulonglong>(index)));
        return values.join(QStringLiteral(","));
    }

    std::vector<size_t> ParseProducts(const QString& text, const QString& fieldName) {
        std::vector<size_t> result;
        if (text.trimmed().isEmpty()) return result;
        for (const auto& part : text.split(',', Qt::KeepEmptyParts)) {
            if (part.trimmed().isEmpty()) {
                throw std::invalid_argument((fieldName + QStringLiteral(" contains an empty body index.")).toStdString());
            }
            bool ok = false;
            const qulonglong index = part.trimmed().toULongLong(&ok);
            if (!ok || index > std::numeric_limits<size_t>::max()) {
                throw std::invalid_argument((fieldName + QStringLiteral(" must contain non-negative body indices separated by commas.")).toStdString());
            }
            result.push_back(static_cast<size_t>(index));
        }
        if (result.empty() && !text.trimmed().isEmpty()) {
            throw std::invalid_argument((fieldName + QStringLiteral(" must contain valid body indices.")).toStdString());
        }
        return result;
    }
}

struct ProjectEditor::BodyWidgets {
    QWidget* page = nullptr;
    NumericInput* length = nullptr;
    NumericInput* diameter = nullptr;
    NumericInput* massInitial = nullptr;
    NumericInput* massFinal = nullptr;
    NumericInput* cgInitial = nullptr;
    NumericInput* cgFinal = nullptr;
    NumericInput* inertiaInitial = nullptr;
    NumericInput* inertiaFinal = nullptr;
    NumericInput* pitchDamping = nullptr;
    NumericInput* thrustPressure = nullptr;
    NumericInput* nozzleDiameter = nullptr;
    NumericInput* centerOfPressure = nullptr;
    NumericInput* centerOfPressureAlpha = nullptr;
    NumericInput* dragInitial = nullptr;
    NumericInput* dragFinal = nullptr;
    NumericInput* dragAlphaSquared = nullptr;
    NumericInput* normalForce = nullptr;
    QLineEdit* thrustPath = nullptr;
    QLineEdit* coefficientPath = nullptr;
    QTableWidget* parachutes = nullptr;
    QTableWidget* transitions = nullptr;
};

ProjectEditor::ProjectEditor(QWidget* parent) : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);

    auto* environmentBox = new QGroupBox(tr("Launch environment"), this);
    auto* environmentForm = new QFormLayout(environmentBox);
    m_place = AddText(environmentForm, tr("Place"), {}, environmentBox, "place");
    m_latitude = AddNumber(environmentForm, tr("Latitude [deg]"), 0.0, environmentBox, "latitude");
    m_longitude = AddNumber(environmentForm, tr("Longitude [deg]"), 0.0, environmentBox, "longitude");
    m_coordinateZone = new QLineEdit(environmentBox);
    m_coordinateZone->setValidator(new QIntValidator(1, 19, m_coordinateZone));
    m_coordinateZone->setObjectName("coordinateZone");
    environmentForm->addRow(tr("Coordinate zone [1–19]"), m_coordinateZone);
    m_magneticDeclination = AddNumber(environmentForm, tr("Magnetic declination [deg]"), 0.0, environmentBox, "magneticDeclination");
    m_railLength = AddNumber(environmentForm, tr("Launch rail length [m]"), 0.0, environmentBox, "railLength");
    m_railAzimuth = AddNumber(environmentForm, tr("Launch rail azimuth [deg]"), 0.0, environmentBox, "railAzimuth");
    m_railElevation = AddNumber(environmentForm, tr("Launch rail elevation [deg]"), 0.0, environmentBox, "railElevation");
    layout->addWidget(environmentBox);

    auto* bodyControls = new QHBoxLayout;
    auto* addBodyButton = new QPushButton(tr("Add body"), this);
    auto* removeBodyButton = new QPushButton(tr("Remove selected body"), this);
    auto* singleButton = new QPushButton(tr("Single-body topology"), this);
    auto* tripleButton = new QPushButton(tr("Legacy three-body topology"), this);
    addBodyButton->setObjectName("addBodyButton");
    removeBodyButton->setObjectName("removeBodyButton");
    singleButton->setObjectName("singleTopologyButton");
    tripleButton->setObjectName("legacyTripleTopologyButton");
    bodyControls->addWidget(addBodyButton);
    bodyControls->addWidget(removeBodyButton);
    bodyControls->addStretch();
    bodyControls->addWidget(singleButton);
    bodyControls->addWidget(tripleButton);
    layout->addLayout(bodyControls);

    m_bodyTabs = new QTabWidget(this);
    m_bodyTabs->setObjectName("bodyTabs");
    layout->addWidget(m_bodyTabs, 1);

    auto* separationBox = new QGroupBox(tr("Separations"), this);
    auto* separationLayout = new QVBoxLayout(separationBox);
    m_separations = NewTable(separationBox,
                             {tr("Source body index (0-based)"), tr("Product body indices (comma-separated, 0-based)")});
    m_separations->setObjectName("separationsTable");
    separationLayout->addWidget(m_separations);
    auto* separationButtons = new QHBoxLayout;
    auto* addSeparationButton = new QPushButton(tr("Add separation"), separationBox);
    auto* removeSeparationButton = new QPushButton(tr("Remove selected separation"), separationBox);
    addSeparationButton->setObjectName("addSeparationButton");
    removeSeparationButton->setObjectName("removeSeparationButton");
    separationButtons->addWidget(addSeparationButton);
    separationButtons->addWidget(removeSeparationButton);
    separationButtons->addStretch();
    separationLayout->addLayout(separationButtons);
    layout->addWidget(separationBox);

    connect(addBodyButton, &QPushButton::clicked, this, [this] { addBody(); });
    connect(removeBodyButton, &QPushButton::clicked, this, [this] { removeBody(); });
    connect(singleButton, &QPushButton::clicked, this, [this] { setSingleTopology(); });
    connect(tripleButton, &QPushButton::clicked, this, [this] { setLegacyTripleTopology(); });
    connect(addSeparationButton, &QPushButton::clicked, this, [this] { addSeparation(); });
    connect(removeSeparationButton, &QPushButton::clicked, this, [this] { removeSeparation(); });

    rebuildBodyTabs({project::Body{}});
}

ProjectEditor::~ProjectEditor() = default;

void ProjectEditor::setDocument(const project::Document& document,
                                std::filesystem::path projectFile) {
    m_projectFile = std::move(projectFile);
    m_formatVersion = document.formatVersion;
    m_place->setText(QString::fromStdString(document.environment.place));
    m_latitude->setValue(document.environment.latitude);
    m_longitude->setValue(document.environment.longitude);
    m_coordinateZone->setText(QString::number(document.environment.coordinateZone));
    m_magneticDeclination->setValue(document.environment.magneticDeclination);
    m_railLength->setValue(document.environment.railLength);
    m_railAzimuth->setValue(document.environment.railAzimuth);
    m_railElevation->setValue(document.environment.railElevation);

    rebuildBodyTabs(document.bodies);
    m_separations->setRowCount(0);
    for (const auto& separation : document.separations) {
        const int row = m_separations->rowCount();
        AddTableRow(m_separations);
        m_separations->item(row, 0)->setText(QString::number(static_cast<qulonglong>(separation.sourceBodyIndex)));
        m_separations->item(row, 1)->setText(ProductsText(separation.productBodyIndices));
    }
}

project::Document ProjectEditor::document() const {
    project::Document result;
    result.formatVersion = m_formatVersion;
    result.environment.place = m_place->text().toStdString();
    result.environment.latitude = m_latitude->value(tr("Latitude [deg]"));
    result.environment.longitude = m_longitude->value(tr("Longitude [deg]"));
    bool zoneOk = false;
    const int zone = m_coordinateZone->text().toInt(&zoneOk);
    if (m_coordinateZone->text().trimmed().isEmpty() || !zoneOk || zone < 1 || zone > 19) {
        throw std::invalid_argument("Coordinate zone must be an integer in [1, 19].");
    }
    result.environment.coordinateZone = zone;
    result.environment.magneticDeclination = m_magneticDeclination->value(tr("Magnetic declination [deg]"));
    result.environment.railLength = m_railLength->value(tr("Launch rail length [m]"));
    result.environment.railAzimuth = m_railAzimuth->value(tr("Launch rail azimuth [deg]"));
    result.environment.railElevation = m_railElevation->value(tr("Launch rail elevation [deg]"));

    for (size_t bodyIndex = 0; bodyIndex < m_bodies.size(); ++bodyIndex) {
        const auto& controls = *m_bodies[bodyIndex];
        const QString prefix = tr("Body %1 ").arg(bodyIndex + 1);
        project::Body body;
        body.length = controls.length->value(prefix + tr("length [m]"));
        body.diameter = controls.diameter->value(prefix + tr("diameter [m]"));
        body.massInitial = controls.massInitial->value(prefix + tr("initial mass [kg]"));
        body.massFinal = controls.massFinal->value(prefix + tr("final mass [kg]"));
        body.centerOfGravityInitial = controls.cgInitial->value(prefix + tr("initial CG from nose [m]"));
        body.centerOfGravityFinal = controls.cgFinal->value(prefix + tr("final CG from nose [m]"));
        body.pitchYawMomentOfInertiaInitial = controls.inertiaInitial->value(prefix + tr("initial pitch/yaw inertia [kg m²]"));
        body.pitchYawMomentOfInertiaFinal = controls.inertiaFinal->value(prefix + tr("final pitch/yaw inertia [kg m²]"));
        body.pitchDampingMomentCoefficient = controls.pitchDamping->value(prefix + tr("pitch damping coefficient"));
        body.engine.thrustFile = PathValue(controls.thrustPath);
        body.engine.thrustMeasuredPressure = controls.thrustPressure->value(prefix + tr("measured thrust pressure [Pa]"));
        body.engine.nozzleDiameter = controls.nozzleDiameter->value(prefix + tr("nozzle diameter [m]"));
        body.aerodynamics.coefficientFile = PathValue(controls.coefficientPath);
        body.aerodynamics.centerOfPressure = controls.centerOfPressure->value(prefix + tr("center of pressure [m]"));
        body.aerodynamics.centerOfPressureAlpha = controls.centerOfPressureAlpha->value(prefix + tr("center of pressure alpha coefficient"));
        body.aerodynamics.dragCoefficientInitial = controls.dragInitial->value(prefix + tr("initial drag coefficient"));
        body.aerodynamics.dragCoefficientFinal = controls.dragFinal->value(prefix + tr("final drag coefficient"));
        body.aerodynamics.dragCoefficientAlphaSquared = controls.dragAlphaSquared->value(prefix + tr("alpha-squared drag coefficient"));
        body.aerodynamics.normalForceCoefficient = controls.normalForce->value(prefix + tr("normal force coefficient"));

        for (int row = 0; row < controls.parachutes->rowCount(); ++row) {
            project::Parachute parachute;
            parachute.openingTimeFromLaunch = OptionalTableNumber(
                controls.parachutes, row, 0, prefix + tr("parachute launch opening time [s]"));
            parachute.openingTimeFromPeak = OptionalTableNumber(
                controls.parachutes, row, 1, prefix + tr("parachute time after peak [s]"));
            parachute.openingHeight = OptionalTableNumber(
                controls.parachutes, row, 2, prefix + tr("parachute opening height [m]"));
            parachute.CdS = TableNumberValue(controls.parachutes, row, 3, prefix + tr("parachute CdS [m²]"));
            body.parachutes.push_back(parachute);
        }

        for (int row = 0; row < controls.transitions->rowCount(); ++row) {
            body.transitions.push_back({
                .time = TableNumberValue(controls.transitions, row, 0, prefix + tr("transition time [s]")),
                .mass = TableNumberValue(controls.transitions, row, 1, prefix + tr("transition mass change [kg]")),
                .Cd = TableNumberValue(controls.transitions, row, 2, prefix + tr("transition drag change")),
            });
        }
        result.bodies.push_back(std::move(body));
    }

    for (int row = 0; row < m_separations->rowCount(); ++row) {
        bool sourceOk = false;
        const auto sourceText = CellText(m_separations, row, 0);
        const qulonglong source = sourceText.toULongLong(&sourceOk);
        if (!sourceOk || source > std::numeric_limits<size_t>::max()) {
            throw std::invalid_argument("Separation source body index must be a non-negative integer.");
        }
        result.separations.push_back({
            .sourceBodyIndex = static_cast<size_t>(source),
            .productBodyIndices = ParseProducts(CellText(m_separations, row, 1), tr("Separation products")),
        });
    }
    return result;
}

void ProjectEditor::rebuildBodyTabs(const std::vector<project::Body>& bodies) {
    while (m_bodyTabs->count() > 0) {
        auto* page = m_bodyTabs->widget(0);
        m_bodyTabs->removeTab(0);
        delete page;
    }
    m_bodies.clear();
    for (const auto& body : bodies) addBody(body);
}

void ProjectEditor::addBody(const project::Body& body) {
    auto controls = std::make_unique<BodyWidgets>();
    controls->page = new QWidget(m_bodyTabs);
    auto* pageLayout = new QVBoxLayout(controls->page);
    auto* scroll = new QScrollArea(controls->page);
    scroll->setWidgetResizable(true);
    auto* content = new QWidget(scroll);
    auto* contentLayout = new QVBoxLayout(content);

    auto* bodyBox = new QGroupBox(tr("Body dimensions and mass properties"), content);
    auto* form = new QFormLayout(bodyBox);
    const auto objectPrefix = QStringLiteral("body%1_").arg(m_bodies.size());
    controls->length = AddNumber(form, tr("Length [m]"), body.length, bodyBox, objectPrefix + "length");
    controls->diameter = AddNumber(form, tr("Diameter [m]"), body.diameter, bodyBox, objectPrefix + "diameter");
    controls->massInitial = AddNumber(form, tr("Initial mass [kg]"), body.massInitial, bodyBox, objectPrefix + "massInitial");
    controls->massFinal = AddNumber(form, tr("Final mass [kg]"), body.massFinal, bodyBox, objectPrefix + "massFinal");
    controls->cgInitial = AddNumber(form, tr("Initial CG from nose [m]"), body.centerOfGravityInitial, bodyBox, objectPrefix + "cgInitial");
    controls->cgFinal = AddNumber(form, tr("Final CG from nose [m]"), body.centerOfGravityFinal, bodyBox, objectPrefix + "cgFinal");
    controls->inertiaInitial = AddNumber(form, tr("Initial pitch/yaw inertia [kg m²]"), body.pitchYawMomentOfInertiaInitial, bodyBox, objectPrefix + "inertiaInitial");
    controls->inertiaFinal = AddNumber(form, tr("Final pitch/yaw inertia [kg m²]"), body.pitchYawMomentOfInertiaFinal, bodyBox, objectPrefix + "inertiaFinal");
    controls->pitchDamping = AddNumber(form, tr("Pitch damping moment coefficient"), body.pitchDampingMomentCoefficient, bodyBox, objectPrefix + "pitchDamping");
    contentLayout->addWidget(bodyBox);

    auto* engineBox = new QGroupBox(tr("Engine"), content);
    auto* engineForm = new QFormLayout(engineBox);
    controls->thrustPath = new QLineEdit(PathText(body.engine.thrustFile, m_projectFile), engineBox);
    controls->thrustPath->setObjectName(objectPrefix + "thrustPath");
    auto* thrustRow = new QWidget(engineBox);
    auto* thrustLayout = new QHBoxLayout(thrustRow);
    thrustLayout->setContentsMargins(0, 0, 0, 0);
    thrustLayout->addWidget(controls->thrustPath, 1);
    auto* thrustBrowse = new QPushButton(tr("Browse…"), thrustRow);
    thrustLayout->addWidget(thrustBrowse);
    engineForm->addRow(tr("Thrust data file"), thrustRow);
    controls->thrustPressure = AddNumber(engineForm, tr("Measured thrust pressure [Pa]"), body.engine.thrustMeasuredPressure, engineBox, objectPrefix + "thrustPressure");
    controls->nozzleDiameter = AddNumber(engineForm, tr("Nozzle diameter [m]"), body.engine.nozzleDiameter, engineBox, objectPrefix + "nozzleDiameter");
    connect(thrustBrowse, &QPushButton::clicked, this, [this, input = controls->thrustPath, row = thrustRow] {
        BrowseReferencePath(input, m_projectFile, row, tr("Data files (*.txt *.csv);;All files (*)"));
    });
    contentLayout->addWidget(engineBox);

    auto* aeroBox = new QGroupBox(tr("Aerodynamics"), content);
    auto* aeroForm = new QFormLayout(aeroBox);
    controls->coefficientPath = new QLineEdit(PathText(body.aerodynamics.coefficientFile, m_projectFile), aeroBox);
    controls->coefficientPath->setObjectName(objectPrefix + "coefficientPath");
    auto* coefficientRow = new QWidget(aeroBox);
    auto* coefficientLayout = new QHBoxLayout(coefficientRow);
    coefficientLayout->setContentsMargins(0, 0, 0, 0);
    coefficientLayout->addWidget(controls->coefficientPath, 1);
    auto* coefficientBrowse = new QPushButton(tr("Browse…"), coefficientRow);
    coefficientLayout->addWidget(coefficientBrowse);
    aeroForm->addRow(tr("Aerodynamic coefficient CSV"), coefficientRow);
    controls->centerOfPressure = AddNumber(aeroForm, tr("Center of pressure [m]"), body.aerodynamics.centerOfPressure, aeroBox, objectPrefix + "centerOfPressure");
    controls->centerOfPressureAlpha = AddNumber(aeroForm, tr("Center of pressure alpha coefficient"), body.aerodynamics.centerOfPressureAlpha, aeroBox, objectPrefix + "centerOfPressureAlpha");
    controls->dragInitial = AddNumber(aeroForm, tr("Initial drag coefficient"), body.aerodynamics.dragCoefficientInitial, aeroBox, objectPrefix + "dragInitial");
    controls->dragFinal = AddNumber(aeroForm, tr("Final drag coefficient"), body.aerodynamics.dragCoefficientFinal, aeroBox, objectPrefix + "dragFinal");
    controls->dragAlphaSquared = AddNumber(aeroForm, tr("Alpha-squared drag coefficient"), body.aerodynamics.dragCoefficientAlphaSquared, aeroBox, objectPrefix + "dragAlphaSquared");
    controls->normalForce = AddNumber(aeroForm, tr("Normal force coefficient"), body.aerodynamics.normalForceCoefficient, aeroBox, objectPrefix + "normalForce");
    connect(coefficientBrowse, &QPushButton::clicked, this, [this, input = controls->coefficientPath, row = coefficientRow] {
        BrowseReferencePath(input, m_projectFile, row, tr("CSV files (*.csv);;All files (*)"));
    });
    contentLayout->addWidget(aeroBox);

    auto* parachuteBox = new QGroupBox(tr("Parachutes"), content);
    auto* parachuteLayout = new QVBoxLayout(parachuteBox);
    controls->parachutes = NewTable(parachuteBox,
                                    {tr("Open at launch time [s] (optional)"),
                                     tr("Open after peak [s] (optional)"),
                                     tr("Open at height [m] (optional)"),
                                     tr("CdS [m²]")});
    controls->parachutes->setObjectName(objectPrefix + "parachutes");
    for (const auto& parachute : body.parachutes) {
        const int row = controls->parachutes->rowCount();
        AddTableRow(controls->parachutes);
        SetOptionalTableNumber(controls->parachutes, row, 0, parachute.openingTimeFromLaunch);
        SetOptionalTableNumber(controls->parachutes, row, 1, parachute.openingTimeFromPeak);
        SetOptionalTableNumber(controls->parachutes, row, 2, parachute.openingHeight);
        SetTableNumber(controls->parachutes, row, 3, parachute.CdS);
    }
    parachuteLayout->addWidget(controls->parachutes);
    auto* parachuteButtons = new QHBoxLayout;
    auto* addParachute = new QPushButton(tr("Add parachute"), parachuteBox);
    addParachute->setObjectName(objectPrefix + "addParachuteButton");
    auto* removeParachute = new QPushButton(tr("Remove selected"), parachuteBox);
    parachuteButtons->addWidget(addParachute);
    parachuteButtons->addWidget(removeParachute);
    parachuteButtons->addStretch();
    parachuteLayout->addLayout(parachuteButtons);
    connect(addParachute, &QPushButton::clicked, this, [table = controls->parachutes] {
        const int row = table->rowCount();
        AddTableRow(table);
        for (int column = 0; column < 3; ++column) OptionalTableInput(table, row, column);
        SetTableNumber(table, row, 3, 0.0);
    });
    connect(removeParachute, &QPushButton::clicked, this, [table = controls->parachutes] { RemoveSelectedTableRow(table); });
    contentLayout->addWidget(parachuteBox);

    auto* transitionBox = new QGroupBox(tr("Mass and drag transitions"), content);
    auto* transitionLayout = new QVBoxLayout(transitionBox);
    controls->transitions = NewTable(transitionBox,
                                     {tr("Time [s]"), tr("Mass change [kg]"), tr("Drag change")});
    controls->transitions->setObjectName(objectPrefix + "transitions");
    for (const auto& transition : body.transitions) {
        const int row = controls->transitions->rowCount();
        AddTableRow(controls->transitions);
        SetTableNumber(controls->transitions, row, 0, transition.time);
        SetTableNumber(controls->transitions, row, 1, transition.mass);
        SetTableNumber(controls->transitions, row, 2, transition.Cd);
    }
    transitionLayout->addWidget(controls->transitions);
    auto* transitionButtons = new QHBoxLayout;
    auto* addTransition = new QPushButton(tr("Add transition"), transitionBox);
    auto* removeTransition = new QPushButton(tr("Remove selected"), transitionBox);
    transitionButtons->addWidget(addTransition);
    transitionButtons->addWidget(removeTransition);
    transitionButtons->addStretch();
    transitionLayout->addLayout(transitionButtons);
    connect(addTransition, &QPushButton::clicked, this, [table = controls->transitions] {
        const int row = table->rowCount();
        AddTableRow(table);
        SetTableNumber(table, row, 0, 0.0);
        SetTableNumber(table, row, 1, 0.0);
        SetTableNumber(table, row, 2, 0.0);
    });
    connect(removeTransition, &QPushButton::clicked, this, [table = controls->transitions] { RemoveSelectedTableRow(table); });
    contentLayout->addWidget(transitionBox);

    content->setLayout(contentLayout);
    scroll->setWidget(content);
    pageLayout->addWidget(scroll);
    const auto index = m_bodies.size();
    m_bodyTabs->addTab(controls->page, tr("Body %1").arg(index + 1));
    m_bodies.push_back(std::move(controls));
}

void ProjectEditor::removeBody() {
    const int index = m_bodyTabs->currentIndex();
    if (index < 0) return;
    if (m_bodies.size() == 1) {
        QMessageBox::information(this, tr("Keep one body"), tr("A project needs at least one body."));
        return;
    }
    const auto removedIndex = static_cast<size_t>(index);
    std::vector<project::Separation> remainingSeparations;
    try {
        for (int row = 0; row < m_separations->rowCount(); ++row) {
            bool sourceOk = false;
            const auto source = CellText(m_separations, row, 0).toULongLong(&sourceOk);
            if (!sourceOk || source == removedIndex) continue;
            auto products = ParseProducts(CellText(m_separations, row, 1), tr("Separation products"));
            products.erase(std::remove(products.begin(), products.end(), removedIndex), products.end());
            for (auto& product : products) {
                if (product > removedIndex) --product;
            }
            const size_t adjustedSource = static_cast<size_t>(source > removedIndex ? source - 1 : source);
            remainingSeparations.push_back({adjustedSource, std::move(products)});
        }
    } catch (const std::exception& error) {
        QMessageBox::warning(this, tr("Invalid separation"), QString::fromStdString(error.what()));
        return;
    }
    delete m_bodyTabs->widget(index);
    m_bodies.erase(m_bodies.begin() + index);
    for (int i = 0; i < m_bodyTabs->count(); ++i) m_bodyTabs->setTabText(i, tr("Body %1").arg(i + 1));
    m_separations->setRowCount(0);
    for (const auto& separation : remainingSeparations) {
        const int row = m_separations->rowCount();
        AddTableRow(m_separations);
        m_separations->item(row, 0)->setText(QString::number(static_cast<qulonglong>(separation.sourceBodyIndex)));
        m_separations->item(row, 1)->setText(ProductsText(separation.productBodyIndices));
    }
}

void ProjectEditor::setSingleTopology() {
    if (m_bodies.size() > 1) {
        const auto answer = QMessageBox::question(this,
                                                  tr("Use one body"),
                                                  tr("This removes the other body definitions and separations. Continue?"),
                                                  QMessageBox::Yes | QMessageBox::No,
                                                  QMessageBox::No);
        if (answer != QMessageBox::Yes) return;
        while (m_bodyTabs->count() > 1) {
            const int last = m_bodyTabs->count() - 1;
            delete m_bodyTabs->widget(last);
            m_bodies.pop_back();
        }
    }
    m_separations->setRowCount(0);
}

void ProjectEditor::setLegacyTripleTopology() {
    while (m_bodies.size() < 3) addBody();
    if (m_bodies.size() > 3) {
        const auto answer = QMessageBox::question(this,
                                                  tr("Use legacy three-body topology"),
                                                  tr("This removes body definitions after body 3. Continue?"),
                                                  QMessageBox::Yes | QMessageBox::No,
                                                  QMessageBox::No);
        if (answer != QMessageBox::Yes) return;
        while (m_bodyTabs->count() > 3) {
            const int last = m_bodyTabs->count() - 1;
            delete m_bodyTabs->widget(last);
            m_bodies.pop_back();
        }
    }
    m_separations->setRowCount(0);
    AddTableRow(m_separations);
    m_separations->item(0, 0)->setText(QStringLiteral("0"));
    m_separations->item(0, 1)->setText(QStringLiteral("1,2"));
}

void ProjectEditor::addSeparation() {
    const int row = m_separations->rowCount();
    AddTableRow(m_separations);
    m_separations->item(row, 0)->setText(QStringLiteral("0"));
    if (m_bodies.size() > 1) m_separations->item(row, 1)->setText(QStringLiteral("1"));
}

void ProjectEditor::removeSeparation() { RemoveSelectedTableRow(m_separations); }
