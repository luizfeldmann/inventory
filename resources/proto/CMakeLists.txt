# Exports the protobuffer + gRPC

# Create the ICD target
set(ICD_NAME "InventoryAPI")
set(ICD_INPUT_FILE "${CMAKE_CURRENT_LIST_DIR}/${ICD_NAME}.proto")

add_library(${ICD_NAME} STATIC "${ICD_INPUT_FILE}")

target_link_libraries(${ICD_NAME} PUBLIC
    protobuf::libprotobuf gRPC::grpc++ )

target_include_directories(${ICD_NAME} PUBLIC 
    "${CMAKE_BINARY_DIR}/resources/proto")

protobuf_generate(TARGET ${ICD_NAME} LANGUAGE cpp)

protobuf_generate(
    TARGET ${ICD_NAME}
    LANGUAGE grpc
    GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc
    PLUGIN protoc-gen-grpc=$<TARGET_FILE:gRPC::grpc_cpp_plugin> )
