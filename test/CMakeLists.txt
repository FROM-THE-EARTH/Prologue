include_directories("../src/")
include_directories(SYSTEM "../deps/include/")

macro(test name files)
    add_executable(${name} ${files})
    target_link_libraries(${name} PRIVATE prologue-core)
    add_test(NAME ${name} COMMAND ${name} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endmacro()

set(SOURCE_FILES "math/Interpolation.cpp")
test(math-interpolation "${SOURCE_FILES}")

set(SOURCE_FILES "math/Quaternion.cpp")
test(math-quaternion "${SOURCE_FILES}")

set(SOURCE_FILES "math/Vector3D.cpp")
test(math-vector3d "${SOURCE_FILES}")

set(SOURCE_FILES "dynamics/WindProfile.cpp")
test(dynamics-wind-profile "${SOURCE_FILES}")

set(SOURCE_FILES
    "dynamics/WindModel.cpp"
)
test(dynamics-wind-model "${SOURCE_FILES}")

set(SOURCE_FILES
    "dynamics/WindModelOriginal.cpp"
)
test(dynamics-wind-model-original "${SOURCE_FILES}")

set(SOURCE_FILES "dynamics/StandardAtmosphere1976.cpp")
test(dynamics-standard-atmosphere-1976 "${SOURCE_FILES}")

set(SOURCE_FILES "core/SimulationInput.cpp")
test(core-simulation-input "${SOURCE_FILES}")

set(SOURCE_FILES "core/Simulation.cpp")
test(core-simulation "${SOURCE_FILES}")

set(SOURCE_FILES "runner/SimulationRunner.cpp")
test(runner-simulation "${SOURCE_FILES}")
target_link_libraries(runner-simulation PRIVATE prologue-runner)

set(SOURCE_FILES "cli/ResultDirectory.cpp" "../src/cli/ResultDirectory.cpp")
test(cli-result-directory "${SOURCE_FILES}")

set(SOURCE_FILES
	"io/ProjectJsonSerializer.cpp"
)
test(io-project-json-serializer "${SOURCE_FILES}")
target_link_libraries(io-project-json-serializer PRIVATE prologue-io)
set_tests_properties(io-project-json-serializer PROPERTIES
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/application"
)

set(SOURCE_FILES
    "io/ApplicationSettingsReader.cpp"
)
test(io-application-settings-reader "${SOURCE_FILES}")
target_link_libraries(io-application-settings-reader PRIVATE prologue-io)
