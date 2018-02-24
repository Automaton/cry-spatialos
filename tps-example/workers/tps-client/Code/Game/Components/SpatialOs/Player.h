#pragma once
#include <SpatialOs/SpatialOsComponent.h>
#include <CrySchematyc/CoreAPI.h>
#include <automaton/player/player.h>

class CPlayerComponent;

using namespace automaton::player;

class CSPlayer final : public ISpatialOsComponent<Player>
{
public:
	CSPlayer();
	virtual ~CSPlayer() {}

	// IEntityComponent
	void ProcessEvent(SEntityEvent& event) override;
	uint64 GetEventMask() const override;
	void FireBullet(const QuatTS& origin, EntityId id);
	void SendKillEvent(worker::EntityId killedPlayer) const;
	void Respawn();

	static void ReflectType(Schematyc::CTypeDesc<CSPlayer>& desc)
	{
		desc.SetGUID("{96E4F531-2980-46CC-B9DA-670CC3C655B4}"_cry_guid);
	}

	void WriteComponent(worker::Entity& entity, worker::Map<std::uint32_t, improbable::WorkerRequirementSet>& writeAcl) override;
	// ISpatialOsComponent
	void ApplyComponentUpdate(const automaton::player::Player::Update& update) override;

	CPlayerComponent *GetPlayerComponent() const { return m_pPlayer; }

private:
	void Initialise(worker::Connection& connection, CSpatialOsView& view, worker::EntityId entityId) override;
	void OnAuthority(bool auth);

	const float HeartbeatInterval = 3;
	float m_lastHeartbeatTime;

	size_t m_authCallback;

	CPlayerComponent *m_pPlayer;
};
