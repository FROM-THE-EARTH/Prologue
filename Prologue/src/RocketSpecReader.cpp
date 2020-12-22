#include "RocketSpecReader.h"

#include "JsonUtils.h"
#include "Constant.h"
#include "CommandLine.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <iostream>
#include <string>


constexpr int multipleRocketNum = 3;
constexpr const char* RocketParamList[multipleRocketNum] = { "rocket1", "rocket2", "rocket3" };


double calcParachuteCd(double massFinal, double terminalVelocity) {
	return massFinal * Constant::G / (0.5 * 1.25 * pow(terminalVelocity, 2) * 1.0);
}


namespace RocketSpecReader {

	bool existInfCd = false;
	bool multipleJudged = false;
	bool isMultiple = false;

	namespace Internal {

		template<typename T>
		T GetValueWithError(const boost::property_tree::ptree& pt, const std::string& key) {
			if (JsonUtils::HasValue<double>(pt, key)) {
				return JsonUtils::GetValue<T>(pt, key);
			}
			else {
				CommandLine::PrintInfo(PrintInfoType::Error, ("key \"" +key + "\" in json file is not valid.").c_str());
				exit(1);
			}
		}

		void ReadRocketParam(const boost::property_tree::ptree& pt, RocketSpec* spec, size_t index) {
			const std::string key = RocketParamList[index];

			spec->rocketParam.push_back({});

			spec->rocketParam[index].length = GetValueWithError<double>(pt, key + ".ref_len");
			spec->rocketParam[index].diameter = GetValueWithError<double>(pt, key + ".diam");
			spec->rocketParam[index].bottomArea = spec->rocketParam[index].diameter * spec->rocketParam[index].diameter * 0.25 * Constant::PI;

			spec->rocketParam[index].CGLengthInitial = GetValueWithError<double>(pt, key + ".CGlen_i");
			spec->rocketParam[index].CGLengthFinal = GetValueWithError<double>(pt, key + ".CGlen_f");

			spec->rocketParam[index].massInitial = GetValueWithError<double>(pt, key + ".mass_i");
			spec->rocketParam[index].massFinal = GetValueWithError<double>(pt, key + ".mass_f");

			spec->rocketParam[index].rollingMomentInertiaInitial = GetValueWithError<double>(pt, key + ".Iyz_i");
			spec->rocketParam[index].rollingMomentInertiaFinal = GetValueWithError<double>(pt, key + ".Iyz_f");

			spec->rocketParam[index].Cmq = GetValueWithError<double>(pt, key + ".Cmq");

			spec->rocketParam[index].parachute.push_back(Parachute());
			spec->rocketParam[index].parachute[0].terminalVelocity = JsonUtils::GetValue<double>(pt, key + ".vel_1st");
			spec->rocketParam[index].parachute[0].openingType = JsonUtils::GetValue<int>(pt, key + ".op_type_1st");
			spec->rocketParam[index].parachute[0].openingTime = JsonUtils::GetValue<double>(pt, key + ".op_time_1st");
			spec->rocketParam[index].parachute[0].delayTime = JsonUtils::GetValue<double>(pt, key + ".delay_time_1st");

			if (spec->rocketParam[index].parachute[0].terminalVelocity == 0.0) {
				existInfCd = true;
				CommandLine::PrintInfo(PrintInfoType::Warning,
					(std::string("Rocket: ") + key).c_str(),
					"Terminal velocity is undefined.",
					"Parachute Cd value is automatically calculated.");
			}
			else {
				spec->rocketParam[index].parachute[0].Cd =
					calcParachuteCd(
						spec->rocketParam[index].massFinal,
						spec->rocketParam[index].parachute[0].terminalVelocity
					);
			}

			spec->rocketParam[index].engine.loadThrustData(JsonUtils::GetValue<std::string>(pt, key + ".motor_file"));
			spec->rocketParam[index].airspeedParam.loadParam(JsonUtils::GetValue<std::string>(pt, key + ".airspeed_param_file"));
			if (!spec->rocketParam[index].airspeedParam.exist()) {
				CommandLine::PrintInfo(PrintInfoType::Information,
					("Rocket: " + key).c_str(),
					"Airspeed param is set from JSON");
				spec->rocketParam[index].airspeedParam.setParam(
					GetValueWithError<double>(pt, key + ".CPlen"),
					JsonUtils::GetValue<double>(pt, key + ".CP_alpha"),
					GetValueWithError<double>(pt, key + ".Cd"),
					JsonUtils::GetValue<double>(pt, key + ".Cd_alpha2"),
					GetValueWithError<double>(pt, key + ".Cna")
				);
			}
			else {
				CommandLine::PrintInfo(PrintInfoType::Information,
					("Rocket: " + key).c_str(),
					"Airspeed param is set from CSV");
			}
		}


		void ReadExtraInfo(const boost::property_tree::ptree& pt, RocketSpec* spec) {
			spec->info.teamName = JsonUtils::GetValue<std::string>(pt, "info.TEAM");
			spec->info.rocketName = JsonUtils::GetValue<std::string>(pt, "info.NAME");
			spec->info.experimentDate = JsonUtils::GetValue<std::string>(pt, "info.DATE");
			spec->info.version = JsonUtils::GetValue<std::string>(pt, "info.VERSION");
		}


		void ReadEnvironment(const boost::property_tree::ptree& pt, RocketSpec* spec) {
			spec->env.place = JsonUtils::GetValue<std::string>(pt, "environment.place");
			spec->env.railLength = GetValueWithError<double>(pt, "environment.rail_len");
			spec->env.railAzimuth = GetValueWithError<double>(pt, "environment.rail_azi");
			spec->env.railElevation = GetValueWithError<double>(pt, "environment.rail_elev");
		}

		void SetInfParachuteCd(RocketSpec* spec) {
			for (size_t i = 0; i < spec->rocketParam.size(); i++) {
				if (spec->rocketParam[i].parachute[0].Cd == 0) {
					for (int j = (int)spec->rocketParam.size() - 1; j >= 0; j--) {
						spec->rocketParam[i].parachute[0].Cd += spec->rocketParam[j].parachute[0].Cd;
					}
				}
			}
		}
	}


	RocketSpec ReadJson(const std::string& filename) {
		RocketSpec spec;
		boost::property_tree::ptree pt;
		boost::property_tree::read_json("input/json/" + filename, pt);

		existInfCd = false;

		/*Read once*/
		Internal::ReadExtraInfo(pt, &spec);

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

		isMultiple = JsonUtils::Exist(pt, RocketParamList[1]);

		multipleJudged = true;

		return isMultiple;
	}
}