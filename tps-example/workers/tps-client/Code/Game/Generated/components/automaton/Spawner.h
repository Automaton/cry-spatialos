#pragma once

#include <improbable/worker.h>
#include <CrySchematyc/CoreAPI.h>
#include <automaton/spawner.h>

#include "SpatialOs/CallbackList.h"
#include "SpatialOs/SpatialOsComponent.h"
#include "SpatialOs/SpatialOs.h"
#include "Generated/Types.h"

struct SSpawnerSpawnPlayerResponseSignal {
    SSpawnerSpawnPlayerResponseSignal() = default;
    SSpawnerSpawnPlayerResponseSignal(automaton::SpawnPlayerResponse spawn_player, uint32_t requestId) : m_spawn_player(spawn_player), m_requestId(requestId) {}
    automaton::SpawnPlayerResponse m_spawn_player;
    uint32_t m_requestId;
};

static inline void ReflectType(Schematyc::CTypeDesc<SSpawnerSpawnPlayerResponseSignal>& desc)
{
    desc.SetGUID("{2C99875F-D7D5-400B-A0BE-B605BB52680C}"_cry_guid);
    desc.SetLabel("OnSpawnerSpawnPlayerResponse");
    desc.AddMember(&SSpawnerSpawnPlayerResponseSignal::m_spawn_player, 0, "Response", "Response", "", automaton::SpawnPlayerResponse::Create());
    desc.AddMember(&SSpawnerSpawnPlayerResponseSignal::m_requestId, 'rid', "RequestId", "RequestId", "", 0);
}

struct SSpawnerSpawnPlayerRequestSignal {
    SSpawnerSpawnPlayerRequestSignal() = default;
    SSpawnerSpawnPlayerRequestSignal(automaton::SpawnPlayerRequest spawn_player, uint32_t requestId) : m_spawn_player(spawn_player), m_requestId(requestId) {}
    automaton::SpawnPlayerRequest m_spawn_player;
    uint32_t m_requestId;
};

static inline void ReflectType(Schematyc::CTypeDesc<SSpawnerSpawnPlayerRequestSignal>& desc)
{
    desc.SetGUID("{D3FBCE0D-3926-4F6D-A8E2-100A1153A1FB}"_cry_guid);
    desc.SetLabel("OnSpawnerSpawnPlayerRequest");
    desc.AddMember(&SSpawnerSpawnPlayerRequestSignal::m_spawn_player, 0, "Request", "Request", "", automaton::SpawnPlayerRequest::Create());
    desc.AddMember(&SSpawnerSpawnPlayerRequestSignal::m_requestId, 'rid', "RequestId", "RequestId", "", 0);
}


class CSSpawner: public ISpatialOsComponent<automaton::Spawner>
{
  public:
      CSSpawner()  {}
      virtual ~CSSpawner() {}

      void Initialise(worker::Connection& connection, worker::View& view, worker::EntityId entityId)
      {
        RegisterDefaultCallbacks();
        m_callbacks->Add(m_view->OnCommandResponse<automaton::Spawner::Commands::SpawnPlayer>([this] (worker::CommandResponseOp<automaton::Spawner::Commands::SpawnPlayer> op)
        {
            if (GetSpatialOsEntityId() == op.EntityId && op.StatusCode == worker::StatusCode::kSuccess) OnSpawnPlayerResponse(*op.Response, op.RequestId.Id);
        }));
        m_callbacks->Add(m_view->OnCommandRequest<automaton::Spawner::Commands::SpawnPlayer>([this] (worker::CommandRequestOp<automaton::Spawner::Commands::SpawnPlayer> op)
        {
            if (GetSpatialOsEntityId() == op.EntityId) OnSpawnPlayerRequest(op.Request, op.RequestId.Id);
        }));

      }

      static void ReflectType(Schematyc::CTypeDesc<CSSpawner>& desc) {
        desc.SetGUID("{8FF296D4-CA67-4A88-825D-8B6CBDC2BCA1}"_cry_guid);
        desc.SetLabel("Spawner");
        desc.SetEditorCategory("SpatialOS");
      }

