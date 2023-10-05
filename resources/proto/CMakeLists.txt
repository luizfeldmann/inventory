# Exports the protobuffer + gRPC

# Create the ICD target
set(ICD_NAME "InventoryAPI")
set(ICD_INPUT_FILE "${CMAKE_CURRENT_LIST_DIR}/${ICD_NAME}.proto")
set(ICD_OUTPUT_DIR "${CMAKE_BINARY_DIR}/ICD/")

set(ICD_PB_SRC  "${ICD_OUTPUT_DIR}/${ICD_NAME}.pb.cc")
set(ICD_PB_INC  "${ICD_OUTPUT_DIR}/${ICD_NAME}.pb.h")
set(ICD_RPC_SRC "${ICD_OUTPUT_DIR}/${ICD_NAME}.grpc.pb.cc")
set(ICD_RPC_INC "${ICD_OUTPUT_DIR}/${ICD_NAME}.grpc.pb.h")

set(ICD_FILES_ALL
    "${ICD_PB_SRC}" 
    "${ICD_PB_INC}"
    "${ICD_RPC_SRC}"
    "${ICD_RPC_INC}" )

add_custom_command( OUTPUT ${ICD_FILES_ALL}
    COMMAND "$<TARGET_FILE:protobuf::protoc>"
    ARGS --grpc_out "${ICD_OUTPUT_DIR}"
        --cpp_out "${ICD_OUTPUT_DIR}"
        --proto_path "${CMAKE_CURRENT_LIST_DIR}"
        --plugin=protoc-gen-grpc="$<TARGET_FILE:gRPC::grpc_cpp_plugin>"
        "${ICD_INPUT_FILE}"
    DEPENDS "${ICD_INPUT_FILE}")

add_library(${ICD_NAME} STATIC ${ICD_FILES_ALL})

target_include_directories(${ICD_NAME} PUBLIC "${ICD_OUTPUT_DIR}")

target_link_libraries(${ICD_NAME} PUBLIC grpc::grpc)
