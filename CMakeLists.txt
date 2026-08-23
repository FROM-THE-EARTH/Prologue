cmake_minimum_required (VERSION 3.16.3)

if (APPLE)
	set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
endif()

enable_language(CXX)
enable_testing()

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

set(PROLOGUE_CORE_SOURCE_FILES
	"src/core/Simulation.cpp"
	"src/core/SimulationInput.cpp"
	"src/core/SimulationSettings.cpp"
	"src/dynamics/StandardAtmosphere1976.cpp"
	"src/dynamics/WindModel.cpp"
	"src/dynamics/WindProfile.cpp"
	"src/env/Environment.cpp"
	"src/math/Interpolation.cpp"
	"src/math/Quaternion.cpp"
	"src/math/Vector3D.cpp"
	"src/rocket/AeroCoefficient.cpp"
	"src/rocket/Engine.cpp"
	"src/rocket/RocketSpec.cpp"
	"src/solver/Solver.cpp"
)

add_library(prologue-core STATIC ${PROLOGUE_CORE_SOURCE_FILES})
target_include_directories(prologue-core PUBLIC "src/")
target_compile_features(prologue-core PUBLIC cxx_std_20)

set(PROLOGUE_IO_SOURCE_FILES
	"src/io/ApplicationSettingsReader.cpp"
	"src/io/SimulationInputReader.cpp"
	"src/io/MeasuredWindProfileReader.cpp"
	"src/utils/JsonUtils.cpp"
)

add_library(prologue-io STATIC ${PROLOGUE_IO_SOURCE_FILES})
target_include_directories(prologue-io PUBLIC "src/")
target_include_directories(prologue-io SYSTEM PRIVATE "deps/include/")
target_link_libraries(prologue-io PUBLIC prologue-core)
target_compile_features(prologue-io PUBLIC cxx_std_20)

set(SOURCE_FILES
	"src/app/CommandLine.cpp"
	"src/app/FetchVersion.cpp"
	"src/app/InputDiagnosticPrinter.cpp"
	"src/app/Option.cpp"

	"src/env/GeoCoordinate.cpp"
	"src/env/Map.cpp"

	"src/math/Algorithm.cpp"

	"src/result/ResultSaver.cpp"
	"src/result/GeographicResult.cpp"

	"src/simulator/DetailSimulator.cpp"
	"src/simulator/ScatterSimulator.cpp"
	"src/simulator/SimulatorBase.cpp"
	"src/simulator/SimulatorFactory.cpp"

	"src/main.cpp"
)

add_executable(Prologue ${SOURCE_FILES})
target_include_directories(Prologue PRIVATE "src/")
target_include_directories(Prologue SYSTEM PRIVATE "deps/include/")

file(GLOB KMLBASE_SOURCES "deps/include/kml/base/*.cc")
list(REMOVE_ITEM KMLBASE_SOURCES
	"${CMAKE_CURRENT_SOURCE_DIR}/deps/include/kml/base/expat_handler_ns.cc"
	"${CMAKE_CURRENT_SOURCE_DIR}/deps/include/kml/base/expat_parser.cc"
	"${CMAKE_CURRENT_SOURCE_DIR}/deps/include/kml/base/time_util.cc"
	"${CMAKE_CURRENT_SOURCE_DIR}/deps/include/kml/base/uri_parser.cc"
	"${CMAKE_CURRENT_SOURCE_DIR}/deps/include/kml/base/zip_file.cc"
)
if (WIN32)
	list(REMOVE_ITEM KMLBASE_SOURCES
		"${CMAKE_CURRENT_SOURCE_DIR}/deps/include/kml/base/file_posix.cc"
	)
else()
	list(REMOVE_ITEM KMLBASE_SOURCES
		"${CMAKE_CURRENT_SOURCE_DIR}/deps/include/kml/base/file_win32.cc"
	)
endif()

file(GLOB KMLDOM_SOURCES "deps/include/kml/dom/*.cc")
list(REMOVE_ITEM KMLDOM_SOURCES
	"${CMAKE_CURRENT_SOURCE_DIR}/deps/include/kml/dom/kml_handler.cc"
	"${CMAKE_CURRENT_SOURCE_DIR}/deps/include/kml/dom/kml_handler_ns.cc"
	"${CMAKE_CURRENT_SOURCE_DIR}/deps/include/kml/dom/parser.cc"
)

add_library(kmlbase STATIC ${KMLBASE_SOURCES})
target_include_directories(kmlbase PUBLIC "deps/include/")
if(WIN32)
    target_compile_definitions(kmlbase PRIVATE UNICODE _UNICODE strncasecmp=_strnicmp)
endif()

add_library(kmldom STATIC ${KMLDOM_SOURCES})
target_include_directories(kmldom PUBLIC "deps/include/")
target_link_libraries(kmldom PRIVATE kmlbase)

target_link_libraries(Prologue PRIVATE prologue-core prologue-io kmldom kmlbase)

add_subdirectory("test")
