cmake_minimum_required (VERSION 3.6.2)
set(CRYENGINE_DIR "C:/Program Files (x86)/Crytek/CRYENGINE Launcher/Crytek/CRYENGINE_5.4")
set(TOOLS_CMAKE_DIR "${CRYENGINE_DIR}/Tools/CMake")

set(PROJECT_BUILD 1)
set(PROJECT_DIR "E:/dev/cry-spatialos/tps-example/workers/tps-client")

include("${TOOLS_CMAKE_DIR}/CommonOptions.cmake")

add_subdirectory("${CRYENGINE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/CRYENGINE")

include("${TOOLS_CMAKE_DIR}/Configure.cmake")
start_sources()

sources_platform(ALL)
add_sources("Game_uber.cpp"
    PROJECTS Game
    SOURCE_GROUP "Root"
		"GamePlugin.cpp"
		"StdAfx.cpp"
		"GamePlugin.h"
		"StdAfx.h"
)
add_sources("Components_uber.cpp"
    PROJECTS Game
    SOURCE_GROUP "Components"
		"Components/Player.cpp"
		"Components/SpawnPoint.cpp"
		"Components/Bullet.h"
		"Components/Player.h"
		"Components/SpawnPoint.h"
)
add_sources("SpatialOs_uber.cpp"
    PROJECTS Game
    SOURCE_GROUP "Components\\\\SpatialOs"
		"Components/SpatialOs/Bullet.cpp"
		"Components/SpatialOs/Movement.cpp"
		"Components/SpatialOs/Player.cpp"
		"Components/SpatialOs/PlayerScore.cpp"
		"Components/SpatialOs/Bullet.h"
		"Components/SpatialOs/Movement.h"
		"Components/SpatialOs/Player.h"
		"Components/SpatialOs/PlayerScore.h"
)
add_sources("Generated_uber.cpp"
    PROJECTS Game
    SOURCE_GROUP "Generated"
		"Generated/Types.h"
)
add_sources("components_uber.cpp"
    PROJECTS Game
    SOURCE_GROUP "Generated\\\\components"
)
add_sources("automaton_uber.cpp"
    PROJECTS Game
    SOURCE_GROUP "Generated\\\\components\\\\automaton"
		"Generated/components/automaton/Spawner.h"
		"Generated/components/automaton/Tree.h"
)
add_sources("player_uber.cpp"
    PROJECTS Game
    SOURCE_GROUP "Generated\\\\components\\\\automaton\\\\player"
		"Generated/components/automaton/player/Bullet.h"
		"Generated/components/automaton/player/Movement.h"
		"Generated/components/automaton/player/Player.h"
		"Generated/components/automaton/player/Score.h"
)
add_sources("improbable_uber.cpp"
    PROJECTS Game
    SOURCE_GROUP "Generated\\\\components\\\\improbable"
		"Generated/components/improbable/EntityAcl.h"
		"Generated/components/improbable/Metadata.h"
		"Generated/components/improbable/Persistence.h"
		"Generated/components/improbable/Position.h"
)
add_sources("SpatialOs_uber.cpp"
    PROJECTS Game
    SOURCE_GROUP "SpatialOs"
		"SpatialOs/SpatialOs.cpp"
		"SpatialOs/SpatialOs.h"
)

end_sources()

CryEngineModule(Game PCH "StdAfx.cpp" SOLUTION_FOLDER "Project")

target_include_directories(${THIS_PROJECT}
PRIVATE 
    "${CRYENGINE_DIR}/Code/CryEngine/CryCommon"
    "${CRYENGINE_DIR}/Code/CryEngine/CryAction"
	"${CRYENGINE_DIR}/Code/CryEngine/CrySchematyc/Core/Interface"
	"${CRYENGINE_DIR}/Code/CryPlugins/CryDefaultEntities/Module"
)

# Set StartUp project in Visual Studio

add_library(GameLauncher STATIC "${CRYENGINE_DIR}/Code/CryEngine/CryCommon/CryCore/Platform/platform.h")
set_target_properties(GameLauncher PROPERTIES LINKER_LANGUAGE CXX)
if (WIN32)
    set_visual_studio_debugger_command(GameLauncher "${CRYENGINE_DIR}/bin/win_x64/GameLauncher.exe" "-project \"E:/dev/cry-spatialos/tps-example/workers/tps-client/Game.cryproject\"")
