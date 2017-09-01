#include "StdAfx.h"
#include "GamePlugin.h"

#include "Components/Player.h"

#include <CrySchematyc/Env/IEnvRegistry.h>
#include <CrySchematyc/Env/EnvPackage.h>

// Included only once per DLL module.
#include <CryCore/Platform/platform_impl.inl>
#include "Generated/Types.h"

CGamePlugin::~CGamePlugin()
{
	gEnv->pSystem->GetISystemEventDispatcher()->RemoveListener(this);

	if (gEnv->pSchematyc)
	{
		gEnv->pSchematyc->GetEnvRegistry().DeregisterPackage(GetSchematycPackageGUID());
		gEnv->pSchematyc->GetEnvRegistry().DeregisterPackage("{051ADC65-49F8-4F97-942F-CA7E71287CFC}"_cry_guid);
	}
}

bool CGamePlugin::Initialize(SSystemGlobalEnvironment& env, const SSystemInitParams& initParams)
{
	// Register for engine system events, in our case we need ESYSTEM_EVENT_GAME_POST_INIT to load the map
	gEnv->pSystem->GetISystemEventDispatcher()->RegisterListener(this, "CGamePlugin");
	SetUpdateFlags(EUpdateType_Update);
	return true;
}

void CGamePlugin::OnPluginUpdate(EPluginUpdateType updateType)
{
	if (updateType == EUpdateType_Update)
	{
		m_spatialOs.ProcessEvents();
	}
}

void CGamePlugin::OnSystemEvent(ESystemEvent event, UINT_PTR wparam, UINT_PTR lparam)
{
	switch (event)
	{
	case ESYSTEM_EVENT_REGISTER_SCHEMATYC_ENV:
	{
		// Register all components that belong to this plug-in
		auto staticAutoRegisterLambda = [](Schematyc::IEnvRegistrar& registrar)
		{
			// Call all static callback registered with the CRY_STATIC_AUTO_REGISTER_WITH_PARAM
			Detail::CStaticAutoRegistrar<Schematyc::IEnvRegistrar&>::InvokeStaticCallbacks(registrar);
		};

		if (gEnv->pSchematyc)
		{
			gEnv->pSchematyc->GetEnvRegistry().RegisterPackage(
				stl::make_unique<Schematyc::CEnvPackage>(
					GetSchematycPackageGUID(),
					"EntityComponents",
					"Crytek GmbH",
					"Components",
					staticAutoRegisterLambda
					)
			);
			gEnv->pSchematyc->GetEnvRegistry().RegisterPackage(
				stl::make_unique<Schematyc::CEnvPackage>(
					"{051ADC65-49F8-4F97-942F-CA7E71287CFC}"_cry_guid,
					"SpatialOS",
					"Automaton Games Ltd",
					"SpatialOS",
					[] (Schematyc::IEnvRegistrar& registrar)
					{
						SpatialOsTypes::RegisterSpatialOs(registrar);
					}
					)
			);


		}
	}break;
		// Called when the game framework has initialized and we are ready for game logic to start
	case ESYSTEM_EVENT_GAME_POST_INIT:
	{
		// Don't need to load the map in editor
		if (!gEnv->IsEditor())
		{
			gEnv->pConsole->ExecuteString("map BasicMap", false, true);
		}
	}
	break;
	case ESYSTEM_EVENT_LEVEL_GAMEPLAY_START:
	{
		if (!gEnv->IsEditor())
		{
			CSpatialOs::RemoveSpatialOsEntities();
			m_spatialOs.ConnectToSpatialOs();
		}
	}
	break;
	case ESYSTEM_EVENT_EDITOR_GAME_MODE_CHANGED:
	{
		if (!gEnv->IsEditor()) return;
		if (wparam)
		{
			m_spatialOs.ConnectToSpatialOs();
		}
		else
		{
			m_spatialOs.DisconnectFromSpatialOs();
		}
	}
	break;
	case ESYSTEM_EVENT_EDITOR_SIMULATION_MODE_CHANGED:
	{
			
	}
	break;
	case ESYSTEM_EVENT_FULL_SHUTDOWN:
	case ESYSTEM_EVENT_FAST_SHUTDOWN:
		if (!gEnv->IsEditor())
			m_spatialOs.DisconnectFromSpatialOs();
		break;
	}
}


CRYREGISTER_SINGLETON_CLASS(CGamePlugin)