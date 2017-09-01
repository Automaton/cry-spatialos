#pragma once

#include <improbable/worker.h>
#include <CrySchematyc/CoreAPI.h>
#include <automaton/player/player.h>

#include "SpatialOs/CallbackList.h"
#include "SpatialOs/SpatialOsComponent.h"
#include "SpatialOs/SpatialOs.h"
#include "Generated/Types.h"

struct SPlayerHeartbeatEventSignal {
    SPlayerHeartbeatEventSignal() = default;
    SPlayerHeartbeatEventSignal(automaton::player::Heartbeat heartbeat) : m_heartbeat(heartbeat) {}
    automaton::player::Heartbeat m_heartbeat;
};

static inline void ReflectType(Schematyc::CTypeDesc<SPlayerHeartbeatEventSignal>& desc)
{
    desc.SetGUID("{1CE22C77-0D62-402B-948D-E2A7F9084CF5}"_cry_guid);
    desc.SetLabel("OnPlayerHeartbeatEvent");
    desc.AddMember(&SPlayerHeartbeatEventSignal::m_heartbeat, 1, "PlayerHeartbeat", "PlayerHeartbeat", "", automaton::player::Heartbeat::Create());
}

struct SPlayerKilledPlayerEventSignal {
    SPlayerKilledPlayerEventSignal() = default;
    SPlayerKilledPlayerEventSignal(automaton::player::KilledPlayer killed_player) : m_killed_player(killed_player) {}
    automaton::player::KilledPlayer m_killed_player;
};

static inline void ReflectType(Schematyc::CTypeDesc<SPlayerKilledPlayerEventSignal>& desc)
{
    desc.SetGUID("{DB9B4CCA-92E7-429B-BB20-871662BC1A35}"_cry_guid);
    desc.SetLabel("OnPlayerKilledPlayerEvent");
    desc.AddMember(&SPlayerKilledPlayerEventSignal::m_killed_player, 1, "PlayerKilledPlayer", "PlayerKilledPlayer", "", automaton::player::KilledPlayer::Create());
}


class CSPlayer: public ISpatialOsComponent<automaton::player::Player>
{
  public:
      CSPlayer()  {}
      virtual ~CSPlayer() {}

      void Initialise(worker::Connection& connection, worker::View& view, worker::EntityId entityId)
      {
        RegisterDefaultCallbacks();
      }

      static void ReflectType(Schematyc::CTypeDesc<CSPlayer>& desc) {
        desc.SetGUID("{F41D9E75-EA98-4D35-A949-1BF7ED60F2AD}"_cry_guid);
        desc.SetLabel("Player");
        desc.SetEditorCategory("SpatialOS");
      }

      void ApplyComponentUpdate(const Component::Update& update) override
      {
        if (!update.heartbeat().empty())
        {
            auto& list = update.heartbeat();
            for (auto& heartbeat : list)
            {
                m_heartbeat_callbacks.Update(heartbeat);
                if (Schematyc::IObject *pObject = GetEntity()->GetSchematycObject())
                {
                	pObject->ProcessSignal(SPlayerHeartbeatEventSignal(heartbeat), GetGUID());
                }
            }
        }
        if (!update.killed_player().empty())
        {
            auto& list = update.killed_player();
            for (auto& killed_player : list)
            {
                m_killed_player_callbacks.Update(killed_player);
                if (Schematyc::IObject *pObject = GetEntity()->GetSchematycObject())
                {
                	pObject->ProcessSignal(SPlayerKilledPlayerEventSignal(killed_player), GetGUID());
                }
            }
        }

        if (!IsComponentReady())
        {
            m_isComponentReady = true;
            m_componentReadyCallbacks.Update();
        }
      }

      void WriteComponent(worker::Entity& entity, worker::Map<std::uint32_t, improbable::WorkerRequirementSet>& writeAcl) override
      {
        entity.Add<Component>(Component::Data());
        writeAcl.emplace(Component::ComponentId, m_spatialOs->GetSnapshotRequirementSet());
      }

      size_t AddHeartbeatCallback(std::function<void(automaton::player::Heartbeat)> cb) { return m_heartbeat_callbacks.Add(cb); }
      bool RemoveHeartbeatCallback(size_t key) { return m_heartbeat_callbacks.Remove(key); }

      size_t AddKilledPlayerCallback(std::function<void(automaton::player::KilledPlayer)> cb) { return m_killed_player_callbacks.Add(cb); }
      bool RemoveKilledPlayerCallback(size_t key) { return m_killed_player_callbacks.Remove(key); }



  private:
      CCallbackList<size_t, automaton::player::Heartbeat> m_heartbeat_callbacks;
      CCallbackList<size_t, automaton::player::KilledPlayer> m_killed_player_callbacks;

};

namespace {
    static void RegisterCSPlayer(Schematyc::IEnvRegistrar& registrar)
    {
    	Schematyc::CEnvRegistrationScope scope = registrar.Scope(IEntity::GetEntityScopeGUID());
        Schematyc::CEnvRegistrationScope componentScope = scope.Register(SCHEMATYC_MAKE_ENV_COMPONENT(CSPlayer));
        componentScope.Register(SCHEMATYC_MAKE_ENV_SIGNAL(SPlayerHeartbeatEventSignal));
        componentScope.Register(SCHEMATYC_MAKE_ENV_SIGNAL(SPlayerKilledPlayerEventSignal));

    }
    CRY_STATIC_AUTO_REGISTER_FUNCTION(&RegisterCSPlayer);
}