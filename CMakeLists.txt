cmake_minimum_required (VERSION 3.0)

option(OVERIDE_HUNTER_RAPIDJASON "Overides hunter for RapidJson" OFF)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/CMakeModules)
message(STATUS "Module path:${CMAKE_MODULE_PATH}")

#include(HunterPackage)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/Hunter/HunterGate.cmake)

#HunterGate(
#    URL "https://github.com/ruslo/hunter/archive/v0.18.64.tar.gz"
#    SHA1 "baf9c8cc4f65306f0e442b5419967b4c4c04589a"
#    LOCAL
#)
HunterGate(
    URL "https://github.com/ruslo/hunter/archive/v0.19.208.tar.gz"
    SHA1 "4128ac8c79c21b250bf825e8119ce0bff05e5132"
    LOCAL
)

project(voxigen)

set_property( GLOBAL PROPERTY USE_FOLDERS ON)
#message(STATUS "module path:${CMAKE_MODULE_PATH}")

hunter_add_package(glm)
find_package(glm CONFIG REQUIRED)
hunter_add_package(glfw)
find_package(glfw3 REQUIRED)
hunter_add_package(glew)
find_package(glew CONFIG REQUIRED)
hunter_add_package(RapidJSON)
find_package(RapidJSON CONFIG REQUIRED)


hunter_add_package(Boost COMPONENTS filesystem system)
find_package(Boost REQUIRED COMPONENTS filesystem system)

add_subdirectory(FastNoiseSIMD)
add_subdirectory(opengl_util)

if(MSVC)
    add_definitions(-DNOMINMAX)
endif()

set(voxigen_headers
    include/voxigen/biome.h
    include/voxigen/boundingBox.h
    include/voxigen/cell.h
    include/voxigen/chunk.h
    include/voxigen/chunkHandle.h
    include/voxigen/chunkFunctions.h
    include/voxigen/chunkMesh.h
    include/voxigen/classFactory.h
    include/voxigen/coords.h
    include/voxigen/cubicMeshBuilder.h
    include/voxigen/dataHandler.h
    include/voxigen/dataStore.h
    include/voxigen/defines.h
    include/voxigen/entity.h
    include/voxigen/equiRectWorldGenerator.h
    include/voxigen/generator.h
    include/voxigen/gridDescriptors.h
    include/voxigen/initGlew.h
    include/voxigen/jsonSerializer.h
    include/voxigen/object.h
    include/voxigen/regularGrid.h
    include/voxigen/region.h
    include/voxigen/regionHandle.h
    include/voxigen/simpleCamera.h
    include/voxigen/simpleChunkRenderer.h
    include/voxigen/simpleFileSystem.h
    include/voxigen/simpleRenderer.h
    include/voxigen/simpleShapes.h
)

set(voxigen_sources
    src/data/biome.cpp
    src/data/entity.cpp
    src/data/gridDescriptors.cpp
    src/data/object.cpp
    src/generator/equiRectWorldGenerator.cpp
    src/rendering/initGlew.cpp
    src/rendering/chunkMesh.cpp
    src/rendering/simpleCamera.cpp
    src/utils/coords.cpp
    src/utils/jsonSerializer.cpp
    src/utils/simpleFileSystem.cpp
)

set(voxigen_libraries
    glm
    opengl_util
    fastNoise
    glew::glew
    Boost::filesystem
    Boost::system
    RapidJSON::rapidjson
)

if(VOXIGEN_OUTPUT_DIRECTORY_DEBUG)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${VOXIGEN_OUTPUT_DIRECTORY_DEBUG})
endif()
if(VOXIGEN_OUTPUT_DIRECTORY_RELEASE)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${VOXIGEN_OUTPUT_DIRECTORY_RELEASE})
endif()

add_library(voxigen SHARED
    ${voxigen_headers}
    ${voxigen_sources}
)

include_directories(${CMAKE_CURRENT_LIST_DIR}/include)

target_link_libraries(voxigen ${voxigen_libraries})

#test app
set(testApp_sources
    testApp/main.cpp
)

add_executable(testApp ${testApp_sources})
target_link_libraries(testApp voxigen glfw)
