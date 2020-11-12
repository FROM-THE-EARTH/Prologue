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
		"latitude longitude"
	};
	const int headerNum = sizeof(headers) / sizeof(*headers);
	const int dataStartingRow = 6;

	namespace Internal {

		std::string DoubleToString(double d, int precision) {
			std::stringstream stream;
			stream << std::fixed << std::setprecision(precision) << d;
			return stream.str();
		}

		std::string AddComma(int i) {
			return std::to_string(i) + comma;
		}

		std::string AddComma(double d, int precision = 2) {
			return DoubleToString(d, precision) + comma;
		}

		std::string AddComma(const std::string str) {
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

		const std::string latitude_longitude = Internal::DoubleToString(result.rocket[rocketIndex].latitude, 8) +"N "
			+ Internal::DoubleToString(result.rocket[rocketIndex].longitude, 8)+"E";

			f <<
			comma <<
			Internal::AddComma(result.windSpeed) <<
			Internal::AddComma(result.windDirection) <<
			Internal::AddComma(result.launchClearVelocity_) <<
			Internal::AddComma(result.rocket[rocketIndex].maxHeight) <<
			Internal::AddComma(result.rocket[rocketIndex].detectPeakTime) <<
			Internal::AddComma(result.rocket[rocketIndex].maxVelocity) <<
			Internal::AddComma(result.rocket[rocketIndex].timeAtParaOpened) <<
			Internal::AddComma(result.rocket[rocketIndex].heightAtParaOpened) <<
			Internal::AddComma(result.rocket[rocketIndex].airVelAtParaOpened) <<
			Internal::AddComma(result.rocket[rocketIndex].terminalVelocity) <<
			Internal::AddComma(result.rocket[rocketIndex].maxAttackAngle) <<
			Internal::AddComma(result.rocket[rocketIndex].maxNormalForce) <<
			Internal::AddComma(result.rocket[rocketIndex].lenFromLaunchPoint) <<
			latitude_longitude;
		f << '\n';

	}

	void Close() {
		f.close();
	}
}