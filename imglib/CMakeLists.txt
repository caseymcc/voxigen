cmake_minimum_required(VERSION 3.3)
project(imglib)

#remove min and max macro declares from windows builds, causes issues with std::min/max
add_definitions(-DNOMINMAX)

option(IMGLIB_USE_BOOST_FILESYSTEM "Turn on/off support boost filesystem" OFF)
option(IMGLIB_USE_COMPILETIME_TYPE "Turn on/off support for compile time type informat" ON)
option(IMGLIB_USE_PNG "Turn on/off support for PNG" ON)
option(IMGLIB_USE_EIGEN "Turn on/off support for eigen library" ON)
option(IMGLIB_USE_OPENCL "Turn on/off support for OpenCl" ON)
option(IMGLIB_USE_OPENCL_CPP "Turn on/off support for OpenCl CPP classes" ON)
option(IMGLIB_USE_CUDA "Turn on/off support for CUDA" OFF)
option(IMGLIB_BUILD_TEST "Turns on test application" ON)

set(imglib_headers
    imglib/colorConversion.h
    imglib/copyTo.h
    imglib/image.h
    imglib/imageIo.h
    imglib/match.h
    imglib/ppm.h
    imglib/simpleImage.h
    imglib/transform.h
)

set(imglib_sources
    colorConversion.cpp
    image.cpp
    match.cpp
    ppm.cpp
    simpleImage.cpp
    transform.cpp
)

set(imglib_libraries 
    Boost::system 
    Boost::filesystem
)

if(IMGLIB_USE_BOOST_FILESYSTEM)
    hunter_add_package(Boost COMPONENTS system filesystem)
    find_package(Boost CONFIG REQUIRED system filesystem)

    set(imglib_libraries ${imglib_libraries} Boost::system Boost::filesystem)
endif()


if(IMGLIB_USE_COMPILETIME_TYPE)
    hunter_add_package(ctti)
    find_package(ctti REQUIRED CONFIG)

    set(imglib_libraries ${imglib_libraries} ctti)
endif()

if(IMGLIB_USE_EIGEN)
    hunter_add_package(Eigen)
    find_package(Eigen3 REQUIRED)
#    add_definitions(-DIMGLIB_EIGEN)

    set(imglib_libraries ${imglib_libraries} Eigen3::Eigen)
endif()

message(STATUS "IMGLIB_USE_PNG ${IMGLIB_USE_PNG}")
if(IMGLIB_USE_PNG)
    hunter_add_package(PNG)
    find_package(PNG CONFIG REQUIRED)
#    add_definitions(-DIMGLIB_PNG)

    set(imglib_headers ${imglib_headers}
        imglib/png.h
    )

    set(imglib_sources ${imglib_sources}
        png.cpp
    )

    set(imglib_libraries ${imglib_libraries} PNG::png)
endif()

if(IMGLIB_USE_OPENCL)
    hunter_add_package(OpenCL)
    find_package(OpenCL CONFIG REQUIRED)
#    add_definitions(-DIMGLIB_OPENCL)
#    if(IMGLIB_USE_OPENCL_CPP)
#        add_definitions(-DIMGLIB_OPENCL_CPP)
#    endif()

    set(imglib_headers ${imglib_headers}
        imglib/colorConversion_cl.h
        imglib/kernels_cl.h
        imglib/openCLImage.h
        imglib/openCLImageBuffer.h
        imglib/operators_cl.h
        imglib/transform_cl.h
        imglib/utils_cl.h
    )

    set(imglib_sources ${imglib_sources}
        colorConversion_cl.cpp
        kernels_cl.cpp
        openCLImage.cpp
        openCLImageBuffer.cpp
        operators_cl.cpp
        transform_cl.cpp
        utils_cl.cpp
    )

    set(imglib_libraries ${imglib_libraries} OpenCL::OpenCL)

    include(EmbedFile)
    set(embedFiles
        kernels/colorConversion.cl
        kernels/operators.cl
        kernels/transform.cl
        kernels/utils.cl
    )
    embed_files(embededFiles "${embedFiles}" "imglib::cl")

endif()

add_library(imglib
    ${imglib_sources}
    ${imglib_headers}
    ${embededFiles}
)

if(NOT BUILD_SHARED_LIBS)
    target_compile_definitions(imglib PUBLIC -DIMGLIB_STATIC_DEFINE)
endif()


include(GenerateExportHeader)
generate_export_header(imglib)

target_link_libraries(imglib ${imglib_libraries})

target_include_directories(imglib PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(imglib PUBLIC ${PROJECT_BINARY_DIR}) #export files will be here

if(IMGLIB_USE_BOOST_FILESYSTEM)
    target_compile_definitions(imglib PUBLIC -DIMGLIB_USE_BOOST_FILESYSTEM)
endif()

if(IMGLIB_USE_COMPILETIME_TYPE)
    target_compile_definitions(imglib PUBLIC -DIMGLIB_USE_COMPILETIME_TYPE)
endif()

if(IMGLIB_USE_OPENCL)
    target_compile_definitions(imglib PUBLIC -DIMGLIB_OPENCL)
    if(IMGLIB_USE_OPENCL_CPP)
        target_compile_definitions(imglib PUBLIC -DIMGLIB_OPENCL_CPP)
    endif()

    get_target_property(OPENCL_BIN_DIR OpenCL::OpenCL IMPORTED_LOCATION_DEBUG)
    get_filename_component(OPENCL_BIN_DIR ${OPENCL_BIN_DIR} DIRECTORY)
endif()

if(IMGLIB_USE_EIGEN)
    target_compile_definitions(imglib PUBLIC -DIMGLIB_EIGEN)
endif()

if(IMGLIB_USE_PNG)
    target_compile_definitions(imglib PUBLIC -DIMGLIB_PNG)
endif()

#build test application
if(IMGLIB_BUILD_TEST)
    set(imglib_benchmark_headers
    )
    
    set(imglib_benchmark_sources
        test/benchmark.cpp
    )

    if(IMGLIB_USE_OPENCL)
        set(imglib_benchmark_headers ${imglib_benchmark_headers}
            test/benchmarkOpenCL.h
        )

        set(imglib_benchmark_sources ${imglib_benchmark_sources}
            test/benchmarkOpenCL.cpp
        )
    endif()

    add_executable(imglibBenchmark WIN32
        ${imglib_benchmark_headers}
        ${imglib_benchmark_sources}
    )

    target_link_libraries(imglibBenchmark imglib gflags-static opencl_util)

    set_target_properties(imglibBenchmark PROPERTIES FOLDER "benchmarks")

    if(WIN32)
        foreach(CONFIG_TYPE ${CMAKE_CONFIGURATION_TYPES} )
            string( TOUPPER ${CONFIG_TYPE} CONFIG_TYPE_UPPER )
            set_target_properties(imglibBenchmark PROPERTIES LINK_FLAGS_${CONFIG_TYPE_UPPER} "/SUBSYSTEM:CONSOLE")
            set_target_properties(imglibBenchmark PROPERTIES COMPILE_DEFINITIONS_${CONFIG_TYPE_UPPER} "_CONSOLE")
        endforeach()
    endif()

    set(RUNTIME_LIBS "")

    if(IMGLIB_USE_OPENCL)
        set(RUNTIME_LIBS ${RUNTIME_LIBS} ${OPENCL_BIN_DIR})
    endif()

    #message(FATAL_ERROR "RUNTIME_LIBS: ${RUNTIME_LIBS}")
    create_target_launcher(imglibBenchmark
        RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBS}
    )
endif()

