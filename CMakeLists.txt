cmake_minimum_required (VERSION 3.0)

option(OVERIDE_HUNTER_RAPIDJASON "Overides hunter for RapidJson" OFF)
option(USE_OCTOMAP "Use octomap octree transversal" OFF)
option(VOXIGEN_TEXTURE "Setup texture classes" ON)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_SOURCE_DIR}/CMakeModules)
message(STATUS "Module path:${CMAKE_MODULE_PATH}")

set(HUNTER_STATUS_DEBUG ON)
set(
    HUNTER_CACHE_SERVERS
    "https://github.com/huntercache/voxigen"
    CACHE
    STRING
    "Default cache server"
)

string(COMPARE EQUAL "$ENV{TRAVIS}" "true" is_travis)
string(COMPARE EQUAL "$ENV{APPVEYOR}" "True" is_appveyor)
string(COMPARE EQUAL "$ENV{GITHUB_USER_PASSWORD}" "" password_is_empty)

if((is_travis OR is_appveyor) AND NOT password_is_empty)
  option(HUNTER_RUN_UPLOAD "Upload cache binaries" ON)
endif()
message(STATUS "Hunter upload: ${HUNTER_RUN_UPLOAD}")

set(
    HUNTER_PASSWORDS_PATH
    "${CMAKE_CURRENT_LIST_DIR}/cmake/Hunter/passwords.cmake"
    CACHE
    FILEPATH
    "Hunter passwords"
)

#include(HunterPackage)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/Hunter/HunterGate.cmake)

HunterGate(
    URL "https://github.com/ruslo/hunter/archive/v0.23.34.tar.gz"
    SHA1 "70287b1ffa810ee4e952052a9adff9b4856d0d54"
)

#HunterGate(
#    URL "https://github.com/ruslo/hunter/archive/v0.23.16.tar.gz"
#    SHA1 "dbe5b1c966414c4a055983c11d5251a2d8f2f16d"
#    LOCAL
#)

#HunterGate(
#    URL "https://github.com/ruslo/hunter/archive/v0.19.208.tar.gz"
#    SHA1 "4128ac8c79c21b250bf825e8119ce0bff05e5132"
#    LOCAL
#)

project(voxigen)

#setup build folders
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin/)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib/)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib/)
#for multi-config builds (e.g. msvc)
foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG_UPPER)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG_UPPER} ${PROJECT_BINARY_DIR}/bin/${OUTPUTCONFIG}/)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG_UPPER} ${PROJECT_BINARY_DIR}/lib/${OUTPUTCONFIG}/)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG_UPPER} ${PROJECT_BINARY_DIR}/lib/${OUTPUTCONFIG}/)
endforeach(OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES)

message(STATUS "USE_OCTOMAP ${USE_OCTOMAP}")
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
#message(STATUS "module path:${CMAKE_MODULE_PATH}")

hunter_add_package(glm)
find_package(glm CONFIG REQUIRED)
hunter_add_package(glfw)
find_package(glfw3 REQUIRED)
hunter_add_package(glew)
find_package(glew CONFIG REQUIRED)
hunter_add_package(RapidJSON)
find_package(RapidJSON CONFIG REQUIRED)
hunter_add_package(gflags)
find_package(gflags CONFIG REQUIRED)
hunter_add_package(glog)
find_package(glog CONFIG REQUIRED)


hunter_add_package(Boost COMPONENTS filesystem system)
find_package(Boost REQUIRED COMPONENTS filesystem system)

if(VOXIGEN_TEXTURE)
    hunter_add_package(PNG)
    find_package(PNG)

    set(IMGLIB_USE_BOOST_FILESYSTEM ON CACHE BOOL "Turn on/off support boost filesystem")
    set(IMGLIB_USE_COMPILETIME_TYPE OFF CACHE BOOL "Turn on/off support for compile time type informat")
    set(IMGLIB_USE_PNG ON CACHE BOOL "Turn on/off support for PNG")
    set(IMGLIB_USE_EIGEN ON CACHE BOOL "Turn on/off support for eigen library")
    set(IMGLIB_USE_OPENCL OFF CACHE BOOL "Turn on/off support for OpenCl")
    set(IMGLIB_USE_OPENCL_CPP OFF CACHE BOOL "Turn on/off support for OpenCl CPP classes")
    set(IMGLIB_USE_CUDA OFF CACHE BOOL "Turn on/off support for CUDA")
    set(IMGLIB_BUILD_TEST OFF CACHE BOOL "Turns on test application")

    add_subdirectory(imglib)
    set_target_properties(imglib PROPERTIES FOLDER Libraries)
endif()

add_subdirectory(FastNoiseSIMD)
set_target_properties(fastNoise PROPERTIES FOLDER Libraries)
add_subdirectory(opengl_util)
set_target_properties(opengl_util PROPERTIES FOLDER Libraries)
add_subdirectory(generic)
set_target_properties(generic PROPERTIES FOLDER Libraries)

if(USE_OCTOMAP)
    add_definitions(-DUSE_OCTOMAP)
    set(OCTOVIS_QT5 OFF)
    set(BUILD_OCTOVIS_SUBPROJECT OFF)
    set(BUILD_DYNAMICETD3D_SUBPROJECT OFF)
    add_subdirectory(octomap)
    set_target_properties (octomap PROPERTIES FOLDER Libraries)
    set_target_properties (octomap-static PROPERTIES FOLDER Libraries)
    set_target_properties (octomath PROPERTIES FOLDER Libraries)
    set_target_properties (octomath-static PROPERTIES FOLDER Libraries)
