cmake_minimum_required (VERSION 3.6.0)
project(snapshotgen LANGUAGES CXX)

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")

get_filename_component(BASE_PATH ${PROJECT_SOURCE_DIR}/../../../ REALPATH)

message(WARNING ${BASE_PATH})

execute_process(COMMAND spatial process_schema generate "--cachePath=${BASE_PATH}/workers/tps-client/.spatialos/schema_codegen_cache_std" "--output=${PROJECT_SOURCE_DIR}/source/Generated/std" --language=cpp --language=cpp "--input=${BASE_PATH}/build/dependencies/schema/standard_library")
execute_process(COMMAND spatial process_schema generate "--cachePath=${BASE_PATH}/workers/tps-client/.spatialos/schema_codegen_cache_usr" "--output=${PROJECT_SOURCE_DIR}/source/Generated/usr" --language=cpp "--input=${BASE_PATH}/schema" --dependency="${BASE_PATH}/build/dependencies/schema/standard_library")

set_property( DIRECTORY PROPERTY VS_STARTUP_PROJECT "snapshotgen" )

include_directories(../Code/ThirdParty/Improbable/windows_x64_release/include source/Generated/usr source/Generated/std)
link_directories(../Code/ThirdParty/Improbable/windows_x64_release/lib)

file(GLOB_RECURSE SPATIAL_CODEGEN_FILES_STD source/Generated/std *.cc *.h)
file(GLOB_RECURSE SPATIAL_CODEGEN_FILES_USR source/Generated/usr *.cc *.h)

add_executable(snapshotgen ${PROJECT_SOURCE_DIR}/source/main.cpp ${SPATIAL_CODEGEN_FILES_STD} ${SPATIAL_CODEGEN_FILES_USR})

target_link_libraries(${PROJECT_NAME} PRIVATE CoreSdk gpr grpc grpc++ libprotobuf RakNetLibStatic ssl WorkerSdk zlibstatic)