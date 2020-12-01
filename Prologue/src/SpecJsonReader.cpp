#include "SpecJsonReader.h"

#include "JsonUtils.h"
#include "Constant.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <iostream>
#include <string>


constexpr int multipleRocketNum = 3;
constexpr const char* RocketParamList[multipleRocketNum] = { "rocket1", "rocket2", "rocket3" };


double calcParachuteCd(double massFinal, double terminalVelocity) {
	return massFinal * Constant::G / (0.5 * 1.25 * pow(terminalVelocity, 2) * 1.0);
}


namespace SpecJsonReader {

	bool existInfCd = false;
	bool multipleJudged = false;
	bool isMultiple = false;

	namespace Internal {

		void ReadRocketParam(const boost::property_tree::ptree& pt, SpecJson* spec, size_t index) {
			const std::string key = RocketParamList[index];

			spec->rocketParam.push_back({});

			spec->rocketParam[index].length = JsonUtils::GetValue<double>(pt, key + ".ref_len");
			spec->rocketParam[index].diameter = JsonUtils::GetValue<double>(pt, key + ".diam");
			spec->rocketParam[index].bottomArea = spec->rocketParam[index].diameter * spec->rocketParam[index].diameter * 0.25 * Constant::PI;

			spec->rocketParam[index].CGLengthInitial = JsonUtils::GetValue<double>(pt, key + ".CGlen_i");
			spec->rocketParam[index].CGLengthFinal = JsonUtils::GetValue<double>(pt, key + ".CGlen_f");

			spec->rocketParam[index].massInitial = JsonUtils::GetValue<double>(pt, key + ".mass_i");
			spec->rocketParam[index].massFinal = JsonUtils::GetValue<double>(pt, key + ".mass_f");

			spec->rocketParam[index].rollingMomentInertiaInitial = JsonUtils::GetValue<double>(pt, key + ".Iyz_i");
			spec->rocketParam[index].rollingMomentInertiaFinal = JsonUtils::GetValue<double>(pt, key + ".Iyz_f");

			spec->rocketParam[index].CPLength = JsonUtils::GetValue<double>(pt, key + ".CPlen");
			spec->rocketParam[index].Cd = JsonUtils::GetValue<double>(pt, key + ".Cd");
			spec->rocketParam[index].Cna = JsonUtils::GetValue<double>(pt, key + ".Cna");
			spec->rocketParam[index].Cmq = JsonUtils::GetValue<double>(pt, key + ".Cmq");

			spec->rocketParam[index].parachute.push_back(Parachute());
			spec->rocketParam[index].parachute[0].terminalVelocity = JsonUtils::GetValue<double>(pt, key + ".vel_1st");
			spec->rocketParam[index].parachute[0].openingType = JsonUtils::GetValue<int>(pt, key + ".op_type_1st");
			spec->rocketParam[index].parachute[0].openingTime = JsonUtils::GetValue<double>(pt, key + ".op_time_1st");
			spec->rocketParam[index].parachute[0].delayTime = JsonUtils::GetValue<double>(pt, key + ".delay_time_1st");

			if (spec->rocketParam[index].parachute[0].terminalVelocity == 0.0) {
				existInfCd = true;
				std::cout << "W: In rocket json file, key: " + key << std::endl;
				std::cout << "   Terminal velocity is undefined." << std::endl;
				std::cout << "   Parachute Cd value is automatically calculated." << std::endl;
				std::cout << std::endl;
			}
			else {
				spec->rocketParam[index].parachute[0].Cd =
					calcParachuteCd(
						spec->rocketParam[index].massFinal,
						spec->rocketParam[index].parachute[0].terminalVelocity
					);
			}
		}


		void ReadMotorInfo(const boost::property_tree::ptree& pt, SpecJson* spec) {
			spec->enginesFilename[0] = JsonUtils::GetValue<std::string>(pt, "motor1.motor_file");
			spec->enginesFilename[1] = JsonUtils::GetValue<std::string>(pt, "motor2.motor_file");
		}


		void ReadExtraInfo(const boost::property_tree::ptree& pt, SpecJson* spec) {
			spec->info.teamName = JsonUtils::GetValue<std::string>(pt, "info.TEAM");
			spec->info.rocketName = JsonUtils::GetValue<std::string>(pt, "info.NAME");
			spec->info.experimentDate = JsonUtils::GetValue<std::string>(pt, "info.DATE");
			spec->info.version = JsonUtils::GetValue<std::string>(pt, "info.VERSION");
		}


		void ReadEnvironment(const boost::property_tree::ptree& pt, SpecJson* spec) {
			spec->env.place = JsonUtils::GetValue<std::string>(pt, "environment.place");
			spec->env.railLength = JsonUtils::GetValue<double>(pt, "environment.rail_len");
			spec->env.railAzimuth = JsonUtils::GetValue<double>(pt, "environment.rail_azi");
			spec->env.railElevation = JsonUtils::GetValue<double>(pt, "environment.rail_elev");
		}

		void SetInfParachuteCd(SpecJson* spec) {
			for (size_t i = 0; i < spec->rocketParam.size(); i++) {
				if (spec->rocketParam[i].parachute[0].Cd == 0) {
					for (int j = (int)spec->rocketParam.size() - 1; j >= 0; j--) {
						spec->rocketParam[i].parachute[0].Cd += spec->rocketParam[j].parachute[0].Cd;
					}
				}
			}
		}
	}


	SpecJson ReadJson(const std::string& filename) {
		SpecJson spec;
		boost::property_tree::ptree pt;
		boost::property_tree::read_json("input/json/" + filename, pt);

		existInfCd = false;

		/*Read once*/
		Internal::ReadExtraInfo(pt, &spec);

		Internal::ReadMotorInfo(pt, &spec);

		Internal::ReadEnvironment(pt, &spec);

		/*Read once or more*/
		size_t i = 0;
		do{
			Internal::ReadRocketParam(pt, &spec, i);
			i++;
		} while (IsMultipleRocket(filename) && i < multipleRocketNum);

		//Set parachute Cd (Multiple rocket)
		if (existInfCd) {
			Internal::SetInfParachuteCd(&spec);
		}

		return spec;
	}


	bool IsMultipleRocket(const std::string& filename) {
		if (multipleJudged) {
			return isMultiple;
		}

		boost::property_tree::ptree pt;
		boost::property_tree::read_json("input/json/" + filename, pt);

		isMultiple = JsonUtils::GetValue<std::string>(pt, "motor2.motor_file") != "";

		multipleJudged = true;

		return isMultiple;
	}
}