CMAKE_MINIMUM_REQUIRED(VERSION 3.19)

# Read project version from file
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/package.json" PACKAGE_JSON)
string(JSON CMAKE_PROJECT_NAME GET "${PACKAGE_JSON}" "name")
string(JSON CMAKE_PROJECT_VERSION GET "${PACKAGE_JSON}" "version")
string(JSON CMAKE_PROJECT_HOMEPAGE_URL GET "${PACKAGE_JSON}" "homepage")
string(JSON CMAKE_PROJECT_AUTHOR GET "${PACKAGE_JSON}" "author")
string(JSON CMAKE_PROJECT_DESCRIPTION GET "${PACKAGE_JSON}" "description")

PROJECT(${CMAKE_PROJECT_NAME}
    LANGUAGES CXX
    VERSION ${CMAKE_PROJECT_VERSION}
    HOMEPAGE_URL "${CMAKE_PROJECT_HOMEPAGE_URL}"
    DESCRIPTION "${CMAKE_PROJECT_DESCRIPTION}")

# Configure project resources file
IF (WIN32)
    SET(RESOURCESFILE "${CMAKE_BINARY_DIR}/resources.rc")
    configure_file(resources/resources.in.rc ${RESOURCESFILE})
ELSE()
    SET(RESOURCESFILE "")
ENDIF()

# Add conan dependencies
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/build/generators")

# Depend: grpc
find_package(protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)

# Depend: cxxops
find_package(cxxopts)

# Create ICD library
add_subdirectory("resources/proto")

# Create app targets
add_subdirectory("client")
add_subdirectory("server")

# Install
include(GNUInstallDirs)

install(TARGETS 
    ${CMAKE_PROJECT_NAME}_client
    ${CMAKE_PROJECT_NAME}_server
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
