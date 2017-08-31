#pragma once

#include <improbable/worker.h>
#include <CrySchematyc/CoreAPI.h>
#include <automaton/player/score.h>

#include "SpatialOs/CallbackList.h"
#include "SpatialOs/SpatialOsComponent.h"
#include "SpatialOs/SpatialOs.h"
#include "Generated/Types.h"

struct SScoreKillsUpdatedSignal {
    SScoreKillsUpdatedSignal() = default;
    SScoreKillsUpdatedSignal(uint32_t kills) : m_kills(kills) {}
    uint32_t m_kills;
};

static inline void ReflectType(Schematyc::CTypeDesc<SScoreKillsUpdatedSignal>& desc)
{
    desc.SetGUID("{F36E60E7-71B4-458E-9253-331FD1F75EB7}"_cry_guid);
    desc.SetLabel("OnScoreKillsChanged");
    desc.AddMember(&SScoreKillsUpdatedSignal::m_kills, 1, "ScoreKills", "ScoreKills", "", 0);
}

struct SScoreDeathsUpdatedSignal {
    SScoreDeathsUpdatedSignal() = default;
    SScoreDeathsUpdatedSignal(uint32_t deaths) : m_deaths(deaths) {}
    uint32_t m_deaths;
};

static inline void ReflectType(Schematyc::CTypeDesc<SScoreDeathsUpdatedSignal>& desc)
{
    desc.SetGUID("{7B2FA68B-8613-4463-B083-1E5EA5362A84}"_cry_guid);
    desc.SetLabel("OnScoreDeathsChanged");
    desc.AddMember(&SScoreDeathsUpdatedSignal::m_deaths, 2, "ScoreDeaths", "ScoreDeaths", "", 0);
}


class CSScore: public ISpatialOsComponent<automaton::player::Score>
{
  public:
      CSScore() : m_kills(0), m_deaths(0) {}
      virtual ~CSScore() {}

      void Initialise(worker::Connection& connection, worker::View& view, worker::EntityId entityId)
      {
        RegisterDefaultCallbacks();
      }

      static void ReflectType(Schematyc::CTypeDesc<CSScore>& desc) {
        desc.SetGUID("{3E58016A-92E5-4018-883D-B5F6AA6DAB26}"_cry_guid);
        desc.SetLabel("Score");
        desc.SetEditorCategory("SpatialOS");
        desc.AddMember(&CSScore::m_kills, 1, "Kills", "Kills", "", 0);
        desc.AddMember(&CSScore::m_deaths, 2, "Deaths", "Deaths", "", 0);

      }

      void ApplyComponentUpdate(const Component::Update& update) override
      {
        if (!update.kills().empty())
        {
            m_kills = *update.kills();
            m_kills_callbacks.Update(m_kills);
        	if (Schematyc::IObject *pObject = GetEntity()->GetSchematycObject())
        	{
        		pObject->ProcessSignal(SScoreKillsUpdatedSignal(m_kills));
        	}
        }
        if (!update.deaths().empty())
        {
            m_deaths = *update.deaths();
            m_deaths_callbacks.Update(m_deaths);
        	if (Schematyc::IObject *pObject = GetEntity()->GetSchematycObject())
        	{
        		pObject->ProcessSignal(SScoreDeathsUpdatedSignal(m_deaths));
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
        entity.Add<Component>(Component::Data(m_kills, m_deaths));
        writeAcl.emplace(Component::ComponentId, m_spatialOs->GetSnapshotRequirementSet());
      }

      uint32_t GetKills() const { return m_kills; }

      void UpdateKills(uint32_t value)
      {
          Component::Update update;
          update.set_kills(value);
          m_connection->SendComponentUpdate<Component>(GetSpatialOsEntityId(), update);
      }

      uint32_t GetDeaths() const { return m_deaths; }

      void UpdateDeaths(uint32_t value)
      {
          Component::Update update;
          update.set_deaths(value);
          m_connection->SendComponentUpdate<Component>(GetSpatialOsEntityId(), update);
      }

      size_t AddKillsCallback(std::function<void(uint32_t)> cb) { return m_kills_callbacks.Add(cb); }
      bool RemoveKillsCallback(size_t key) { return m_kills_callbacks.Remove(key); }

      size_t AddDeathsCallback(std::function<void(uint32_t)> cb) { return m_deaths_callbacks.Add(cb); }
      bool RemoveDeathsCallback(size_t key) { return m_deaths_callbacks.Remove(key); }



  private:
      uint32_t m_kills;
      uint32_t m_deaths;
      CCallbackList<size_t, uint32_t> m_kills_callbacks;
      CCallbackList<size_t, uint32_t> m_deaths_callbacks;

};

namespace {
    static void RegisterCSScore(Schematyc::IEnvRegistrar& registrar)
    {
    	Schematyc::CEnvRegistrationScope scope = registrar.Scope(IEntity::GetEntityScopeGUID());
        Schematyc::CEnvRegistrationScope componentScope = scope.Register(SCHEMATYC_MAKE_ENV_COMPONENT(CSScore));
        {
            auto pFunction = SCHEMATYC_MAKE_ENV_FUNCTION(&CSScore::UpdateKills, "{CB39D973-3AC3-47DA-9E82-2477ABAB07B2}"_cry_guid, "UpdateKills");
            pFunction->BindInput(1, 1, "Kills");
            componentScope.Register(pFunction);
        }
        {
            auto pFunction = SCHEMATYC_MAKE_ENV_FUNCTION(&CSScore::UpdateDeaths, "{93EC8622-947D-4773-9AA7-2FBE3AB1C5EE}"_cry_guid, "UpdateDeaths");
            pFunction->BindInput(1, 1, "Deaths");
            componentScope.Register(pFunction);
        }

        componentScope.Register(SCHEMATYC_MAKE_ENV_SIGNAL(SScoreKillsUpdatedSignal));
        componentScope.Register(SCHEMATYC_MAKE_ENV_SIGNAL(SScoreDeathsUpdatedSignal));

    }
    CRY_STATIC_AUTO_REGISTER_FUNCTION(&RegisterCSScore);
}