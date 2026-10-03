#include <exception>

#include <QApplication>
#include <QCommandLineParser>
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>

#include "gui/MainWindow.hpp"

namespace {
    QString SampleRoot() {
        QDir current(QCoreApplication::applicationDirPath());
        if (QFileInfo::exists(current.filePath("sample/prologue.settings.json"))) return current.filePath("sample");
        for (int level = 0; level < 6; ++level) {
            if (QFileInfo::exists(current.filePath("application/prologue.settings.json"))) return current.filePath("application");
            if (!current.cdUp()) break;
        }
        return QDir::current().filePath("application");
    }
}

int main(int argc, char* argv[]) {
    QApplication application(argc, argv);
    application.setApplicationName("Prologue GUI Demo");
    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addOption({"project", "Open a version-2 project JSON.", "file"});
    parser.addOption({"settings", "Open CLI application settings.", "file"});
    parser.process(application);
    gui::MainWindow window;
    try {
        const QDir sample(SampleRoot());
        window.loadProject(parser.isSet("project") ? parser.value("project") : sample.filePath("input/spec/spec_single.json"));
        window.loadSettings(parser.isSet("settings") ? parser.value("settings") : sample.filePath("prologue.settings.json"));
    } catch (const std::exception& error) {
        QMessageBox::warning(&window, "Prologue", QString::fromUtf8(error.what()));
    }
    window.show();
    return application.exec();
}
