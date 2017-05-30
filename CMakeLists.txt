cmake_minimum_required (VERSION 3.0)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/CMakeModules)
message(STATUS "module path:${CMAKE_MODULE_PATH}")

#include(HunterPackage)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/Hunter/HunterGate.cmake)

HunterGate(
    URL "https://github.com/ruslo/hunter/archive/v0.18.64.tar.gz"
    SHA1 "baf9c8cc4f65306f0e442b5419967b4c4c04589a"
    LOCAL
)

project(voxigen)

set_property( GLOBAL PROPERTY USE_FOLDERS ON)
#message(STATUS "module path:${CMAKE_MODULE_PATH}")

hunter_add_package(glm)
find_package(glm CONFIG REQUIRED)
hunter_add_package(glfw)
find_package(glfw3 REQUIRED)

if(MSVC)
	add_definitions(-DNOMINMAX)
endif()

set(voxigen_headers
	include/voxigen/boundingBox.h
    include/voxigen/chunk.h
	include/voxigen/chunkCache.h
    include/voxigen/chunkHandler.h
)

set(voxigen_sources
	src/chunker/boundingBox.cpp
    src/chunker/chunk.cpp
	src/chunker/chunkCache.cpp
    src/chunker/chunkHandler.cpp
)

set(voxigen_libraries
	${GLFW_LIBRARIES}
    bullet.lib
)

add_library(voxigen SHARED
    ${voxigen_headers}
    ${voxigen_sources}
)

include_directories(${GLM_INCLUDES})
include_directories(${GLFW_INCLUDES})
include_directories(${CMAKE_CURRENT_LIST_DIR}/include)

#use bullet from urho3D
include_directories(${URHO3D_DIR}/Source/ThirdParty/Bullet)
link_directories(${URHO3D_DIR}/buildx64/Source/ThirdParty/Bullet/Debug ${URHO3D_DIR}/buildx64/Source/ThirdParty/Bullet/Release)

include_directories(${POLYVOX_DIR}/include)

target_link_libraries(voxigen ${voxigen_libraries})

#test app
set(testApp_sources
    testApp/main.cpp
)

add_executable(testApp ${testApp_sources})
target_link_libraries(testApp voxigen)
