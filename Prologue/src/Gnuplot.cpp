#include "Gnuplot.h"

#include "Solver.h"
#include "MapFeature.h"

#include <string>
#include <fstream>
#include <vector>
#include <filesystem>


enum class PlotDimension{
	Dimension2D,
	Dimension3D
};

struct GnuplotRange{
	double xMin, xMax;
	double yMin, yMax;
};


namespace Gnuplot {

	FILE* p;

	std::string dirname;
	std::string command;
	Map map = Map::NOSIRO_SEA;
	PlotDimension dimension;
	GnuplotRange range;
	size_t rocketCount = 0;
	size_t plotCount = 0;//plot group
	double windSpeed = 0.0, windDirection = 0.0;

	namespace Internal {

		void Initialize() {
			//reset
			fprintf(p, "reset\n");

			//Ticslevel
			fprintf(p, "set ticslevel 0\n");

			//grid
			fprintf(p, "set grid\n");

			//legends box
			fprintf(p, "set key opaque box\n");

			//Label
			fprintf(p, "set xlabel \"Distance[m](+: East, -: West)\"\n");
			fprintf(p, "set ylabel \"Distance[m](+: North, -: South)\"\n");
			fprintf(p, "set zlabel \"Height[m]\"\n");

			//title, xyz ratio, range
			switch (dimension)
			{
			case PlotDimension::Dimension2D:
				fprintf(p, "set size ratio -1\n");
				fprintf(p, "set xrange[%f:%f]\n", range.xMin, range.xMax);
				fprintf(p, "set yrange[%f:%f]\n", range.yMin, range.yMax);
				command = "plot ";
				break;

			case PlotDimension::Dimension3D:
				fprintf(p, "set title \"Wind: %.2f[m/s] %.2f[deg]\"\n", windSpeed, windDirection);
				fprintf(p, "set view equal xyz\n");
				command = "splot ";
				break;
			}
			fprintf(p, "set title font \"MS Gothic, 20\"\n");
		}

		void PlotLaunchPoint() {
			std::ofstream file("result/" + dirname + "/data/launch.txt");
			file << 0 << " " << 0 << " " << 0 << std::endl;
			file.close();
		}

		void Show2D() {
			command += "\"data/launch.txt\" ";
			command += "title \"LaunchPoint\" ";
			command += "with ";
			command += "points ";
			command += "pt 7 ps 2";
			command += "lc rgb \"yellow\"";
			command += ", ";

			for (size_t i = 0; i < plotCount; i++) {
				command += "\"data/result" + std::to_string(i) + ".txt\" ";
				size_t k = i / rocketCount;
				if (k == 0) {
					command += "title \"Rocket No." + std::to_string(i % rocketCount + 1) + "\" ";
				}
				else {
					command += "notitle ";
				}
				command += "with ";
				command += "lines ";
				command += "lw 2 ";

				if (i % rocketCount == 0) {
					command += "lc rgb \"red\"";
				}else{
					command += "lc rgb \"blue\"";
				}

				if (i != plotCount - 1)
					command += ", ";
			}

			command += "\n";
			fprintf(p, command.c_str());
		}

		void Show3D() {
			command += "\"data/launch.txt\" ";
			command += "title \"LaunchPoint\" ";
			command += "with ";
			command += "points ";
			command += "pt 7 ps 2";
			command += "lc rgb \"yellow\"";
			command += ", ";

			for (size_t i = 0; i < plotCount; i++) {
				command += "\"data/result" + std::to_string(i) + ".txt\" ";
				command += "title \"Rocket No." + std::to_string(i + 1) + "\" ";
				command += "with ";
				command += "lines ";
				command += "lw 2";

				if (i != plotCount - 1)
					command += ", ";
			}

			command += "\n";
			fprintf(p, command.c_str());
		}

		void CalcRange(const SolvedResult& result, bool init, bool end) {
			if (init) {
				range = {
					result.rocket[0].flightData[0].pos.x,
					result.rocket[0].flightData[0].pos.x,
					result.rocket[0].flightData[0].pos.y,
					result.rocket[0].flightData[0].pos.y
				};
			}

			for (const auto& r : result.rocket) {
				for (const auto& f : r.flightData) {
					if (range.xMin > f.pos.x)range.xMin = f.pos.x;
					if (range.xMax < f.pos.x)range.xMax = f.pos.x;
					if (range.yMin > f.pos.y)range.yMin = f.pos.y;
					if (range.yMax < f.pos.y)range.yMax = f.pos.y;
				}
			}

			if (end) {
				const double deltaX = (range.xMax - range.xMin) * 0.2;
				const double deltaY = (range.yMax - range.yMin) * 0.2;

				range.xMin -= deltaX;
				range.xMax += deltaX;
				range.yMin -= deltaY;
				range.yMax += deltaY;
			}
		}

