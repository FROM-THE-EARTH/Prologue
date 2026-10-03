cmake_minimum_required (VERSION 3.16.3)

if (APPLE)
	set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
endif()

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

include(CTest)
option(PROLOGUE_BUILD_CLI "Build the command-line interface" ON)
option(PROLOGUE_BUILD_GUI "Build the native Qt desktop demo" OFF)
set(PROLOGUE_FILE_IO_DEFAULT OFF)
if(PROLOGUE_BUILD_CLI OR PROLOGUE_BUILD_GUI)
	set(PROLOGUE_FILE_IO_DEFAULT ON)
endif()
option(PROLOGUE_BUILD_FILE_IO "Build file, geographic and result adapters" ${PROLOGUE_FILE_IO_DEFAULT})
if((PROLOGUE_BUILD_CLI OR PROLOGUE_BUILD_GUI) AND NOT PROLOGUE_BUILD_FILE_IO)
	message(FATAL_ERROR "The CLI and GUI require PROLOGUE_BUILD_FILE_IO=ON")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../application)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ../application)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ../application)

set(PROLOGUE_CORE_SOURCE_FILES
	"src/core/Simulation.cpp"
	"src/core/SimulationInput.cpp"
	"src/dynamics/AtmosphereSettings.cpp"
	"src/dynamics/StandardAtmosphere1976.cpp"
	"src/dynamics/WindModel.cpp"
	"src/dynamics/WindModelSettings.cpp"
	"src/dynamics/WindProfile.cpp"
	"src/math/Interpolation.cpp"
	"src/math/Quaternion.cpp"
	"src/math/Vector3D.cpp"
	"src/rocket/AeroCoefficient.cpp"
	"src/rocket/Engine.cpp"
	"src/rocket/RocketSpec.cpp"
	"src/solver/Solver.cpp"
	"src/solver/SolverSettings.cpp"
)

add_library(prologue-core STATIC ${PROLOGUE_CORE_SOURCE_FILES})
target_include_directories(prologue-core PUBLIC "src/")
target_compile_features(prologue-core PUBLIC cxx_std_20)

add_library(prologue-project STATIC
	"src/project/SimulationInputBuilder.cpp"
)
target_include_directories(prologue-project PUBLIC "src/")
target_link_libraries(prologue-project PUBLIC prologue-core)
target_compile_features(prologue-project PUBLIC cxx_std_20)

add_library(prologue-runner STATIC
	"src/runner/SimulationRunner.cpp"
)
target_include_directories(prologue-runner PUBLIC "src/")
target_link_libraries(prologue-runner PUBLIC prologue-core)
target_compile_features(prologue-runner PUBLIC cxx_std_20)

if(PROLOGUE_BUILD_FILE_IO)
add_library(prologue-geography STATIC
	"src/geography/GeoCoordinate.cpp"
	"src/geography/GeographicResult.cpp"
)
target_include_directories(prologue-geography PUBLIC "src/")
target_include_directories(prologue-geography SYSTEM PUBLIC "deps/include/")
target_link_libraries(prologue-geography PUBLIC prologue-core)
target_compile_features(prologue-geography PUBLIC cxx_std_20)

set(PROLOGUE_IO_SOURCE_FILES
	"src/io/ApplicationSettingsReader.cpp"
	"src/io/ApplicationSettingsWriter.cpp"
	"src/io/FileProjectResourceProvider.cpp"
	"src/io/MeasuredWindProfileReader.cpp"
	"src/io/ProjectJsonSerializer.cpp"
	"src/utils/JsonUtils.cpp"
)

add_library(prologue-io STATIC ${PROLOGUE_IO_SOURCE_FILES})
target_include_directories(prologue-io PUBLIC "src/")
target_include_directories(prologue-io SYSTEM PRIVATE "deps/include/")
target_link_libraries(prologue-io PUBLIC prologue-core prologue-project)
target_compile_features(prologue-io PUBLIC cxx_std_20)

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

add_library(prologue-result STATIC "src/result/ResultSaver.cpp")
target_include_directories(prologue-result PUBLIC "src/")
target_include_directories(prologue-result SYSTEM PRIVATE "deps/include/")
target_link_libraries(prologue-result PUBLIC prologue-geography PRIVATE kmldom kmlbase)
target_compile_features(prologue-result PUBLIC cxx_std_20)
endif()

if(PROLOGUE_BUILD_CLI)
add_executable(Prologue
	"src/cli/CommandLine.cpp"
	"src/cli/FetchVersion.cpp"
	"src/cli/InputDiagnosticPrinter.cpp"
	"src/cli/Option.cpp"
	"src/cli/ProgressObserver.cpp"
	"src/cli/ResultDirectory.cpp"
	"src/cli/ResultSaveProgressObserver.cpp"
	"src/cli/SimulationRequest.cpp"
	"src/main.cpp"
)
target_include_directories(Prologue PRIVATE "src/")
target_include_directories(Prologue SYSTEM PRIVATE "deps/include/")
target_link_libraries(Prologue PRIVATE prologue-runner prologue-io prologue-result)
endif()

if(PROLOGUE_BUILD_GUI)
	add_subdirectory("src/gui")
endif()

if(BUILD_TESTING)
	add_subdirectory("test")
endif()
