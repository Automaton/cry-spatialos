#pragma once

#include <improbable/worker.h>
#include <CrySchematyc/CoreAPI.h>
#include <improbable/standard_library.h>

#include "SpatialOs/CallbackList.h"
#include "SpatialOs/SpatialOsComponent.h"
#include "SpatialOs/SpatialOs.h"
#include "Generated/Types.h"

struct SEntityAclReadAclUpdatedSignal {
    SEntityAclReadAclUpdatedSignal() = default;
    SEntityAclReadAclUpdatedSignal(improbable::WorkerRequirementSet read_acl) : m_read_acl(read_acl) {}
    improbable::WorkerRequirementSet m_read_acl;
};

static inline void ReflectType(Schematyc::CTypeDesc<SEntityAclReadAclUpdatedSignal>& desc)
{
    desc.SetGUID("{A0A36263-572D-4763-B8E7-C9F312F59074}"_cry_guid);
    desc.SetLabel("OnEntityAclReadAclChanged");
    desc.AddMember(&SEntityAclReadAclUpdatedSignal::m_read_acl, 1, "EntityAclReadAcl", "EntityAclReadAcl", "", improbable::WorkerRequirementSet::Create());
}

struct SEntityAclComponentWriteAclUpdatedSignal {
    SEntityAclComponentWriteAclUpdatedSignal() = default;
    SEntityAclComponentWriteAclUpdatedSignal(worker::Map<uint32_t, improbable::WorkerRequirementSet> component_write_acl) : m_component_write_acl(component_write_acl) {}
    worker::Map<uint32_t, improbable::WorkerRequirementSet> m_component_write_acl;
};

static inline void ReflectType(Schematyc::CTypeDesc<SEntityAclComponentWriteAclUpdatedSignal>& desc)
{
    desc.SetGUID("{14627D22-EA75-4134-ADEB-A42D4E87F792}"_cry_guid);
    desc.SetLabel("OnEntityAclComponentWriteAclChanged");
    desc.AddMember(&SEntityAclComponentWriteAclUpdatedSignal::m_component_write_acl, 2, "EntityAclComponentWriteAcl", "EntityAclComponentWriteAcl", "", worker::Map<uint32_t, improbable::WorkerRequirementSet>());
}


class CSEntityAcl: public ISpatialOsComponent<improbable::EntityAcl>
{
  public:
      CSEntityAcl() : m_read_acl(improbable::WorkerRequirementSet::Create()), m_component_write_acl(worker::Map<uint32_t, improbable::WorkerRequirementSet>()) {}
      virtual ~CSEntityAcl() {}

      void Initialise(worker::Connection& connection, worker::View& view, worker::EntityId entityId)
      {
        RegisterDefaultCallbacks();
      }

      static void ReflectType(Schematyc::CTypeDesc<CSEntityAcl>& desc) {
        desc.SetGUID("{286F683C-6EBB-4BC5-BE9F-73E5B75E92DB}"_cry_guid);
        desc.SetLabel("EntityAcl");
        desc.SetEditorCategory("SpatialOS");
        desc.AddMember(&CSEntityAcl::m_read_acl, 1, "ReadAcl", "ReadAcl", "", improbable::WorkerRequirementSet::Create());
        desc.AddMember(&CSEntityAcl::m_component_write_acl, 2, "ComponentWriteAcl", "ComponentWriteAcl", "", worker::Map<uint32_t, improbable::WorkerRequirementSet>());

      }

