#pragma once
#include <iostream>


enum class PrintInfoType {
	Information,
	Warning,
	Error
};


namespace CommandLine {

	namespace Internal {

		extern size_t counter;

		void ShowChoices();

		void PrintInfoLines();

		template<class... Args>
		void ShowChoices(const char* s, Args... choices) {

			std::cout << counter << ": " << s << std::endl;

			counter++;

			ShowChoices(std::forward<Args>(choices)...);
		}

		template<class... Args>
		void PrintInfoLines(const char* line, Args... lines) {

			std::cout << "   " << line << std::endl;

			PrintInfoLines(std::forward<Args>(lines)...);
		}

		template<class... Args>
		void PrintInfoFirstLine(PrintInfoType type, const char* line, Args... lines) {

			switch (type)
			{
			case PrintInfoType::Information:
				std::cout << "I: ";
				break;

			case PrintInfoType::Warning:
				std::cout << "W: ";
				break;

			case PrintInfoType::Error:
				std::cout << "E: ";
				break;
			}

			std::cout << line << std::endl;

			PrintInfoLines(std::forward<Args>(lines)...);
		}
	}

	template<class... Args>
	void Question(const char* question, Args... choices) {

		std::cout << "<!===" << question << "===!>" << std::endl;

		Internal::counter = 1;

		Internal::ShowChoices(std::forward<Args>(choices)...);
	}

	template<typename Type>
	Type InputIndex(size_t size) {

		int var = -1;

		std::cin >> var;

		while (var <= 0 || var > size) {
			std::cin >> var;
		}

		std::cout << std::endl;

		return static_cast<Type>(var);
	}

	template<class... Args>
	void PrintInfo(PrintInfoType type, Args... lines) {
		Internal::PrintInfoFirstLine(type, std::forward<Args>(lines)...);
		std::cout << std::endl;
	}
}