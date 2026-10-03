# Keep numerical code independent of frontends and file-format adapters.
file(GLOB_RECURSE core_files
    "${PROLOGUE_SOURCE_DIR}/src/core/*.hpp" "${PROLOGUE_SOURCE_DIR}/src/core/*.cpp"
    "${PROLOGUE_SOURCE_DIR}/src/dynamics/*.hpp" "${PROLOGUE_SOURCE_DIR}/src/dynamics/*.cpp"
    "${PROLOGUE_SOURCE_DIR}/src/math/*.hpp" "${PROLOGUE_SOURCE_DIR}/src/math/*.cpp"
    "${PROLOGUE_SOURCE_DIR}/src/rocket/*.hpp" "${PROLOGUE_SOURCE_DIR}/src/rocket/*.cpp"
    "${PROLOGUE_SOURCE_DIR}/src/solver/*.hpp" "${PROLOGUE_SOURCE_DIR}/src/solver/*.cpp"
    "${PROLOGUE_SOURCE_DIR}/src/env/*.hpp" "${PROLOGUE_SOURCE_DIR}/src/misc/Constant.hpp")
foreach(source IN LISTS core_files)
    file(READ "${source}" contents)
    if(contents MATCHES "#[ \t]*include[ \t]*[<\"](cli/|io/|config/|project/|runner/|result/|geography/|boost/|kml/|Qt|Q[A-Z]|filesystem|fstream|misc/Platform)")
        message(FATAL_ERROR "Core includes an interface dependency: ${source}")
    endif()
    if(contents MATCHES "std::(cout|cerr|cin)|system[ \t]*\\(|popen[ \t]*\\(")
        message(FATAL_ERROR "Core performs frontend IO: ${source}")
    endif()
endforeach()
