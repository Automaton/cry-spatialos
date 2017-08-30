#pragma once

#include <improbable/worker.h>
#include <CrySchematyc/CoreAPI.h>
#include <automaton/tree.h>

#include "SpatialOs/CallbackList.h"
#include "SpatialOs/SpatialOsComponent.h"
#include "SpatialOs/SpatialOs.h"
#include "Generated/Types.h"

struct STreeDeadUpdatedSignal {
    STreeDeadUpdatedSignal() = default;
    STreeDeadUpdatedSignal(bool dead) : m_dead(dead) {}
    bool m_dead;
};

static inline void ReflectType(Schematyc::CTypeDesc<STreeDeadUpdatedSignal>& desc)
{
    desc.SetGUID("{96141EDB-E1DC-44F9-A691-6F93DB43D4AE}"_cry_guid);
    desc.SetLabel("OnTreeDeadChanged");
    desc.AddMember(&STreeDeadUpdatedSignal::m_dead, 1, "TreeDead", "TreeDead", "", false);
}


class CSTree: public ISpatialOsComponent<automaton::Tree>
{
  public:
      CSTree() : m_dead(false) {}
      virtual ~CSTree() {}

      void Initialise(worker::Connection& connection, worker::View& view, worker::EntityId entityId)
      {
        RegisterDefaultCallbacks();
      }

      static void ReflectType(Schematyc::CTypeDesc<CSTree>& desc) {
        desc.SetGUID("{C5016FE7-77C4-41D2-ABC0-6FB8604D0A5D}"_cry_guid);
        desc.SetLabel("Tree");
        desc.SetEditorCategory("SpatialOS");
        desc.AddMember(&CSTree::m_dead, 1, "Dead", "Dead", "", false);

      }

      void ApplyComponentUpdate(const Component::Update& update) override
      {
        if (!update.dead().empty())
        {
            m_dead = *update.dead();
            m_dead_callbacks.Update(m_dead);
        	if (Schematyc::IObject *pObject = GetEntity()->GetSchematycObject())
        	{
        		pObject->ProcessSignal(STreeDeadUpdatedSignal(m_dead));
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
        entity.Add<Component>(Component::Data(m_dead));
        writeAcl.emplace(Component::ComponentId, m_spatialOs->GetSnapshotRequirementSet());
      }

      bool GetDead() const { return m_dead; }

      void UpdateDead(bool value)
      {
          Component::Update update;
          update.set_dead(value);
          m_connection->SendComponentUpdate<Component>(GetSpatialOsEntityId(), update);
      }

      size_t AddDeadCallback(std::function<void(bool)> cb) { return m_dead_callbacks.Add(cb); }
      bool RemoveDeadCallback(size_t key) { return m_dead_callbacks.Remove(key); }



  private:
      bool m_dead;
      CCallbackList<size_t, bool> m_dead_callbacks;

};

namespace {
    static void RegisterCSTree(Schematyc::IEnvRegistrar& registrar)
    {
    	Schematyc::CEnvRegistrationScope scope = registrar.Scope(IEntity::GetEntityScopeGUID());
        Schematyc::CEnvRegistrationScope componentScope = scope.Register(SCHEMATYC_MAKE_ENV_COMPONENT(CSTree));
        {
            auto pFunction = SCHEMATYC_MAKE_ENV_FUNCTION(&CSTree::UpdateDead, "{C1CE2DF7-56FE-4CA3-B856-4DA1A2FA403B}"_cry_guid, "UpdateDead");
            pFunction->BindInput(1, 1, "Dead");
            componentScope.Register(pFunction);
        }

        componentScope.Register(SCHEMATYC_MAKE_ENV_SIGNAL(STreeDeadUpdatedSignal));

    }
    CRY_STATIC_AUTO_REGISTER_FUNCTION(&RegisterCSTree);
}