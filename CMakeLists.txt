cmake_minimum_required (VERSION 3.8)
project(project_NewAdventures)
add_subdirectory(CWSDK)
add_library (NewAdventures SHARED
	"main.cpp"
	"WorldRegion.cpp"
	"WorldRegion.h"
	"JitteredGrid.h"
	"JitteredGrid.cpp"
	"Structure.h"
	"Structure.cpp"
	"City.h"
	"City.cpp"
	"DebugTree.h"
	"DebugTree.cpp"
  )
target_link_libraries (NewAdventures LINK_PUBLIC CWSDK)