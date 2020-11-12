#include "CommandLine.h"

#include "Gnuplot.h"

#include <filesystem>

struct Commands {
	const char* command;
	const char* description;
	void(*operation)();
};


namespace CommandLine {

	namespace Internal {

		size_t Counter = 1;

		std::string OutputDir;

		bool Exit = false;

		void ShowChoices() {}

		void PrintInfoLines() {}

		void ShowHelp();

		void OpenExplorer() {
			const std::filesystem::path p = std::filesystem::current_path();
			const std::string c = "explorer " + p.string() + "\\result\\" + OutputDir;
			system(c.c_str());
		}

		void ExitApplication() {
			Exit = true;
		}
	}

	const Commands commands[] = {
		{"show", "Show result in Gnuplot", Gnuplot::Show},
		{"open", "Open result folder in explorer", Internal::OpenExplorer},
		{"help", "Show commands", Internal::ShowHelp},
		{"exit", "Exit application", Internal::ExitApplication}
	};


	namespace Internal {
		void ShowHelp() {
			for (auto& c : commands) {
				std::cout << "\t" << c.command << ": " << c.description << std::endl;
			}
		}
	}


	void Run() {
		std::cout << "Input the command" << std::endl;
		Internal::ShowHelp();

		std::string cmd;
		bool unknown = true;

		while (!Internal::Exit) {
			unknown = true;

			std::cout << ">";
			std::cin >> cmd;
			for (auto& c : commands) {
				if (cmd == c.command) {
					unknown = false;
					c.operation();
				}
			}

			if (unknown) {
				Internal::ShowHelp();
			}
		}
	}


	void SetOutputDir(const std::string dirname) {
		Internal::OutputDir = dirname;
	}
}