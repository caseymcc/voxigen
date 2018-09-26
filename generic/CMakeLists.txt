project(generic)

#remove min and max macro declares from windows builds, causes issues with std::min/max
add_definitions(-DNOMINMAX)

hunter_add_package(Boost )
find_package(Boost CONFIG)

set(generic_sources
    attribute.cpp
    attributeContainer.cpp
    attributes.cpp
    jsonSerializer.cpp
    errorMsg.cpp
)

set(generic_headers
    generic/generic_export.h
    generic/attribute.h
    generic/attributeContainer.h
    generic/attributes.h
    generic/callIf.h
    generic/errorMsg.h
    generic/jsonSerializer.h
    generic/serializer.h
    generic/types.h
)

set(generic_libraries
    Boost::boost
)

add_library(generic STATIC
        ${generic_sources}
        ${generic_headers}
    )

target_link_libraries(generic ${generic_libraries})
target_include_directories(generic PUBLIC ${CMAKE_CURRENT_LIST_DIR})
