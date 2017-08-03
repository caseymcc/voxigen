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
hunter_add_package(glew)
find_package(glew CONFIG REQUIRED)
hunter_add_package(RapidJSON)
find_package(RapidJSON CONFIG REQUIRED)
hunter_add_package(Boost COMPONENTS filesystem system)
find_package(Boost REQUIRED COMPONENTS filesystem system)

add_subdirectory(FastNoiseSIMD)
add_subdirectory(opengl_util)

#hack for Urho3D
set(URHO3D_INCLUDES ${URHO3D_DIR}/buildx64/include)
set(URHO3D_LIBS 
    optimized ${URHO3D_DIR}/buildx64/lib/Urho3D.lib 
    debug ${URHO3D_DIR}/buildx64/lib/Urho3D_d.lib
    optimized ${URHO3D_DIR}/buildx64/Source/ThirdParty/SDL/Release/SDL.lib
    debug ${URHO3D_DIR}/buildx64/Source/ThirdParty/SDL/Debug/SDL.lib
)
set(URHO3D_3RDPARTY_INCLUDE ${URHO3D_INCLUDES}/Urho3D/ThirdParty)

#hack for bullet
set(BULLET_INCLUDES ${URHO3D_DIR}/Source/ThirdParty/Bullet)
set(BULLET_LIBS optimized ${URHO3D_DIR}/buildx64/Source/ThirdParty/Bullet/Release/Bullet.lib debug ${URHO3D_DIR}/buildx64/Source/ThirdParty/Bullet/Debug/Bullet.lib)

#hack for libnoise, need to add to hunter
set(LIBNOISE_INCLUDES ${LIBNOISE_ROOT_DIR}/src)
set(LIBNOISE_LIBS optimized ${LIBNOISE_ROOT_DIR}/_buildx64/src/Release/noise.lib debug ${LIBNOISE_ROOT_DIR}/_buildx64/src/Debug/noise.lib)

if(MSVC)
    add_definitions(-DNOMINMAX)
endif()

set(voxigen_headers
    include/voxigen/biome.h
    include/voxigen/boundingBox.h
    include/voxigen/cell.h
    include/voxigen/chunk.h
    include/voxigen/chunkHandle.h
    include/voxigen/classFactory.h
    include/voxigen/coords.h
    include/voxigen/dataHandler.h
    include/voxigen/dataStore.h
    include/voxigen/defines.h
    include/voxigen/entity.h
    include/voxigen/equiRectWorldGenerator.h
    include/voxigen/generator.h
    include/voxigen/gridDescriptors.h
    include/voxigen/initGlew.h
    include/voxigen/object.h
    include/voxigen/regularGrid.h
    include/voxigen/segment.h
    include/voxigen/segmentHandle.h
    include/voxigen/simpleCamera.h
    include/voxigen/simpleChunkRenderer.h
    include/voxigen/simpleRenderer.h
    include/voxigen/simpleShapes.h
)

set(voxigen_sources
    src/data/biome.cpp
    src/data/entity.cpp
    src/data/gridDescriptors.cpp
    src/data/object.cpp
    src/rendering/initGlew.cpp
    src/rendering/simpleCamera.cpp
    src/utils/coords.cpp
)

set(voxigen_libraries
    glm
    ${BULLET_LIBS}
    ${LIBNOISE_LIBS}
    opengl_util
    fastNoise
    glew::glew
    RapidJSON::rapidjson
    Boost::filesystem
    Boost::system
)

add_library(voxigen SHARED
    ${voxigen_headers}
    ${voxigen_sources}
)


#include_directories(${GLM_INCLUDES})
#include_directories(${GLFW_INCLUDES})
include_directories(${LIBNOISE_INCLUDES})
#include_directories(${BULLET_INCLUDES})
include_directories(${POLYVOX_DIR}/include)

include_directories(${CMAKE_CURRENT_LIST_DIR}/include)

target_link_libraries(voxigen ${voxigen_libraries})

#test app
set(testApp_sources
    testApp/main.cpp
)

add_executable(testApp ${testApp_sources})
target_link_libraries(testApp voxigen glfw)

#urho3D test app
include_directories(${URHO3D_INCLUDES})
include_directories(${URHO3D_3RDPARTY_INCLUDE}) 

add_definitions(-DURHO3D_WIN32_CONSOLE)

set(Urho3DApp_headers
    urho3DApp/urho3DApp.h
)

set(Urho3DApp_sources
    urho3DApp/urho3DApp.cpp
)

add_executable(Urho3DApp ${Urho3DApp_headers} ${Urho3DApp_sources})
target_link_libraries(Urho3DApp ${URHO3D_LIBS} voxigen Winmm.lib version.lib Imm32.lib)