endif()

add_library(Sandbox STATIC "${CRYENGINE_DIR}/Code/CryEngine/CryCommon/CryCore/Platform/platform.h")
set_target_properties(Sandbox PROPERTIES LINKER_LANGUAGE CXX)
if (WIN32)
    set_visual_studio_debugger_command(Sandbox "${CRYENGINE_DIR}/bin/win_x64/Sandbox.exe" "-project \"E:/dev/cry-spatialos/tps-example/workers/tps-client/Game.cryproject\"")
endif()

add_library(GameServer STATIC "${CRYENGINE_DIR}/Code/CryEngine/CryCommon/CryCore/Platform/platform.h")
set_target_properties(GameServer PROPERTIES LINKER_LANGUAGE CXX)
if (WIN32)
    set_visual_studio_debugger_command(GameServer "${CRYENGINE_DIR}/bin/win_x64/Game_Server.exe" "-project \"E:/dev/cry-spatialos/tps-example/workers/tps-client/Game.cryproject\"")
endif()

set_solution_startup_target(GameLauncher)

if (WIN32)
    set_visual_studio_debugger_command( ${THIS_PROJECT} "${CRYENGINE_DIR}/bin/win_x64/GameLauncher.exe" "-project \"E:/dev/cry-spatialos/tps-example/workers/tps-client/Game.cryproject\"" )
endif()

#BEGIN-CUSTOM
# Make any custom changes here, modifications outside of the block will be discarded on regeneration.
get_filename_component(BASE_PATH ${PROJECT_SOURCE_DIR}/../../../../ REALPATH)
get_filename_component(GEN_CODE_PATH ${PROJECT_SOURCE_DIR}/../Generated/ REALPATH)

add_custom_target(spatial_codegen_std
				  COMMAND spatial process_schema generate --cachePath="${BASE_PATH}/workers/tps-client/.spatialos/schema_codegen_cache_std" --output="${PROJECT_SOURCE_DIR}/../Generated/std" --language=cpp --input="${BASE_PATH}/build/dependencies/schema/standard_library"
)
add_custom_target(spatial_codegen_usr
					COMMAND spatial process_schema generate --cachePath="${BASE_PATH}/workers/tps-client/.spatialos/schema_codegen_cache_usr" --output="${PROJECT_SOURCE_DIR}/../Generated/usr" --language=cpp --input="${BASE_PATH}/schema" --dependency="${BASE_PATH}/build/dependencies/schema/standard_library"
)

add_custom_target(spatial_codegen_json
					COMMAND spatial process_schema generate --cachePath="${BASE_PATH}/workers/tps-client/.spatialos/schema_codegen_cache_json" --output="${PROJECT_SOURCE_DIR}/../Generated/json" --language=ast_json
)

add_custom_target(codegen_components
					COMMAND java -jar ${BASE_PATH}/../tools/ComponentGenerator/output/ComponentGenerator-1.0.jar "${BASE_PATH}/workers/tps-client/Code/Generated/json" "${BASE_PATH}/workers/tps-client/Code/Game/Generated" "${BASE_PATH}/workers/tps-client/guids/"
)

