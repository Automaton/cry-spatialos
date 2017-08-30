#pragma once

#include <improbable/worker.h>
#include <CrySchematyc/CoreAPI.h>
#include <improbable/standard_library.h>

#include "SpatialOs/CallbackList.h"
#include "SpatialOs/SpatialOsComponent.h"
#include "SpatialOs/SpatialOs.h"
#include "Generated/Types.h"

class CSPersistence: public ISpatialOsComponent<improbable::Persistence>
{
  public:
      CSPersistence()  {}
      virtual ~CSPersistence() {}

      void Initialise(worker::Connection& connection, worker::View& view, worker::EntityId entityId)
      {
        RegisterDefaultCallbacks();
      }

      static void ReflectType(Schematyc::CTypeDesc<CSPersistence>& desc) {
        desc.SetGUID("{D1BF6539-F8F6-452C-98D7-C75E2D0E1A81}"_cry_guid);
        desc.SetLabel("Persistence");
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


  private:
};

namespace {
    static void RegisterCSPersistence(Schematyc::IEnvRegistrar& registrar)
    {
    	Schematyc::CEnvRegistrationScope scope = registrar.Scope(IEntity::GetEntityScopeGUID());
        Schematyc::CEnvRegistrationScope componentScope = scope.Register(SCHEMATYC_MAKE_ENV_COMPONENT(CSPersistence));
    }
    CRY_STATIC_AUTO_REGISTER_FUNCTION(&RegisterCSPersistence);
}