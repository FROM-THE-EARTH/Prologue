#include "ResultSaver.h"

#include "Solver.h"
#include "CommandLine.h"

#include <fstream>
#include <sstream>
#include <iomanip>


namespace ResultSaver {

	std::ofstream f;
	const char comma = ',';
	const std::string headers[] = {
		"wind_speed",
		"wind_dir",
		"launch_vel",
		"max_height",
		"max_height_time",
		"max_vel",
		"time_paraopen",
		"height_paraopen",
		"airspeed_paraopen",
		"vel_terminal",
		"max_attack_angle",
		"max_normal_force",
		"len_from_launch",
		"latitude, longitude"
	};
	const int headerNum = sizeof(headers) / sizeof(*headers);
	const int dataStartingRow = 6;

	namespace Internal {

		std::string addComma(int str) {
			return std::to_string(str) + comma;
		}

		std::string addComma(double str, int precision = 2) {
			std::stringstream stream;
			stream << std::fixed << std::setprecision(precision) << str;
			const std::string s = stream.str() + comma;
			return s;
		}

		std::string addComma(const std::string str) {
			return str + comma;
		}
	}

	void OpenFile(const char* filepath, size_t rows) {

		f.open(filepath);

		if (!f) {
			const std::string s = "Failed to open : " + std::string(filepath);
			CommandLine::PrintInfo(PrintInfoType::Error, s.c_str());
			return;
		}

		//unit description
		f << "length=[m] | velocity=[m/s] | angle=[deg]\n";

		//header
		f << comma;
		for (auto& h : headers) {
			f << h << comma;
		}
		f << '\n';

		//min
		f << "min" << comma;
		char column = 'B';
		for (size_t i = 0; i < headerNum; i++) {
			char range[16];
			sprintf_s(range, 16, "%c%d:%c%d", column, dataStartingRow, column, static_cast<int>(rows + dataStartingRow - 1));
			char min[32] = "";
			sprintf_s(min, 32, "=MIN(%s)", range);

			column++;
			f << min << comma;
		}
		f << '\n';

		//max
		f << "max" << comma;
		column = 'B';
		for (size_t i = 0; i < headerNum; i++) {
			char range[16];
			sprintf_s(range, 16, "%c%d:%c%d", column, dataStartingRow, column, static_cast<int>(rows + dataStartingRow - 1));
			char max[32] = "";
			sprintf_s(max, 32, "=MAX(%s)", range);

			column++;
			f << max << comma;
		}
		f << '\n';

		//empty row
		f << '\n';
	}

	void WriteLine(const SolvedResult& result, size_t rocketIndex) {

		const std::string latitude_longitude = Internal::addComma(result.rocket[rocketIndex].latitude, 8) +"N, "
			+ Internal::addComma(result.rocket[rocketIndex].longitude, 8)+"E";

			f <<
			comma <<
			Internal::addComma(result.windSpeed) <<
			Internal::addComma(result.windDirection) <<
			Internal::addComma(result.launchClearVelocity_) <<
			Internal::addComma(result.rocket[rocketIndex].maxHeight) <<
			Internal::addComma(result.rocket[rocketIndex].detectPeakTime) <<
			Internal::addComma(result.rocket[rocketIndex].maxVelocity) <<
			Internal::addComma(result.rocket[rocketIndex].timeAtParaOpened) <<
			Internal::addComma(result.rocket[rocketIndex].heightAtParaOpened) <<
			Internal::addComma(result.rocket[rocketIndex].airVelAtParaOpened) <<
			Internal::addComma(result.rocket[rocketIndex].terminalVelocity) <<
			Internal::addComma(result.rocket[rocketIndex].maxAttackAngle) <<
			Internal::addComma(result.rocket[rocketIndex].maxNormalForce) <<
			Internal::addComma(result.rocket[rocketIndex].lenFromLaunchPoint) <<
			latitude_longitude;
		f << '\n';

	}

	void Close() {
		f.close();
	}
}