endif()

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
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
    include/voxigen/faces.h
    include/voxigen/generator.h
    include/voxigen/gridDescriptors.h
    include/voxigen/handleState.h
    include/voxigen/heightmapMeshBuilder.h
    include/voxigen/initGlew.h
    include/voxigen/jsonSerializer.h
    include/voxigen/object.h
    include/voxigen/processQueue.h
    include/voxigen/regular2DGrid.h
    include/voxigen/regularGrid.h
    include/voxigen/region.h
    include/voxigen/regionHandle.h
    include/voxigen/renderAction.h
    include/voxigen/renderCube.h
    include/voxigen/renderPrepThread.h
    include/voxigen/search.h
    include/voxigen/simpleCamera.h
    include/voxigen/simpleChunkRenderer.h
    include/voxigen/simpleFilesystem.h
    include/voxigen/simpleRegionRenderer.h
    include/voxigen/simpleRenderer.h
    include/voxigen/simpleShapes.h
)

set(voxigen_inline
    include/voxigen/chunkHandle.inl
    include/voxigen/processQueue.inl
    include/voxigen/renderCube.inl
    include/voxigen/renderPrepThread.inl
    include/voxigen/regionHandle.inl
    include/voxigen/simpleChunkRenderer.inl
    include/voxigen/simpleRegionRenderer.inl
    include/voxigen/simpleRenderer.inl
)

set(voxigen_sources
    src/data/biome.cpp
    src/data/entity.cpp
    src/data/gridDescriptors.cpp
    src/data/regular2DGrid.cpp
    src/data/object.cpp
    src/generator/equiRectWorldGenerator.cpp
    src/rendering/initGlew.cpp
    src/rendering/chunkMesh.cpp
    src/rendering/simpleCamera.cpp
    src/utils/coords.cpp
    src/utils/jsonSerializer.cpp
    src/utils/simpleFilesystem.cpp
)


set(voxigen_public_libraries
    fastNoise
    glew::glew
    Boost::filesystem
    Boost::system
    gflags
    glog::glog
    generic
)

set(voxigen_libraries ${voxigen_public_libraries}
    opengl_util
    glm
    RapidJSON::rapidjson
)

if(VOXIGEN_TEXTURE)
    set(voxigen_headers ${voxigen_headers}
        include/voxigen/blendMethod.h
        include/voxigen/chunkTextureMesh.h
        include/voxigen/layerMethod.h
        include/voxigen/textureAtlas.h
        include/voxigen/texturedMesh.h
        include/voxigen/texturePack.h
    )

    set(voxigen_sources ${voxigen_sources}
        src/rendering/blendMethod.cpp
        src/rendering/chunkTextureMesh.cpp
        src/rendering/layerMethod.cpp
        src/rendering/textureAtlas.cpp
        src/rendering/texturePack.cpp
    )

    set(voxigen_libraries ${voxigen_libraries}
        imglib
    )
endif()

if(USE_OCTOMAP)
    set(voxigen_libraries ${voxigen_libraries} octomap-static)
endif()

if(VOXIGEN_OUTPUT_DIRECTORY_DEBUG)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${VOXIGEN_OUTPUT_DIRECTORY_DEBUG})
endif()
if(VOXIGEN_OUTPUT_DIRECTORY_RELEASE)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${VOXIGEN_OUTPUT_DIRECTORY_RELEASE})
endif()

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
add_library(voxigen SHARED
    ${voxigen_headers}
    ${voxigen_inline}
    ${voxigen_sources}
)
else()
add_library(voxigen STATIC
    ${voxigen_headers}
    ${voxigen_inline}
    ${voxigen_sources}
)
endif()

#if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
#    target_compile_definitions(voxigen PUBLIC GLM_FORCE_EXPLICIT_CTOR)
#    if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 8.0)
#        target_compile_options(voxigen PUBLIC "-lstdc++fs")
#    else()
#        target_compile_options(voxigen PUBLIC "-lstdc++fs")
#    endif()
#endif()

if(TARGET fastNoise_sse2)
    add_dependencies(voxigen fastNoise_sse2)
    set_target_properties(fastNoise_sse2 PROPERTIES FOLDER Libraries)
endif()
if(TARGET fastNoise_sse41)
    add_dependencies(voxigen fastNoise_sse41)
    set_target_properties(fastNoise_sse41 PROPERTIES FOLDER Libraries)
endif()
if(TARGET fastNoise_avx2)
    add_dependencies(voxigen fastNoise_avx2)
    set_target_properties(fastNoise_avx2 PROPERTIES FOLDER Libraries)
endif()
if(TARGET fastNoise_avx512)
    add_dependencies(voxigen fastNoise_avx512)
    set_target_properties(fastNoise_avx512 PROPERTIES FOLDER Libraries)
endif()

source_group("Source Files" FILES ${voxigen_inline})
include_directories(${CMAKE_CURRENT_LIST_DIR}/include)

target_link_libraries(voxigen ${voxigen_libraries})

#if(APPLE)
#    target_include_directories(voxigen PUBLIC "/opt/X11/include")
#    target_link_libraries(voxigen X11)
#endif()

#test app
set(testApp_sources
    testApp/main.cpp
)

add_executable(testApp ${testApp_sources})
target_link_libraries(testApp voxigen glfw)

set(voxigen_public_libraries ${voxigen_public_libraries} glfw)
foreach(target ${voxigen_public_libraries})
    set(VS_ENVIROMENT_DIR ${VS_ENVIROMENT_DIR} "$<TARGET_FILE_DIR:${target}>")
endforeach()
set(VS_WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})

include(CreateLaunchers)

create_target_launcher(testApp
    RUNTIME_LIBRARY_DIRS "${VS_ENVIROMENT_DIR}"
    WORKING_DIRECTORY "${VS_WORKING_DIRECTORY}"
)

