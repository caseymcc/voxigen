cmake_minimum_required (VERSION 3.0)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/CMakeModules)
message(STATUS "module path:${CMAKE_MODULE_PATH}")

include(HunterPackage)

HunterGate(
    URL "https://github.com/ruslo/hunter/archive/v0.15.33.tar.gz"
	SHA1 "7849ccc2607964afd5fc8b48130473a133665670"
    LOCAL
)

project(voxigen)

set_property( GLOBAL PROPERTY USE_FOLDERS ON)

#message(STATUS "module path:${CMAKE_MODULE_PATH}")
#include(addDependency)

#initCPM()
#add_dependency("glm" GIT_REPOSITORY "https://github.com/g-truc/glm.git" GIT_TAG "0.9.7.6")
#add_dependency("glfw" GIT_REPOSITORY "https://github.com/glfw/glfw.git" GIT_TAG "3.2")

hunter_add_package(glm)
find_package(glm CONFIG REQUIRED)
hunter_add_package(glfw)
find_package(glfw3 REQUIRED)

if(MSVC)
	add_definitions(-DNOMINMAX)
endif()

set(voxigen_headers
	chunker/boundingBox.h
    chunker/chunk.h
	chunker/chunkCache.h
    chunker/chunkHandler.h
)

set(voxigen_sources
	chunker/boundingBox.cpp
    chunker/chunk.cpp
	chunker/chunkCache.cpp
    chunker/chunkHandler.cpp
)

set(voxigen_libraries
	${GLFW_LIBRARIES}
)

add_library(voxigen SHARED
    ${voxigen_headers}
    ${voxigen_sources}
)

include_directories(${GLM_INCLUDES})
include_directories(${GLFW_INCLUDES})

#target_link_libraries(voxigen ${voxigen_libraries} ${CPM_LIBRARIES})
target_link_libraries(voxigen ${voxigen_libraries})

#set_target_properties(ImageFilters PROPERTIES 
#	FOLDER "filters"
#)