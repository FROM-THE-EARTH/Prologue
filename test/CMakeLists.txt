include_directories("../src/")

macro(test name files)
    add_executable(${name} ${files})
    add_test(NAME ${name} COMMAND ${name} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endmacro()

set(SOURCE_FILES "math/Algorithm.cpp" "../src/app/CommandLine.cpp" "../src/math/Algorithm.cpp")
test(math-algorithm "${SOURCE_FILES}")

set(SOURCE_FILES "math/Quaternion.cpp" "../src/math/Quaternion.cpp" "../src/math/Vector3D.cpp")
test(math-quaternion "${SOURCE_FILES}")

set(SOURCE_FILES "math/Vector3D.cpp" "../src/math/Vector3D.cpp" "../src/math/Quaternion.cpp")
test(math-vector3d "${SOURCE_FILES}")

set(SOURCE_FILES "dynamics/WindProfile.cpp" "../src/dynamics/WindProfile.cpp")
test(dynamics-wind-profile "${SOURCE_FILES}")

set(SOURCE_FILES "dynamics/StandardAtmosphere1976.cpp" "../src/dynamics/StandardAtmosphere1976.cpp")
test(dynamics-standard-atmosphere-1976 "${SOURCE_FILES}")
