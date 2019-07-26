cmake_minimum_required (VERSION 3.0)

option(OVERIDE_HUNTER_RAPIDJASON "Overides hunter for RapidJson" OFF)
option(VOXIGEN_RENDERING "Enable simple rendering" ON)
option(VOXIGEN_TEXTURE "Setup texture classes" ON)
option(VOXIGEN_TESTAPP "Build test app" ON)
option(VOXIGEN_MAPGENAPP "Build mapgen app" ON)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_SOURCE_DIR}/CMakeModules)

set(HUNTER_STATUS_DEBUG ON)
set(HUNTER_KEEP_PACKAGE_SOURCES ON)
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

include(${CMAKE_CURRENT_LIST_DIR}/cmake/Hunter/HunterGate.cmake)

HunterGate(
    URL "https://github.com/ruslo/hunter/archive/v0.23.204.tar.gz"
    SHA1 "32cfed254da901f6f184027d530d8da47e035b85"
    LOCAL
)

project(voxigen)

#setup build folders
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin/)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib/)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib/)
#for multi-config builds (e.g. msvc)
if(CMAKE_CONFIGURATION_TYPES)
    foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
        string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG_UPPER)
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG_UPPER} ${PROJECT_BINARY_DIR}/bin/${OUTPUTCONFIG}/)
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG_UPPER} ${PROJECT_BINARY_DIR}/lib/${OUTPUTCONFIG}/)
        set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG_UPPER} ${PROJECT_BINARY_DIR}/lib/${OUTPUTCONFIG}/)
    endforeach(OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES)
else()
    #make sure there is a build type
    if(NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE Debug)
    endif()
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

hunter_add_package(glm)
find_package(glm CONFIG REQUIRED)
hunter_add_package(RapidJSON)
find_package(RapidJSON CONFIG REQUIRED)
hunter_add_package(gflags)
find_package(gflags CONFIG REQUIRED)
hunter_add_package(glog)
find_package(glog CONFIG REQUIRED)
hunter_add_package(HastyNoise)
find_package(HastyNoise CONFIG REQUIRED)
hunter_add_package(CreateLaunchers)
find_package(CreateLaunchers CONFIG REQUIRED)

set(VOXIGEN_BOOST_FILESYSTEM 0)
set(VOXIGEN_STD_FILESYSTEM 1)
set(VOXIGEN_EXPERIMENTAL_FILESYSTEM 2)
set(VOXIGEN_USE_FILESYSTEM ${VOXIGEN_BOOST_FILESYSTEM})

#find what filesystem to use
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    add_definitions(-DNOMINMAX)

    if(MSVC_VERSION GREATER_EQUAL 1900)
        set(VOXIGEN_USE_FILESYSTEM ${VOXIGEN_EXPERIMENTAL_FILESYSTEM})
        if(MSVC_VERSION GREATER 1900)
            include(CheckCXXCompilerFlag)

            CHECK_CXX_COMPILER_FLAG("/std:c++17" _cpp_17_flag_supported)
            if(_cpp_17_flag_supported)
                add_compile_options("/std:c++17")
                set(VOXIGEN_USE_FILESYSTEM ${VOXIGEN_STD_FILESYSTEM})
            endif()
        endif()
    endif()
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang") 
    if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 7.0)
        set(VOXIGEN_USE_FILESYSTEM ${VOXIGEN_STD_FILESYSTEM})
    endif()
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
    set(CMAKE_CXX_STANDARD 14)
    set(VOXIGEN_USE_FILESYSTEM ${VOXIGEN_BOOST_FILESYSTEM})
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(VOXIGEN_USE_FILESYSTEM ${VOXIGEN_EXPERIMENTAL_FILESYSTEM})
    if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 8.0)
        set(VOXIGEN_USE_FILESYSTEM ${VOXIGEN_STD_FILESYSTEM})
    endif()
endif()

if(VOXIGEN_USE_FILESYSTEM EQUAL ${VOXIGEN_BOOST_FILESYSTEM})
    hunter_add_package(Boost COMPONENTS filesystem)
    find_package(Boost REQUIRED COMPONENTS filesystem)
endif()

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

add_subdirectory(opengl_util)
set_target_properties(opengl_util PROPERTIES FOLDER Libraries)
set(GENERIC_DISABLE_ATTRIBUTE ON CACHE BOOL "Disable support for attributes" FORCE)
add_subdirectory(generic)
set_target_properties(generic PROPERTIES FOLDER Libraries)
add_subdirectory(dt)
set_target_properties(dt PROPERTIES FOLDER Libraries)

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
    include/voxigen/filesystem.h
    include/voxigen/generator.h
    include/voxigen/gridDescriptors.h
    include/voxigen/handleState.h
    include/voxigen/heightmapMeshBuilder.h
    include/voxigen/jsonSerializer.h
    include/voxigen/nativeGL.h
    include/voxigen/noise.h
    include/voxigen/object.h
    include/voxigen/processQueue.h
    include/voxigen/regular2DGrid.h
    include/voxigen/regularGrid.h
    include/voxigen/region.h
    include/voxigen/regionHandle.h
    include/voxigen/search.h
    include/voxigen/simpleCamera.h
    include/voxigen/simpleFilesystem.h
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
    src/biome.cpp
    src/chunkMesh.cpp
    src/coords.cpp
    src/entity.cpp
    src/equiRectWorldGenerator.cpp
    src/gridDescriptors.cpp
    src/jsonSerializer.cpp
    src/noise.cpp
    src/object.cpp
    src/regular2DGrid.cpp
    src/simpleCamera.cpp
    src/nativeGL.cpp
    src/noise.cpp
    src/simpleFilesystem.cpp
)


