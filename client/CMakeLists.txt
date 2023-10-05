# Client

set(CMAKE_TARGET_NAME "${CMAKE_PROJECT_NAME}_client")

add_executable(${CMAKE_TARGET_NAME}
    # Resources
    ${RESOURCESFILE}    
    # Sources
    "src/CInventoryClient.cpp"
    "src/main.cpp"
    # Includes
    "include/CInventoryClient.h" )

target_include_directories(${CMAKE_TARGET_NAME} PRIVATE "include")

target_link_libraries(${CMAKE_TARGET_NAME} PRIVATE 
    cxxopts::cxxopts
    grpc::grpc
    InventoryAPI )