		void SetMap() {
			if (dimension == PlotDimension::Dimension2D) {
				command += "\"../../input/map/";

				switch (map)
				{
				case Map::IZU_LAND:
					command += "izu_land.png\" ";
					command += "binary filetype=png dx=1.00 dy=1.00 origin=(-730, -840) with rgbimage notitle,";
					break;

				case Map::IZU_SEA:
					command += "izu_sea.png\" ";
					command += "binary filetype=png dx=5.77 dy=5.77 origin=(-2420,-5650) with rgbimage notitle,";
					break;

				case Map::NOSIRO_LAND:
					command += "nosiro_land.png\" ";
					command += "binary filetype=png dx=0.59 dy=0.59 origin=(-421,-676) with rgbimage notitle,";
					break;

				case Map::NOSIRO_SEA:
					command += "nosiro_sea.png\" ";
					command += "binary filetype=png dx=7.0 dy=7.0 origin=(-8700,-3650) with rgbimage notitle,";
					break;

				case Map::UNKNOWN:
					std::cout << "<!----THIS MAP IS UNAVAILABLE----!>" << std::endl;
					command += "unknown.png\" ";
					break;
				}
			}
		}


		void SaveAsPNG() {
			fprintf(p, "set terminal pngcairo size 1920, 1440\n");

			fprintf(p, "set output \"result.png\"\n");

			fprintf(p, "load \"result.plt\"\n");

			fprintf(p, "set terminal windows\n");

			fprintf(p, "set output\n");
		}
	}


	void Initialize(const char* _dirname, Map m) {
		map = m;
		dirname = _dirname;

		const std::filesystem::path dir = "result/" + dirname + "/data";
		if (!std::filesystem::exists(dir)) {
			std::filesystem::create_directory(dir);
		}
	}


	void Plot(const SolvedResult& result) {
		Internal::PlotLaunchPoint();

		plotCount = result.rocket.size();

		for (size_t i = 0; i < plotCount; i++) {
			const std::string fname = "result/" + dirname + "/data/result" + std::to_string(i) + ".txt";
			std::ofstream file(fname.c_str());
			for (const auto& f : result.rocket[i].flightData) {
				file << f.pos.x << " " << f.pos.y << " " << f.pos.z << std::endl;
			}
			file.close();
		}

		windSpeed = result.windSpeed;
		windDirection = result.windDirection;

		dimension = PlotDimension::Dimension3D;
	}

	
	void Plot(const std::vector<SolvedResult>& result) {
		Internal::PlotLaunchPoint();

		rocketCount = result[0].rocket.size();

		for (size_t i = 0; i < result.size(); i++) {
			if (result[i].windDirection == 0.0) {
				plotCount++;
			}
			Internal::CalcRange(result[i], i == 0, i == result.size() - 1);
		}

		const size_t directions = result.size() / plotCount;
		const size_t winds = plotCount;
		plotCount *= rocketCount;

		for (size_t i = 0; i < winds; i++) {//winds
			for (size_t j = 0; j < rocketCount; j++) {//rockets
				const std::string fname = "result/"+ dirname +"/data/result" + std::to_string(i * rocketCount + j) + ".txt";
				std::ofstream file(fname.c_str());

				for (size_t k = 0; k < directions; k++) {//directions
					const size_t n = directions * i + k;
					const Vector3D pos = result[n].rocket[j].flightData[0].pos;
					file << pos.x << " " << pos.y << std::endl;
				}

				//add initial point to be circle
				const size_t ini = directions * i;
				file << result[ini].rocket[j].flightData[0].pos.x << " " << result[ini].rocket[j].flightData[0].pos.y << std::endl;

				file.close();
			}
		}

		dimension = PlotDimension::Dimension2D;
	}

	void Show() {
		if (std::filesystem::exists("result.plt")) {
			p = _popen("gnuplot", "w");
			fprintf(p, "load \"result.plt\"\n");
		}
		else {
			p = _popen("gnuplot", "w");
			if (p == nullptr) {
				std::cout << "Could not open gnuplot" << std::endl;
				return;
			}

			Internal::Initialize();

			fprintf(p, "cd \"result/%s\"\n", dirname.c_str());

			switch (dimension)
			{
			case PlotDimension::Dimension2D:
				Internal::SetMap();
				Internal::Show2D();
				break;

			case PlotDimension::Dimension3D:
				Internal::Show3D();
				break;
			}
		}

		fflush(p);

		system("pause");

		fprintf(p, "exit\n");

		_pclose(p);
	}

	void Save() {
		p = _popen("gnuplot", "w");
		if (p == nullptr) {
			std::cout << "Could not open gnuplot" << std::endl;
			return;
		}

		Internal::Initialize();

		fprintf(p, "cd \"result/%s\"\n", dirname.c_str());

		switch (dimension)
		{
		case PlotDimension::Dimension2D:
			Internal::SetMap();
			Internal::Show2D();
			break;

		case PlotDimension::Dimension3D:
			Internal::Show3D();
			break;
		}

		fflush(p);

		fprintf(p, "save \"result.plt\"\n");

		fprintf(p, "replot\n");

		fprintf(p, "set terminal windows\n");

		fflush(p);

		if (dimension == PlotDimension::Dimension2D) {
			Internal::SaveAsPNG();
		}

		fprintf(p, "exit\n");

		_pclose(p);
	}
}