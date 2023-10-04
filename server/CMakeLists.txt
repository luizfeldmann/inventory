# Server

set(CMAKE_TARGET_NAME "${CMAKE_PROJECT_NAME}_server")

add_executable(${CMAKE_TARGET_NAME}
    # Resources
    ${RESOURCESFILE}
    # Sources
    "src/main.cpp"
    # Includes
    )

target_include_directories(${CMAKE_TARGET_NAME} PRIVATE "include")

target_link_libraries(${CMAKE_TARGET_NAME} PRIVATE InventoryAPI)
