add_library(protocol STATIC)

target_sources(protocol
    PRIVATE
        interface.cpp
        error.cpp
    PUBLIC
        FILE_SET HEADERS BASE_DIRS ${PROJECT_SOURCE_DIR}
        FILES
            interface.hpp
)

target_link_libraries(protocol Boost::json)
