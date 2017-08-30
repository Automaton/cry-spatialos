#pragma once

#include <improbable/worker.h>
#include <CrySchematyc/CoreAPI.h>
#include <improbable/standard_library.h>

#include "SpatialOs/CallbackList.h"
#include "SpatialOs/SpatialOsComponent.h"
#include "SpatialOs/SpatialOs.h"
#include "Generated/Types.h"

struct SMetadataEntityTypeUpdatedSignal {
    SMetadataEntityTypeUpdatedSignal() = default;
    SMetadataEntityTypeUpdatedSignal(std::string entity_type) : m_entity_type(entity_type) {}
    std::string m_entity_type;
};

static inline void ReflectType(Schematyc::CTypeDesc<SMetadataEntityTypeUpdatedSignal>& desc)
{
    desc.SetGUID("{E922357F-0073-40E2-88EB-A1D3E01A229F}"_cry_guid);
    desc.SetLabel("OnMetadataEntityTypeChanged");
    desc.AddMember(&SMetadataEntityTypeUpdatedSignal::m_entity_type, 1, "MetadataEntityType", "MetadataEntityType", "", "");
}


class CSMetadata: public ISpatialOsComponent<improbable::Metadata>
{
  public:
      CSMetadata() : m_entity_type("") {}
      virtual ~CSMetadata() {}

      void Initialise(worker::Connection& connection, worker::View& view, worker::EntityId entityId)
      {
        RegisterDefaultCallbacks();
      }

      static void ReflectType(Schematyc::CTypeDesc<CSMetadata>& desc) {
        desc.SetGUID("{E342AA19-E493-4225-A058-6432BFE9819A}"_cry_guid);
        desc.SetLabel("Metadata");
        desc.SetEditorCategory("SpatialOS");
        desc.AddMember(&CSMetadata::m_entity_type, 1, "EntityType", "EntityType", "", "");

      }

      void ApplyComponentUpdate(const Component::Update& update) override
      {
        if (!update.entity_type().empty())
        {
            m_entity_type = *update.entity_type();
            m_entity_type_callbacks.Update(m_entity_type);
        	if (Schematyc::IObject *pObject = GetEntity()->GetSchematycObject())
        	{
        		pObject->ProcessSignal(SMetadataEntityTypeUpdatedSignal(m_entity_type));
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
        entity.Add<Component>(Component::Data(m_entity_type));
        writeAcl.emplace(Component::ComponentId, m_spatialOs->GetSnapshotRequirementSet());
      }

      std::string GetEntityType() const { return m_entity_type; }

      void UpdateEntityType(std::string value)
      {
          Component::Update update;
          update.set_entity_type(value);
          m_connection->SendComponentUpdate<Component>(GetSpatialOsEntityId(), update);
      }

      size_t AddEntityTypeCallback(std::function<void(std::string)> cb) { return m_entity_type_callbacks.Add(cb); }
      bool RemoveEntityTypeCallback(size_t key) { return m_entity_type_callbacks.Remove(key); }



  private:
      std::string m_entity_type;
      CCallbackList<size_t, std::string> m_entity_type_callbacks;

};

namespace {
    static void RegisterCSMetadata(Schematyc::IEnvRegistrar& registrar)
    {
    	Schematyc::CEnvRegistrationScope scope = registrar.Scope(IEntity::GetEntityScopeGUID());
        Schematyc::CEnvRegistrationScope componentScope = scope.Register(SCHEMATYC_MAKE_ENV_COMPONENT(CSMetadata));
        {
            auto pFunction = SCHEMATYC_MAKE_ENV_FUNCTION(&CSMetadata::UpdateEntityType, "{BE44B40B-95D6-4C07-BC06-22AE9D0CB024}"_cry_guid, "UpdateEntityType");
            pFunction->BindInput(1, 1, "EntityType");
            componentScope.Register(pFunction);
        }

        componentScope.Register(SCHEMATYC_MAKE_ENV_SIGNAL(SMetadataEntityTypeUpdatedSignal));

    }
    CRY_STATIC_AUTO_REGISTER_FUNCTION(&RegisterCSMetadata);
}