set(voxigen_public_libraries
    HastyNoise::hastyNoise
    gflags
    glog::glog
    generic
    dt
)

set(voxigen_libraries ${voxigen_public_libraries}
    opengl_util
    glm
    RapidJSON::rapidjson
)

if(VOXIGEN_USE_FILESYSTEM EQUAL ${VOXIGEN_BOOST_FILESYSTEM})
    set(voxigen_libraries ${voxigen_libraries}
        Boost::filesystem
    )
endif()

if(VOXIGEN_RENDERING)
    hunter_add_package(glbinding)
    find_package(glbinding CONFIG REQUIRED)
#    hunter_add_package(globjects)
#    find_package(globjects CONFIG REQUIRED)

    set(voxigen_headers ${voxigen_headers}
        include/voxigen/initOpengl.h
        include/voxigen/glm_point.h
        include/voxigen/renderAction.h
        include/voxigen/renderCube.h
        include/voxigen/renderPrepThread.h
        include/voxigen/simpleChunkRenderer.h
        include/voxigen/simpleRegionRenderer.h
        include/voxigen/simpleRenderer.h
        include/voxigen/simpleShapes.h
    )

    set(voxigen_sources ${voxigen_sources}
        src/initOpengl.cpp
    )

    set(voxigen_libraries ${voxigen_libraries} glbinding::glbinding) #globjects::globjects)
endif()

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
        src/blendMethod.cpp
        src/chunkTextureMesh.cpp
        src/layerMethod.cpp
        src/textureAtlas.cpp
        src/texturePack.cpp
    )

    set(voxigen_libraries ${voxigen_libraries}
        imglib
    )
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

target_compile_definitions(voxigen PUBLIC "-DVOXIGEN_USE_FILESYSTEM=${VOXIGEN_USE_FILESYSTEM}")
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
    if(VOXIGEN_USE_FILESYSTEM GREATER ${VOXIGEN_BOOST_FILESYSTEM})
        target_compile_options(voxigen PUBLIC "-lstdc++fs")
        target_link_libraries(voxigen stdc++fs)
    endif()
endif()

source_group("Source Files" FILES ${voxigen_inline})
include_directories(${CMAKE_CURRENT_LIST_DIR}/include)

target_link_libraries(voxigen ${voxigen_libraries})


if(VOXIGEN_TESTAPP OR VOXIGEN_MAPGENAPP)
    hunter_add_package(imgui)
    find_package(imgui CONFIG REQUIRED)
    hunter_add_package(glfw)
    find_package(glfw3 REQUIRED)

    include(CreateLaunchers)
endif()

##test app
if(VOXIGEN_TESTAPP)
    set(testApp_sources
        testApp/debugScreen.h
        testApp/debugScreen.cpp
        testApp/main.cpp
        testApp/marker.h
        testApp/marker.cpp
        testApp/initGlew.h
        testApp/initGlew.cpp
        testApp/world.h
        testApp/renderingOptions.h
    )
    
    add_executable(testApp ${testApp_sources})

    target_include_directories(testApp PRIVATE ${CMAKE_CURRENT_LIST_DIR})
    target_link_libraries(testApp voxigen glfw imgui::imgui)
    
    set(testApp_public_libraries ${voxigen_public_libraries} glfw)

    set(testApp_vs_enviroment_dir)
    foreach(target ${testApp_public_libraries})
        list(APPEND testApp_vs_enviroment_dir "$<TARGET_FILE_DIR:${target}>")
    endforeach()
    set(testApp_vs_working_dir ${CMAKE_CURRENT_LIST_DIR})
    
    create_target_launcher(testApp
        RUNTIME_LIBRARY_DIRS "${testApp_vs_enviroment_dir}"
        WORKING_DIRECTORY "${testApp_vs_working_dir}"
    )
endif()

if(VOXIGEN_MAPGENAPP)
    
    set(mapgenApp_sources
        mapgenApp/colorMap.h
        mapgenApp/colorMap.cpp
        mapgenApp/main.cpp
        mapgenApp/randomcolor.h
        mapgenApp/randomcolor.cpp
        mapgenApp/mapgen.h
        mapgenApp/mapgen.cpp
        testApp/initGlew.h
        testApp/initGlew.cpp
        testApp/world.h
    )
    
    add_executable(mapgenApp ${mapgenApp_sources})

    target_include_directories(mapgenApp PRIVATE ${CMAKE_CURRENT_LIST_DIR})
    target_link_libraries(mapgenApp voxigen glfw imgui::imgui)

    set(mapgenApp_public_libraries ${voxigen_public_libraries} glfw)
    set(mapgenApp_vs_enviroment_dir)
    foreach(target ${mapgenApp_public_libraries})
        list(APPEND mapgenApp_vs_enviroment_dir "$<TARGET_FILE_DIR:${target}>")
    endforeach()
    set(mapgenApp_vs_working_dir ${CMAKE_CURRENT_LIST_DIR})
    
    create_target_launcher(mapgenApp
        RUNTIME_LIBRARY_DIRS "${mapgenApp_vs_enviroment_dir}"
        WORKING_DIRECTORY "${mapgenApp_vs_working_dir}"
    )
endif()

