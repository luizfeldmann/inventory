# Server

set(CMAKE_TARGET_NAME "${CMAKE_PROJECT_NAME}_server")

add_executable(${CMAKE_TARGET_NAME}
    # Resources
    ${RESOURCESFILE}
    # Sources
    "src/main.cpp"
    "src/CInventoryServer.cpp"
    # Includes
    "include/CInventoryServer.h" )

target_include_directories(${CMAKE_TARGET_NAME} PRIVATE "include")

set_target_properties(${CMAKE_TARGET_NAME}
    PROPERTIES
    CXX_STANDARD 17)

# Add depends
find_package(ensurethread)

target_link_libraries(${CMAKE_TARGET_NAME} PRIVATE 
    grpc::grpc
    cxxopts::cxxopts
    ensurethread::ensurethread
    InventoryAPI )
