cmake_minimum_required (VERSION 3.0)

option(OVERIDE_HUNTER_RAPIDJASON "Overides hunter for RapidJson" OFF)
option(VOXIGEN_RENDERING "Enable simple rendering" ON)
option(VOXIGEN_TEXTURE "Setup texture classes" ON)
option(VOXIGEN_TESTAPP "Build test app" ON)
option(VOXIGEN_MAPGENAPP "Build mapgen app" ON)
option(VOXIGEN_INSTALL_LIBS "Build mapgen app" OFF)

message(STATUS "VOXIGEN_TESTAPP: ${VOXIGEN_TESTAPP}")
if(VOXIGEN_TESTAPP)
    #requires texture and rendering
    set(VOXIGEN_TEXTURE ON CACHE BOOL "Setup texture classes" FORCE)
    set(VOXIGEN_RENDERING ON CACHE BOOL "Enable simple rendering" FORCE)

    message(STATUS "VOXIGEN_RENDERING: ${VOXIGEN_RENDERING}")
endif()

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

project(voxigen VERSION 0.0.1)
set(library_targets)

include(${CMAKE_CURRENT_LIST_DIR}/cmake/targetInfo.cmake)

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

#overide hastnoise
message(STATUS "VOXIGEN_OVERIDE_HASTY ${VOXIGEN_OVERIDE_HASTY}")
if(VOXIGEN_OVERIDE_HASTY AND NOT VOXIGEN_OVERIDE_HASTY STREQUAL "")
    add_library(HastyNoise::hastyNoise SHARED IMPORTED)

    set_target_properties(HastyNoise::hastyNoise PROPERTIES
        INTERFACE_COMPILE_OPTIONS "/bigobj;/std:c++17"
        INTERFACE_INCLUDE_DIRECTORIES "${VOXIGEN_OVERIDE_HASTY}"
        INTERFACE_LINK_DIRECTORIES "${VOXIGEN_OVERIDE_HASTY}/build/pdb-files/Debug"
    )

    set_property(TARGET HastyNoise::hastyNoise APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
    set_target_properties(HastyNoise::hastyNoise PROPERTIES
      IMPORTED_IMPLIB_RELEASE "${VOXIGEN_OVERIDE_HASTY}/build/Release/hastyNoise.lib"
      IMPORTED_LOCATION_RELEASE "${VOXIGEN_OVERIDE_HASTY}/build/Release/hastyNoise.dll"
    )
    set_property(TARGET HastyNoise::hastyNoise APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
    set_target_properties(HastyNoise::hastyNoise PROPERTIES
      IMPORTED_IMPLIB_DEBUG "${VOXIGEN_OVERIDE_HASTY}/build/Debug/hastyNoised.lib"
      IMPORTED_LOCATION_DEBUG "${VOXIGEN_OVERIDE_HASTY}/build/Debug/hastyNoised.dll"
      )
else()
    hunter_add_package(HastyNoise)
    find_package(HastyNoise CONFIG REQUIRED)
endif()

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

if(VOXIGEN_RENDERING)
    add_subdirectory(opengl_util)
    set_target_properties(opengl_util PROPERTIES FOLDER Libraries)
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

set(GENERIC_DISABLE_ATTRIBUTE ON CACHE BOOL "Disable support for attributes" FORCE)
add_subdirectory(generic)
set_target_properties(generic PROPERTIES FOLDER Libraries)
#add_subdirectory(dt)
#set_target_properties(dt PROPERTIES FOLDER Libraries)


set(voxigen_fileio
    include/voxigen/fileio/filesystem.h
    include/voxigen/fileio/jsonSerializer.h
    src/fileio/jsonSerializer.cpp
    include/voxigen/fileio/log.h
    include/voxigen/fileio/simpleFilesystem.h
    src/fileio/simpleFilesystem.cpp
)
source_group("fileio" FILES ${voxigen_fileio})

set(voxigen_generators
    include/voxigen/generators/biome.h
    src/generators/biome.cpp
    include/voxigen/generators/equiRectWorldGenerator.h
    include/voxigen/generators/equiRectWorldGenerator.inl
    src/generators/equiRectWorldGenerator.cpp
    include/voxigen/generators/generator.h
)
source_group("generators" FILES ${voxigen_generators})

set(voxigen_maths
    include/voxigen/maths/boundingBox.h
    include/voxigen/maths/coords.h
    src/maths/coords.cpp
)
source_group("maths" FILES ${voxigen_maths})

set(voxigen_meshbuilders
    include/voxigen/meshbuilders/cubicMeshBuilder.h
    include/voxigen/meshbuilders/heightmapMeshBuilder.h
)
source_group("meshbuilders" FILES ${voxigen_meshbuilders})

set(voxigen_meshes
    include/voxigen/meshes/chunkMesh.h
    src/meshes/chunkMesh.cpp
    include/voxigen/meshes/faces.h
    include/voxigen/meshes/heightMap.h
    include/voxigen/meshes/meshBuffer.h
)
source_group("meshes" FILES ${voxigen_meshes})

set(voxigen_volume
    include/voxigen/volume/activeVolume.h
    include/voxigen/volume/activeVolume.inl
    include/voxigen/volume/cell.h
    include/voxigen/volume/chunk.h
    include/voxigen/volume/chunkFunctions.h
    include/voxigen/volume/chunkHandle.h
    include/voxigen/volume/chunkHandle.inl
    include/voxigen/volume/dataHandler.h
    include/voxigen/volume/dataStore.h
    include/voxigen/volume/gridDescriptors.h
    src/volume/gridDescriptors.cpp
    include/voxigen/volume/gridFunctions.h
    include/voxigen/volume/handleState.h
    include/voxigen/volume/region.h
    include/voxigen/volume/regionHandle.h
    include/voxigen/volume/regionHandle.inl
    include/voxigen/volume/regular2DGrid.h
    src/volume/regular2DGrid.cpp
    include/voxigen/volume/regularGrid.h
)
source_group("volume" FILES ${voxigen_volume})

set(voxigen_source
    include/voxigen/classFactory.h
    include/voxigen/defines.h
    include/voxigen/entity.h
    src/entity.cpp
    include/voxigen/noise.h
    src/noise.cpp
    include/voxigen/object.h
    src/object.cpp
    include/voxigen/processQueue.h
    include/voxigen/processQueue.inl
    include/voxigen/search.h
    include/voxigen/simpleCamera.h
    src/simpleCamera.cpp
    include/voxigen/voxigen_export.h
)
source_group("source" FILES ${voxigen_source})

set(voxigen_source ${voxigen_source}
    ${voxigen_fileio}
    ${voxigen_generators}
    ${voxigen_maths}
    ${voxigen_meshbuilders}
    ${voxigen_meshes}
    ${voxigen_volume}
)

#force debug to use HastyNoise release
set_target_properties(HastyNoise::hastyNoise PROPERTIES
    MAP_IMPORTED_CONFIG_RELEASE RELEASE
    MAP_IMPORTED_CONFIG_DEBUG RELEASE
    MAP_IMPORTED_CONFIG_RELWITHDEBUGINFO RELEASE
)

set(voxigen_public_libraries
    HastyNoise::hastyNoise
    gflags
    glog::glog
    generic
#    dt
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

    set(voxigen_rendering
        include/voxigen/rendering/initOpengl.h
        src/rendering/initOpengl.cpp
        include/voxigen/rendering/nativeGL.h
        src/rendering/nativeGL.cpp
        include/voxigen/rendering/openglDebug.h
        src/rendering/openglDebug.cpp
        include/voxigen/rendering/renderAction.h
        include/voxigen/rendering/renderPrepThread.h
        include/voxigen/rendering/renderPrepThread.inl
        include/voxigen/rendering/simpleChunkRenderer.h
        include/voxigen/rendering/simpleChunkRenderer.inl
        include/voxigen/rendering/simpleRegionRenderer.h
        include/voxigen/rendering/simpleRegionRenderer.inl
        include/voxigen/rendering/simpleRenderer.h
        include/voxigen/rendering/simpleShapes.h
        include/voxigen/rendering/simpleRenderer.inl
    )
    source_group("rendering" FILES ${voxigen_rendering})

    set(voxigen_rendering_shaders
        resources/shaders/chunk_frag.glsl
        resources/shaders/chunk_vert.glsl
        resources/shaders/chunkOutline_frag.glsl
        resources/shaders/chunkOutline_vert.glsl
        resources/shaders/region_frag.glsl
        resources/shaders/region_vert.glsl
        resources/shaders/regionOutline_frag.glsl
        resources/shaders/regionOutline_vert.glsl
    )
    source_group("rendering\\shaders" FILES ${voxigen_rendering_shaders})
    set_source_files_properties(${voxigen_rendering_shaders} PROPERTIES HEADER_FILE_ONLY TRUE)

    set(voxigen_rendering_maths
        include/voxigen/maths/glm_point.h
    )
    source_group("maths" FILES ${voxigen_rendering_maths})

    set(voxigen_source ${voxigen_source}
        ${voxigen_rendering}
        ${voxigen_rendering_shaders}
        ${voxigen_rendering_maths}
    )

    set(voxigen_libraries ${voxigen_libraries} 
        glbinding::glbinding
        #glbinding::glbinding-aux
        #globjects::globjects
    )
endif()

if(VOXIGEN_TEXTURE)
    set(voxigen_texturing
        include/voxigen/texturing/blendMethod.h
        src/texturing/blendMethod.cpp
        include/voxigen/texturing/layerMethod.h
        src/texturing/layerMethod.cpp
#        include/voxigen/texturing/textureArray.h
#        src/texturing/textureArray.cpp
        include/voxigen/texturing/textureAtlas.h
        src/texturing/textureAtlas.cpp
        include/voxigen/texturing/texturePack.h
        src/texturing/texturePack.cpp
    )
    source_group("texturing" FILES ${voxigen_texturing})

    set(voxigen_texturing_meshes
        include/voxigen/meshes/chunkTextureMesh.h
        src/meshes/chunkTextureMesh.cpp
        include/voxigen/meshes/texturedMesh.h
    )
    source_group("meshes" FILES ${voxigen_texturing_meshes})

    set(voxigen_source ${voxigen_source}
        ${voxigen_texturing}
        ${voxigen_texturing_meshes}
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

add_library(voxigen
    ${voxigen_source}
)
list(APPEND library_targets voxigen)

if(NOT BUILD_SHARED_LIBS)
    target_compile_definitions(voxigen PUBLIC -DVOXIGEN_STATIC_DEFINE -DGLBINDING_STATIC_DEFINE)
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
    
    set(testApp_libraries
        voxigen 
        glfw 
        imgui::imgui
    )

    add_executable(testApp ${testApp_sources})
    list(APPEND library_targets testApp)

    target_include_directories(testApp PRIVATE ${CMAKE_CURRENT_LIST_DIR})
    target_link_libraries(testApp ${testApp_libraries})
    
#    set(testApp_public_libraries ${voxigen_public_libraries} glfw) #add hasty manually to bin directory

    set(testApp_vs_enviroment_dir)
    get_target_link_directories(testApp_vs_enviroment_dir ${testApp_libraries})
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
    list(APPEND library_targets mapgenApp)

    target_include_directories(mapgenApp PRIVATE ${CMAKE_CURRENT_LIST_DIR})
    target_link_libraries(mapgenApp voxigen glfw imgui::imgui)

    set(mapgenApp_public_libraries ${voxigen_public_libraries} glfw) #add hasty manually to bin directory
    set(mapgenApp_vs_enviroment_dir)
    get_target_link_directories(mapgenApp_vs_enviroment_dir ${mapgenApp_public_libraries})
    set(mapgenApp_vs_working_dir ${CMAKE_CURRENT_LIST_DIR})
    
    create_target_launcher(mapgenApp
        RUNTIME_LIBRARY_DIRS "${mapgenApp_vs_enviroment_dir}"
        WORKING_DIRECTORY "${mapgenApp_vs_working_dir}"
    )
endif()


##installer
#include(GNUInstallDirs) 
#
## Layout. This works for all platforms:
##   * <prefix>/lib*/cmake/<PROJECT-NAME>
##   * <prefix>/lib*/
##   * <prefix>/include/
#set(config_install_dir "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}")
#set(generated_dir "${CMAKE_CURRENT_BINARY_DIR}/generated")
#
## Configuration
#set(version_config "${generated_dir}/${PROJECT_NAME}ConfigVersion.cmake")
#set(project_config "${generated_dir}/${PROJECT_NAME}Config.cmake")
#set(TARGETS_EXPORT_NAME "${PROJECT_NAME}Targets")
#set(namespace "${PROJECT_NAME}::")
#
## Include module with fuction 'write_basic_package_version_file'
#include(CMakePackageConfigHelpers)
#
## Configure '<PROJECT-NAME>ConfigVersion.cmake'
## Use:
##   * PROJECT_VERSION
#write_basic_package_version_file(
#    "${version_config}" COMPATIBILITY SameMajorVersion
#)
#
## Configure '<PROJECT-NAME>Config.cmake'
## Use variables:
##   * TARGETS_EXPORT_NAME
##   * PROJECT_NAME
#configure_package_config_file(
#    "cmake/Config.cmake.in"
#    "${project_config}"
#    INSTALL_DESTINATION "${config_install_dir}"
#)
#
## Targets:
##   * <prefix>/lib/libname.a
##   * header location after install: <prefix>/include/${PROJECT_NAME}/include.hpp
##   * headers can be included by C++ code `#include <${PROJECT_NAME}/include.hpp>`
#message(STATUS "library_targets: ${library_targets}")
#install(
#    TARGETS ${library_targets}
#    EXPORT "${TARGETS_EXPORT_NAME}"
#    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
#    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
#    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
#    INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
#)
#
## Headers:
#install(
#    FILES ${voxigen_headers}
#    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}"
#)
#
## Config
##   * <prefix>/lib/cmake/${PROJECT_NAME}/${PROJECT_NAME}Config.cmake
##   * <prefix>/lib/cmake/${PROJECT_NAME}/${PROJECT_NAME}ConfigVersion.cmake
#install(
#    FILES "${project_config}" "${version_config}"
#    DESTINATION "${config_install_dir}"
#)
#
## Config
##   * <prefix>/lib/cmake/${PROJECT_NAME}/${PROJECT_NAME}Targets.cmake
#install(
#    EXPORT "${TARGETS_EXPORT_NAME}"
#    NAMESPACE "${namespace}"
#    DESTINATION "${config_install_dir}"
#)
#
#if(MSVC)
#    #install pdbs
#    get_cmake_property(is_multi GENERATOR_IS_MULTI_CONFIG)
#
#    if(is_multi)
#      set(config_suffix "$<CONFIG>")
#    else()
#      set(config_suffix "")
#    endif() 
#
#    if(BUILD_SHARED_LIBS)
#      set(pdb_dst ${CMAKE_INSTALL_BINDIR})
#    else()
#      set(pdb_dst ${CMAKE_INSTALL_LIBDIR})
#    endif() 
#
#    install(
#        DIRECTORY "${pdb_output_dir}/${config_suffix}/"
#        DESTINATION ${pdb_dst}
#    )
#endif()

#install any targets that voxigen uses
#if(VOXIGEN_INSTALL_LIBS)
#    set(voxigen_link_files)
#    set(voxigen_link_libs)
#    get_target_link_libraries(voxigen_link_files voxigen_link_libs voxigen)
#    message(STATUS "voxigen_link_files: ${voxigen_link_files}")
#    message(STATUS "voxigen_link_libs: ${voxigen_link_libs}")
#
#    foreach(target_name ${voxigen_link_files})
#        add_custom_command(TARGET voxigen
#            POST_BUILD
#            COMMAND COMMAND copy "$<TARGET_FILE:${target_name}>" "${CMAKE_CURRENT_BINARY_DIR}/$<TARGET_FILE_NAME:${target_name}>"
#            COMMENT "Copying ${target_name}\n"
#        )
#    endforeach()
#
#    foreach(target_name ${voxigen_link_libs})
#        add_custom_command(TARGET voxigen
#            POST_BUILD
#            COMMAND COMMAND copy "$<TARGET_LINKER_FILE:${target_name}>" "${CMAKE_CURRENT_BINARY_DIR}/$<TARGET_LINKER_FILE_NAME:${target_name}>"
#            COMMENT "Copying ${target_name}\n"
#        )
#    endforeach()
#endif()


