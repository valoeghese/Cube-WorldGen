cmake_minimum_required (VERSION 3.8)
project(project_Dummy)
add_subdirectory(CWSDK)
add_library (Dummy SHARED
"main.cpp"
"WorldRegion.cpp"
"WorldRegion.h"
"JitteredGrid.h"
"JitteredGrid.cpp"
 "Structure.h" "Structure.cpp" "City.h" "City.cpp")
target_link_libraries (Dummy LINK_PUBLIC CWSDK)