      void ApplyComponentUpdate(const Component::Update& update) override
      {
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

      uint32_t SendSpawnPlayerRequest(const automaton::SpawnPlayerRequest& request) const
      {
          return m_connection->SendCommandRequest<automaton::Spawner::Commands::SpawnPlayer>(GetSpatialOsEntityId(), request, 3000).Id;
      }

      void SendSpawnPlayerResponse(const automaton::SpawnPlayerResponse& resp, uint32_t requestId) const
      {
          m_connection->SendCommandResponse<automaton::Spawner::Commands::SpawnPlayer>(worker::RequestId<worker::IncomingCommandRequest<automaton::Spawner::Commands::SpawnPlayer>>(requestId), resp);
      }

      size_t AddSpawnPlayerResponseCallback(std::function<void(automaton::SpawnPlayerResponse, uint32_t)> cb) { return m_spawn_player_rs_callbacks.Add(cb); }
      bool RemoveSpawnPlayerResponseCallback(size_t key) { return m_spawn_player_rs_callbacks.Remove(key); }

      size_t AddSpawnPlayerRequestCallback(std::function<void(automaton::SpawnPlayerRequest, uint32_t)> cb) { return m_spawn_player_rq_callbacks.Add(cb); }
      bool RemoveSpawnPlayerRequestCallback(size_t key) { return m_spawn_player_rq_callbacks.Remove(key); }

      void OnSpawnPlayerResponse(automaton::SpawnPlayerResponse response, uint32_t requestId)
      {
          m_spawn_player_rs_callbacks.Update(response, requestId);
          if (Schematyc::IObject *pObject = GetEntity()->GetSchematycObject())
          {
          	pObject->ProcessSignal(SSpawnerSpawnPlayerResponseSignal(response, requestId));
          }
      }

      void OnSpawnPlayerRequest(automaton::SpawnPlayerRequest request, uint32_t requestId)
      {
          m_spawn_player_rq_callbacks.Update(request, requestId);

          if (Schematyc::IObject *pObject = GetEntity()->GetSchematycObject())
          {
           	pObject->ProcessSignal(SSpawnerSpawnPlayerRequestSignal(request, requestId));
          }
      }



  private:
      CCallbackList<size_t, automaton::SpawnPlayerResponse, uint32_t> m_spawn_player_rs_callbacks;
      CCallbackList<size_t, automaton::SpawnPlayerRequest, uint32_t> m_spawn_player_rq_callbacks;

};

namespace {
    static void RegisterCSSpawner(Schematyc::IEnvRegistrar& registrar)
    {
    	Schematyc::CEnvRegistrationScope scope = registrar.Scope(IEntity::GetEntityScopeGUID());
        Schematyc::CEnvRegistrationScope componentScope = scope.Register(SCHEMATYC_MAKE_ENV_COMPONENT(CSSpawner));
        {
            auto pFunction = SCHEMATYC_MAKE_ENV_FUNCTION(&CSSpawner::SendSpawnPlayerRequest, "{1EEAAE28-B811-4D32-AA76-FE1CD625A34D}"_cry_guid, "SendSpawnPlayerRequest");
            pFunction->BindInput(1, 1, "SpawnPlayer");
            pFunction->BindOutput(0, 0, "RequestId");
            componentScope.Register(pFunction);
        }
        {
            auto pFunction = SCHEMATYC_MAKE_ENV_FUNCTION(&CSSpawner::SendSpawnPlayerResponse, "{290C8743-CB93-4E56-8178-8A41448C5D3B}"_cry_guid, "SendSpawnPlayerResponse");
            pFunction->BindInput(1, 1, "SpawnPlayer");
            pFunction->BindInput(2, 2, "RequestId");
            componentScope.Register(pFunction);
        }

        componentScope.Register(SCHEMATYC_MAKE_ENV_SIGNAL(SSpawnerSpawnPlayerResponseSignal));
        componentScope.Register(SCHEMATYC_MAKE_ENV_SIGNAL(SSpawnerSpawnPlayerRequestSignal));

    }
    CRY_STATIC_AUTO_REGISTER_FUNCTION(&RegisterCSSpawner);
}