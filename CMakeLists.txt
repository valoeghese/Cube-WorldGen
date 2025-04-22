cmake_minimum_required (VERSION 3.8)
project(project_NewAdventures)
add_subdirectory(CWSDK)
add_library (NewAdventures SHARED
	"main.cpp"
	"src/WorldRegion.cpp"
	"src/WorldRegion.h"
	"src/JitteredGrid.h"
	"src/JitteredGrid.cpp"
	"src/Structure.h"
	"src/Structure.cpp"
	"src/City.h"
	"src/City.cpp"
	"src/DebugTree.h"
	"src/DebugTree.cpp"
    "src/hooks/WorldGenHooks.h")
target_link_libraries (NewAdventures LINK_PUBLIC CWSDK)