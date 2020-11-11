#pragma once
#pragma once
#include <string>
#include <vector>


struct JsonInformation {
	std::string teamName;
	std::string rocketName;
	std::string experimentDate;
	std::string version;
};

struct Parachute {
	double terminalVelocity;
	int openingType;
	double openingTime;
	double delayTime;
	double openingHeight;

	double Cd = 0;
};

struct RocketParam {
	double length;//m
	double diameter;//m
	double bottomArea;//m^2

	double CGLengthInitial;//m
	double CGLengthFinal;//m

	double massInitial;//kg
	double massFinal;//kg

	double rollingMomentInertiaInitial;//kg*m^2
	double rollingMomentInertiaFinal;//kg*m^2

	double CPLength;//m

	double Cd;

	double Cna;

	double Cmq;

	std::vector<Parachute> parachute;
};

struct Environment {
	std::string place;
	double railLength;
	double railAzimuth;
	double railElevation;
};


struct SpecJson {
	JsonInformation info;
	std::vector <RocketParam> rocketParam;//could be multiple(multiple rocket)
	Environment env;
	std::vector<std::string> enginesFilename = std::vector<std::string>(2);
};


/*
{
"info":{
	"TEAM":		"Team name",
	"NAME":		"Rocket name",
	"DATE":		"Experiment date",
	"VERSION":	"Submission version"
	},

"rocket":{
	"ref_len":	"Rocket length[m]",
	"diam":		"Rocket diameter[m]",

	"CGlen_i":	"Initial CG place from nose[m] �d�S",
	"CGlen_f":	"Final CG place from nose[m]�@�d�S",

	"mass_i":	"Initial mass[kg] ��������+�_���܎��ʁv",
	"mass_f":	"Final mass[kg] ��������",

	"Iyz_i":	"Initial inertia moment of rolling[kg*m^2]�@�s�b�`/���[",
	"Iyz_f":	"Final inertia moment of rolling[kg*m^2] �s�b�`/���[",

	"CPlen":	"CP place from nose[m] ���͒��S",

	"Cd":		"Drag coefficient �R�͌W��",
	"Cna":		"Normal force coefficient �@���͐���",
	"Cmq":		"Pitch damping moment coefficient �s�b�`�������[�����g�W��",

	"vel_1st":	"Terminal velocity of 1st parachute",
	"op_type_1st":	"0:detect-peak, 1:fixed-time",
	"op_time_1st": "open time",
	"delay_time_1st": "�p���V���[�g���J���̂ɂ����鎞��",

	"vel_2nd":	"Terminal velocity of 1st parachute",
	"op_type_2nd":	"0:following, 1:fixed-time",
	"op_time_2nd": "open time",
	"op_height_2nd": "�p���V���[�g���J������",

	"vel_3rd":	"Terminal velocity of 1st parachute",
	"op_type_3rd":	"0:following, 1:fixed-time",
	"op_height_3rd": "�p���V���[�g���J������",

	"op_time_3rd": "open time",
	"op_time_4th": "open time",
	"op_height_4th": "�p���V���[�g���J������"
	},

"motor":{
	"motor_file":	"Hypertek_J250.txt",
	"motor_2nd": "",
	"iject_time_2nd": "",
	"iject_heught_2nd": ""
	}
}

*/