      void ApplyComponentUpdate(const Component::Update& update) override
      {
        if (!update.read_acl().empty())
        {
            m_read_acl = *update.read_acl();
            m_read_acl_callbacks.Update(m_read_acl);
        	if (Schematyc::IObject *pObject = GetEntity()->GetSchematycObject())
        	{
        		pObject->ProcessSignal(SEntityAclReadAclUpdatedSignal(m_read_acl));
        	}
        }
        if (!update.component_write_acl().empty())
        {
            m_component_write_acl = *update.component_write_acl();
            m_component_write_acl_callbacks.Update(m_component_write_acl);
        	if (Schematyc::IObject *pObject = GetEntity()->GetSchematycObject())
        	{
        		pObject->ProcessSignal(SEntityAclComponentWriteAclUpdatedSignal(m_component_write_acl));
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
        entity.Add<Component>(Component::Data(m_read_acl, m_component_write_acl));
        writeAcl.emplace(Component::ComponentId, m_spatialOs->GetSnapshotRequirementSet());
      }

      improbable::WorkerRequirementSet GetReadAcl() const { return m_read_acl; }

      void UpdateReadAcl(improbable::WorkerRequirementSet value)
      {
          Component::Update update;
          update.set_read_acl(value);
          m_connection->SendComponentUpdate<Component>(GetSpatialOsEntityId(), update);
      }

      worker::Map<uint32_t, improbable::WorkerRequirementSet> GetComponentWriteAcl() const { return m_component_write_acl; }

      void UpdateComponentWriteAcl(worker::Map<uint32_t, improbable::WorkerRequirementSet> value)
      {
          Component::Update update;
          update.set_component_write_acl(value);
          m_connection->SendComponentUpdate<Component>(GetSpatialOsEntityId(), update);
      }

      size_t AddReadAclCallback(std::function<void(improbable::WorkerRequirementSet)> cb) { return m_read_acl_callbacks.Add(cb); }
      bool RemoveReadAclCallback(size_t key) { return m_read_acl_callbacks.Remove(key); }

      size_t AddComponentWriteAclCallback(std::function<void(worker::Map<uint32_t, improbable::WorkerRequirementSet>)> cb) { return m_component_write_acl_callbacks.Add(cb); }
      bool RemoveComponentWriteAclCallback(size_t key) { return m_component_write_acl_callbacks.Remove(key); }



  private:
      improbable::WorkerRequirementSet m_read_acl;
      worker::Map<uint32_t, improbable::WorkerRequirementSet> m_component_write_acl;
      CCallbackList<size_t, improbable::WorkerRequirementSet> m_read_acl_callbacks;
      CCallbackList<size_t, worker::Map<uint32_t, improbable::WorkerRequirementSet>> m_component_write_acl_callbacks;

};

namespace {
    static void RegisterCSEntityAcl(Schematyc::IEnvRegistrar& registrar)
    {
    	Schematyc::CEnvRegistrationScope scope = registrar.Scope(IEntity::GetEntityScopeGUID());
        Schematyc::CEnvRegistrationScope componentScope = scope.Register(SCHEMATYC_MAKE_ENV_COMPONENT(CSEntityAcl));
        {
            auto pFunction = SCHEMATYC_MAKE_ENV_FUNCTION(&CSEntityAcl::UpdateReadAcl, "{537E3102-C5B2-4099-9A0E-77FA1AFE4795}"_cry_guid, "UpdateReadAcl");
            pFunction->BindInput(1, 1, "ReadAcl");
            componentScope.Register(pFunction);
        }
        {
            auto pFunction = SCHEMATYC_MAKE_ENV_FUNCTION(&CSEntityAcl::UpdateComponentWriteAcl, "{B9EBFEB8-5137-43E0-B1C3-0F618D6A9DDD}"_cry_guid, "UpdateComponentWriteAcl");
            pFunction->BindInput(1, 1, "ComponentWriteAcl");
            componentScope.Register(pFunction);
        }

        componentScope.Register(SCHEMATYC_MAKE_ENV_SIGNAL(SEntityAclReadAclUpdatedSignal));
        componentScope.Register(SCHEMATYC_MAKE_ENV_SIGNAL(SEntityAclComponentWriteAclUpdatedSignal));

    }
    CRY_STATIC_AUTO_REGISTER_FUNCTION(&RegisterCSEntityAcl);
}