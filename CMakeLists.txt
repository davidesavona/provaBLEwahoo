cmake_minimum_required(VERSION 3.5.0)
project(BLEwahoo VERSION 0.1.0 LANGUAGES C CXX)

file(GLOB BLEwahoo_SRC
     "*.h"
     "*.cpp"
)
add_executable(BLEwahoo ${BLEwahoo_SRC})

include(CTest)
enable_testing()
set(CMAKE_AUTOMOC ON)
set_target_properties(${PROJECT_NAME} PROPERTIES AUTOMOC ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core REQUIRED)
find_package(Qt5 COMPONENTS Bluetooth Widgets REQUIRED)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

target_link_libraries(BLEwahoo Qt${QT_VERSION_MAJOR}::Core Qt5::Bluetooth Qt5::Widgets wiringPi pthread bcm2835)