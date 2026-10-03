# GUI dependencies are optional and never propagate into prologue-core.
cmake_minimum_required(VERSION 3.24)
find_package(Qt6 6.5 REQUIRED GLOBAL COMPONENTS Widgets Concurrent Network Svg PrintSupport Xml OpenGL)
include(FetchContent)

find_package(JKQTPlotterSharedLib QUIET CONFIG GLOBAL)
if(NOT TARGET JKQTPlotterSharedLib)
    set(JKQtPlotter_BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE)
    set(JKQtPlotter_BUILD_STATIC_LIBS OFF CACHE BOOL "" FORCE)
    set(JKQtPlotter_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(jkqtplotter
        URL https://github.com/jkriege2/JKQtPlotter/archive/0d4e3bff7dfac7eb40ec7b14eda1f188b448a119.zip
        URL_HASH SHA256=b1dfa381cd1d4d515f984a875809d3111e53462069303e73c56872e88da64bea
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
    FetchContent_MakeAvailable(jkqtplotter)
    # The demo uses the full plotter; the upstream fast plotter is unrelated.
    if(TARGET JKQTFastPlotterSharedLib)
        set_target_properties(JKQTFastPlotterSharedLib PROPERTIES EXCLUDE_FROM_ALL TRUE)
    endif()
endif()

# Only local, bounded-memory ZIP reading is needed for KMZ overlays.
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared dependency libraries" FORCE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(INSTALL_PROJECT OFF CACHE BOOL "" FORCE)
set(BUILD_NO_STDIO ON CACHE BOOL "" FORCE)
FetchContent_Declare(miniz
    URL https://github.com/richgel999/miniz/archive/77d0dce8627735138c51770d1799a1ef48f2117d.zip
    URL_HASH SHA256=dafc25ba2b722970ca9b41f8103bf04a61d23dcc7a082905138946a318ff0bfb
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
FetchContent_MakeAvailable(miniz)

add_library(prologue-gui-widgets STATIC
    "MainWindow.cpp"
    "FileActions.cpp"
    "SimulationController.cpp"
    "MapSeriesBuilder.cpp"
    "input/ProjectEditor.cpp"
    "input/NumericInput.cpp"
    "input/SettingsEditor.cpp"
    "plot/PlotPanel.cpp"
    "plot/ResultSeries.cpp"
    "map/MapWidget.cpp"
    "map/KmlOverlay.cpp")
target_include_directories(prologue-gui-widgets PUBLIC "${CMAKE_SOURCE_DIR}/src")
target_link_libraries(prologue-gui-widgets PUBLIC
    prologue-runner prologue-io prologue-result
    Qt6::Widgets Qt6::Concurrent Qt6::Network Qt6::Svg Qt6::PrintSupport Qt6::Xml
    JKQTPlotterSharedLib miniz)
target_compile_features(prologue-gui-widgets PUBLIC cxx_std_20)
if(MSVC)
    target_compile_options(prologue-gui-widgets PRIVATE /utf-8)
endif()

add_executable(PrologueGUI WIN32 "main.cpp")
target_link_libraries(PrologueGUI PRIVATE prologue-gui-widgets)
set_target_properties(PrologueGUI PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/gui"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/gui"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/gui")
if(WIN32)
    # Keep the LGPL libraries replaceable beside the demo executable.
    foreach(library JKQTPlotterSharedLib JKQTCommonSharedLib JKQTMathTextSharedLib)
        add_custom_command(TARGET PrologueGUI POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "$<TARGET_FILE:${library}>" "$<TARGET_FILE_DIR:PrologueGUI>")
    endforeach()
    get_target_property(qtCoreLocation Qt6::Core IMPORTED_LOCATION_DEBUG)
    get_filename_component(qtBinaryDirectory "${qtCoreLocation}" DIRECTORY)
    find_program(PROLOGUE_WINDEPLOYQT windeployqt HINTS "${qtBinaryDirectory}" REQUIRED)
    add_custom_command(TARGET PrologueGUI POST_BUILD
        COMMAND "${PROLOGUE_WINDEPLOYQT}" "$<IF:$<CONFIG:Debug>,--debug,--release>"
            --no-translations --no-compiler-runtime
            --dir "$<TARGET_FILE_DIR:PrologueGUI>"
            "$<TARGET_FILE:PrologueGUI>" "$<TARGET_FILE:JKQTPlotterSharedLib>"
        VERBATIM)
    if(IS_DIRECTORY "${qtBinaryDirectory}/../sbom")
        add_custom_command(TARGET PrologueGUI POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory "${qtBinaryDirectory}/../sbom"
                "$<TARGET_FILE_DIR:PrologueGUI>/licenses/qt-sbom")
    endif()
endif()

add_custom_command(TARGET PrologueGUI POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/licenses"
        "$<TARGET_FILE_DIR:PrologueGUI>/licenses"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/THIRD_PARTY_NOTICES.md"
        "$<TARGET_FILE_DIR:PrologueGUI>/THIRD_PARTY_NOTICES.md")
# Copy only the repository's explicit sample files, never users' input or temp files.
foreach(sample prologue.settings.json input/spec/spec_single.json input/spec/spec_multi.json
        input/thrust/Sample_G40-4W.txt input/thrust/Sample_K240.txt
        input/aero_coef/sample.csv input/wind/sample.csv)
    get_filename_component(sampleDirectory "${sample}" DIRECTORY)
    add_custom_command(TARGET PrologueGUI POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:PrologueGUI>/sample/${sampleDirectory}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/application/${sample}"
            "$<TARGET_FILE_DIR:PrologueGUI>/sample/${sample}")
endforeach()
