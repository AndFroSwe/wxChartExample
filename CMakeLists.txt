cmake_minimum_required(VERSION 3.31 FATAL_ERROR)

project(wxChartExample)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(wxWidgets COMPONENTS base core REQUIRED)

add_executable(ChartApp WIN32 ChartApp.cpp)
target_link_libraries(ChartApp
  PRIVATE ${wxWidgets_LIBRARIES}
)
target_include_directories(ChartApp
   PRIVATE ${wxWidgets_INCLUDE_DIRS}
)
