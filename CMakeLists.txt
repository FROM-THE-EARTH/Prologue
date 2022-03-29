cmake_minimum_required (VERSION 3.1)

enable_language(CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (MSVC)
	set(CMAKE_CXX_FLAGS "/W4 /EHsc")
else()
	set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wno-unknown-pragmas -pthread")
endif()

project(Prologue)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../application)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ../application)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ../application)

include_directories(
	SYSTEM "deps/include/"
	"src/"
)

set(SOURCE_FILES
	"src/app/AppSetting.cpp"
	"src/app/CommandLine.cpp"

	"src/dynamics/WindModel.cpp"

	"src/env/Environment.cpp"
	"src/env/Map.cpp"

	"src/gnuplot/Gnuplot.cpp"

	"src/math/Algorithm.cpp"
	"src/math/Quaternion.cpp"
	"src/math/Vector3D.cpp"

	"src/rocket/AirspeedParam.cpp"
	"src/rocket/Engine.cpp"
	"src/rocket/RocketSpec.cpp"

	"src/solver/Simulator.cpp"
	"src/solver/Solver.cpp"

	"src/utils/JsonUtils.cpp"
	"src/utils/ResultSaver.cpp"

	"src/main.cpp"
)

add_executable(Prologue ${SOURCE_FILES})