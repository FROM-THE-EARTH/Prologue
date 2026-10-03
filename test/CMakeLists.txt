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

test(rocket-data-validation "rocket/DataValidation.cpp")

set(SOURCE_FILES "runner/SimulationRunner.cpp")
test(runner-simulation "${SOURCE_FILES}")
target_link_libraries(runner-simulation PRIVATE prologue-runner)

if(PROLOGUE_BUILD_CLI)
set(SOURCE_FILES "cli/ResultDirectory.cpp" "../src/cli/ResultDirectory.cpp")
test(cli-result-directory "${SOURCE_FILES}")
endif()

if(PROLOGUE_BUILD_FILE_IO)
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

test(io-application-settings-writer "io/ApplicationSettingsWriter.cpp")
target_link_libraries(io-application-settings-writer PRIVATE prologue-io)
if(MSVC)
    target_compile_options(io-application-settings-writer PRIVATE /utf-8)
endif()

test(result-saver "result/ResultSaver.cpp")
target_link_libraries(result-saver PRIVATE prologue-result)
endif()

if(PROLOGUE_BUILD_GUI)
    test(gui-result-series "gui/ResultSeries.cpp")
    target_link_libraries(gui-result-series PRIVATE prologue-gui-widgets)
    test(gui-web-mercator "gui/WebMercator.cpp")
    target_link_libraries(gui-web-mercator PRIVATE prologue-gui-widgets)
    test(gui-kml-overlay "gui/KmlOverlay.cpp")
    target_link_libraries(gui-kml-overlay PRIVATE prologue-gui-widgets)
    test(gui-input-editors "gui/InputEditors.cpp")
    target_link_libraries(gui-input-editors PRIVATE prologue-gui-widgets)
    test(gui-plot-panel "gui/PlotPanel.cpp")
    target_link_libraries(gui-plot-panel PRIVATE prologue-gui-widgets)
    test(gui-simulation-controller "gui/SimulationController.cpp")
    target_link_libraries(gui-simulation-controller PRIVATE prologue-gui-widgets)
    set_tests_properties(gui-simulation-controller PROPERTIES WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/application")
    if(MSVC)
        target_compile_options(gui-input-editors PRIVATE /utf-8)
        target_compile_options(gui-simulation-controller PRIVATE /utf-8)
    endif()
    set_tests_properties(gui-input-editors gui-plot-panel PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
    set_tests_properties(gui-result-series gui-web-mercator gui-kml-overlay gui-input-editors gui-plot-panel gui-simulation-controller PROPERTIES
        ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:$<TARGET_FILE_DIR:Qt6::Core>;PATH=path_list_prepend:$<TARGET_FILE_DIR:JKQTPlotterSharedLib>"
    )
endif()

add_test(NAME architecture-core-boundary
    COMMAND ${CMAKE_COMMAND} -DPROLOGUE_SOURCE_DIR=${CMAKE_SOURCE_DIR}
            -P ${CMAKE_CURRENT_SOURCE_DIR}/architecture/CheckCoreBoundary.cmake)
