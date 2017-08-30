#pragma once

#include <improbable/worker.h>
#include <CrySchematyc/CoreAPI.h>
#include <improbable/standard_library.h>

#include "SpatialOs/CallbackList.h"
#include "SpatialOs/SpatialOsComponent.h"
#include "SpatialOs/SpatialOs.h"
#include "Generated/Types.h"

struct SPositionCoordsUpdatedSignal {
    SPositionCoordsUpdatedSignal() = default;
    SPositionCoordsUpdatedSignal(improbable::Coordinates coords) : m_coords(coords) {}
    improbable::Coordinates m_coords;
};

static inline void ReflectType(Schematyc::CTypeDesc<SPositionCoordsUpdatedSignal>& desc)
{
    desc.SetGUID("{A6FF27DB-801A-44B4-8813-87B4E723184B}"_cry_guid);
    desc.SetLabel("OnPositionCoordsChanged");
    desc.AddMember(&SPositionCoordsUpdatedSignal::m_coords, 1, "PositionCoords", "PositionCoords", "", improbable::Coordinates::Create());
}


class CSPosition: public ISpatialOsComponent<improbable::Position>
{
  public:
      CSPosition() : m_coords(improbable::Coordinates::Create()) {}
      virtual ~CSPosition() {}

      void Initialise(worker::Connection& connection, worker::View& view, worker::EntityId entityId)
      {
        RegisterDefaultCallbacks();
      }

      static void ReflectType(Schematyc::CTypeDesc<CSPosition>& desc) {
        desc.SetGUID("{E7E8D085-CAED-47F4-A3C8-A2E10B6BEF50}"_cry_guid);
        desc.SetLabel("Position");
        desc.SetEditorCategory("SpatialOS");
        desc.AddMember(&CSPosition::m_coords, 1, "Coords", "Coords", "", improbable::Coordinates::Create());

      }

      void ApplyComponentUpdate(const Component::Update& update) override
      {
        if (!update.coords().empty())
        {
            m_coords = *update.coords();
            m_coords_callbacks.Update(m_coords);
        	if (Schematyc::IObject *pObject = GetEntity()->GetSchematycObject())
        	{
        		pObject->ProcessSignal(SPositionCoordsUpdatedSignal(m_coords));
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
        entity.Add<Component>(Component::Data(m_coords));
        writeAcl.emplace(Component::ComponentId, m_spatialOs->GetSnapshotRequirementSet());
      }

      improbable::Coordinates GetCoords() const { return m_coords; }

      void UpdateCoords(improbable::Coordinates value)
      {
          Component::Update update;
          update.set_coords(value);
          m_connection->SendComponentUpdate<Component>(GetSpatialOsEntityId(), update);
      }

      size_t AddCoordsCallback(std::function<void(improbable::Coordinates)> cb) { return m_coords_callbacks.Add(cb); }
      bool RemoveCoordsCallback(size_t key) { return m_coords_callbacks.Remove(key); }



  private:
      improbable::Coordinates m_coords;
      CCallbackList<size_t, improbable::Coordinates> m_coords_callbacks;

};

namespace {
    static void RegisterCSPosition(Schematyc::IEnvRegistrar& registrar)
    {
    	Schematyc::CEnvRegistrationScope scope = registrar.Scope(IEntity::GetEntityScopeGUID());
        Schematyc::CEnvRegistrationScope componentScope = scope.Register(SCHEMATYC_MAKE_ENV_COMPONENT(CSPosition));
        {
            auto pFunction = SCHEMATYC_MAKE_ENV_FUNCTION(&CSPosition::UpdateCoords, "{2A370421-BBB8-47F4-B1D6-58B81BDDEEE7}"_cry_guid, "UpdateCoords");
            pFunction->BindInput(1, 1, "Coords");
            componentScope.Register(pFunction);
        }

        componentScope.Register(SCHEMATYC_MAKE_ENV_SIGNAL(SPositionCoordsUpdatedSignal));

    }
    CRY_STATIC_AUTO_REGISTER_FUNCTION(&RegisterCSPosition);
}