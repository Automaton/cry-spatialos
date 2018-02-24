#pragma once

#include <CrySystem/ICryPlugin.h>
#include <CryGame/IGameFramework.h>
#include <CryEntitySystem/IEntityClass.h>
#include <CryNetwork/INetwork.h>
#include "SpatialOs/SpatialOs.h"

class CPlayerComponent;

// The entry-point of the application
// An instance of CGamePlugin is automatically created when the library is loaded
// We then construct the local player entity and CPlayerComponent instance when OnClientConnectionReceived is first called.
class CGamePlugin 
	: public ICryPlugin
	, public ISystemEventListener
{
public:
	CRYINTERFACE_SIMPLE(ICryPlugin)
	CRYGENERATE_SINGLETONCLASS_GUID(CGamePlugin, "Game_Blank", "f01244b0-a4e7-4dc6-91e1-0ed18906fe7c"_cry_guid)
	static CryGUID GetSchematycPackageGUID() { return "{0900F201-49F3-4B3C-81D1-0C91CF3C8FDA}"_cry_guid; }

	virtual ~CGamePlugin();
	
	// ICryPlugin
	virtual const char* GetName() const override { return "GamePlugin"; }
	virtual const char* GetCategory() const override { return "Game"; }
	virtual bool Initialize(SSystemGlobalEnvironment& env, const SSystemInitParams& initParams) override;
	void OnPluginUpdate(EPluginUpdateType updateType) override;
	// ~ICryPlugin

	// ISystemEventListener
	virtual void OnSystemEvent(ESystemEvent event, UINT_PTR wparam, UINT_PTR lparam) override;
	// ~ISystemEventListener

	static CSpatialOs& GetSpatialOs() { return *s_spatialOs; }

protected:
	// Map containing player components, key is the channel id received in OnClientConnectionReceived
	std::unordered_map<int, EntityId> m_players;
	static CSpatialOs* s_spatialOs;
};