# execute_process(COMMAND spatial process_schema generate "--cachePath=${BASE_PATH}/workers/tps-client/.spatialos/schema_codegen_cache_std" "--output=${GEN_CODE_PATH}/std" --language=cpp "--input=${BASE_PATH}/build/dependencies/schema/standard_library")
# execute_process(COMMAND spatial process_schema generate "--cachePath=${BASE_PATH}/workers/tps-client/.spatialos/schema_codegen_cache_usr" "--output=${GEN_CODE_PATH}/usr" --language=cpp "--input=${BASE_PATH}/schema")
execute_process(COMMAND java -jar ${BASE_PATH}/../tools/ComponentGenerator/output/ComponentGenerator-1.0.jar "${BASE_PATH}/workers/tps-client/Code/Generated/json" "${BASE_PATH}/workers/tps-client/Code/Game/Generated" "${BASE_PATH}/workers/tps-client/guids/")
add_library(spatial_codegen STATIC "")
set_target_properties(spatial_codegen PROPERTIES LINKER_LANGUAGE CXX)
add_dependencies(spatial_codegen spatial_codegen_std spatial_codegen_usr spatial_codegen_json)
add_dependencies(codegen_components spatial_codegen)
if (CMAKE_CL_64 AND WIN32)
    if(_DEBUG)
        target_include_directories(${THIS_PROJECT} PRIVATE ../ThirdParty/Improbable/windows_x64_debug/include)
        target_include_directories(spatial_codegen PRIVATE ../ThirdParty/Improbable/windows_x64_debug/include)
		file(GLOB_RECURSE LIB_LIST "../ThirdParty/Improbable/windows_x64_debug/lib/*.lib")
    else()
        target_include_directories(${THIS_PROJECT} PRIVATE ../ThirdParty/Improbable/windows_x64_release/include)
        target_include_directories(spatial_codegen PRIVATE ../ThirdParty/Improbable/windows_x64_release/include)
        file(GLOB_RECURSE LIB_LIST "../ThirdParty/Improbable/windows_x64_release/lib/*.lib")
	endif()
	target_compile_options(spatial_codegen PRIVATE "/EHsc")
elseif (LINUX)
    target_include_directories(${THIS_PROJECT} PRIVATE ../ThirdParty/Improbable/linux_x64/include)
    target_include_directories(spatial_codegen PRIVATE ../ThirdParty/Improbable/linux_x64/include)
	file(GLOB_RECURSE LIB_LIST "../ThirdParty/Improbable/linux_x64/lib/*.a")
endif()

file(GLOB_RECURSE SPATIAL_CODEGEN_FILES_STD ${GEN_CODE_PATH}/std/*.cc ${GEN_CODE_PATH}/std/*.h)
file(GLOB_RECURSE SPATIAL_CODEGEN_FILES_USR ${GEN_CODE_PATH}/usr/*.cc ${GEN_CODE_PATH}/usr/*.h)

target_include_directories(${THIS_PROJECT} PRIVATE ${GEN_CODE_PATH}/std ${GEN_CODE_PATH}/usr)
target_include_directories(spatial_codegen PRIVATE ${GEN_CODE_PATH}/std ${GEN_CODE_PATH}/usr)
target_sources(spatial_codegen PRIVATE ${SPATIAL_CODEGEN_FILES_STD} ${SPATIAL_CODEGEN_FILES_USR})
target_link_libraries(spatial_codegen PRIVATE ${LIB_LIST})

add_dependencies(${THIS_PROJECT} spatial_codegen)
add_dependencies(${THIS_PROJECT} codegen_components)

target_link_libraries(${THIS_PROJECT} PRIVATE spatial_codegen ${LIB_LIST})

target_sources(${THIS_PROJECT} 
	PRIVATE
		"${BASE_PATH}/../sdk/src/SpatialOs/SpatialOsEntitySpawner.cpp"
		"${BASE_PATH}/../sdk/src/SpatialOs/CallbackList.h"
		"${BASE_PATH}/../sdk/src/SpatialOs/Commander.h"
		"${BASE_PATH}/../sdk/src/SpatialOs/ComponentSerialiser.h"
		"${BASE_PATH}/../sdk/src/SpatialOs/ScopedViewCallbacks.h"
		"${BASE_PATH}/../sdk/src/SpatialOs/ISpatialOs.h"
		"${BASE_PATH}/../sdk/src/SpatialOs/SpatialOsComponent.h"
		"${BASE_PATH}/../sdk/src/SpatialOs/SpatialOsEntitySpawner.h"
		"${BASE_PATH}/../sdk/src/SpatialOs/component/SpatialOsComponent.h"
		"${BASE_PATH}/../sdk/src/SpatialOs/component/SpatialOsComponent.cpp"
		"${BASE_PATH}/../sdk/src/SpatialOs/SpatialOsView.h"
		"${BASE_PATH}/../sdk/src/SpatialOs/IPostInitializable.h"
)
target_include_directories(${THIS_PROJECT}
	PRIVATE
	"${BASE_PATH}/../sdk/src")

#END-CUSTOM