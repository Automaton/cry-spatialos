#pragma once

#include <improbable/worker.h>
#include <CrySchematyc/CoreAPI.h>
#include <automaton/player/movement.h>

#include "SpatialOs/CallbackList.h"
#include "SpatialOs/SpatialOsComponent.h"
#include "SpatialOs/SpatialOs.h"
#include "Generated/Types.h"
#include <automaton/quaternion.h>


struct SMovementMovingUpdatedSignal {
    SMovementMovingUpdatedSignal() = default;
    SMovementMovingUpdatedSignal(bool moving) : m_moving(moving) {}
    bool m_moving;
};

static inline void ReflectType(Schematyc::CTypeDesc<SMovementMovingUpdatedSignal>& desc)
{
    desc.SetGUID("{1BBCF85B-4E2F-4D21-9BA9-09EE654F548A}"_cry_guid);
    desc.SetLabel("OnMovementMovingChanged");
    desc.AddMember(&SMovementMovingUpdatedSignal::m_moving, 1, "MovementMoving", "MovementMoving", "", false);
}

struct SMovementOrientationUpdatedSignal {
    SMovementOrientationUpdatedSignal() = default;
    SMovementOrientationUpdatedSignal(automaton::Quaternion orientation) : m_orientation(orientation) {}
    automaton::Quaternion m_orientation;
};

static inline void ReflectType(Schematyc::CTypeDesc<SMovementOrientationUpdatedSignal>& desc)
{
    desc.SetGUID("{6AC9C48A-FC73-416A-99BD-977E4ABC8753}"_cry_guid);
    desc.SetLabel("OnMovementOrientationChanged");
    desc.AddMember(&SMovementOrientationUpdatedSignal::m_orientation, 2, "MovementOrientation", "MovementOrientation", "", automaton::Quaternion::Create());
}

struct SMovementRunningUpdatedSignal {
    SMovementRunningUpdatedSignal() = default;
    SMovementRunningUpdatedSignal(bool running) : m_running(running) {}
    bool m_running;
};

static inline void ReflectType(Schematyc::CTypeDesc<SMovementRunningUpdatedSignal>& desc)
{
    desc.SetGUID("{0246C7CA-6902-4BC6-8C95-9F18EF0FC7EE}"_cry_guid);
    desc.SetLabel("OnMovementRunningChanged");
    desc.AddMember(&SMovementRunningUpdatedSignal::m_running, 3, "MovementRunning", "MovementRunning", "", false);
}


class CSMovement: public ISpatialOsComponent<automaton::player::Movement>
{
  public:
      CSMovement() : m_moving(false), m_orientation(automaton::Quaternion::Create()), m_running(false) {}
      virtual ~CSMovement() {}

      void Initialise(worker::Connection& connection, worker::View& view, worker::EntityId entityId)
      {
        RegisterDefaultCallbacks();
      }

      static void ReflectType(Schematyc::CTypeDesc<CSMovement>& desc) {
        desc.SetGUID("{A3A27F9E-E4EA-42A3-9213-96350C3AE63D}"_cry_guid);
        desc.SetLabel("Movement");
        desc.SetEditorCategory("SpatialOS");
        desc.AddMember(&CSMovement::m_moving, 1, "Moving", "Moving", "", false);
        desc.AddMember(&CSMovement::m_orientation, 2, "Orientation", "Orientation", "", automaton::Quaternion::Create());
        desc.AddMember(&CSMovement::m_running, 3, "Running", "Running", "", false);

      }

      void ApplyComponentUpdate(const Component::Update& update) override
      {
        if (!update.moving().empty())
        {
            m_moving = *update.moving();
            m_moving_callbacks.Update(m_moving);
        	if (Schematyc::IObject *pObject = GetEntity()->GetSchematycObject())
        	{
        		pObject->ProcessSignal(SMovementMovingUpdatedSignal(m_moving));
        	}
        }
        if (!update.orientation().empty())
        {
            m_orientation = *update.orientation();
            m_orientation_callbacks.Update(m_orientation);
        	if (Schematyc::IObject *pObject = GetEntity()->GetSchematycObject())
        	{
        		pObject->ProcessSignal(SMovementOrientationUpdatedSignal(m_orientation));
        	}
        }
        if (!update.running().empty())
        {
            m_running = *update.running();
            m_running_callbacks.Update(m_running);
        	if (Schematyc::IObject *pObject = GetEntity()->GetSchematycObject())
        	{
        		pObject->ProcessSignal(SMovementRunningUpdatedSignal(m_running));
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
        entity.Add<Component>(Component::Data(m_moving, m_orientation, m_running));
        writeAcl.emplace(Component::ComponentId, m_spatialOs->GetSnapshotRequirementSet());
      }

      bool GetMoving() const { return m_moving; }

      void UpdateMoving(bool value)
      {
          Component::Update update;
          update.set_moving(value);
          m_connection->SendComponentUpdate<Component>(GetSpatialOsEntityId(), update);
      }

      automaton::Quaternion GetOrientation() const { return m_orientation; }

      void UpdateOrientation(automaton::Quaternion value)
      {
          Component::Update update;
          update.set_orientation(value);
          m_connection->SendComponentUpdate<Component>(GetSpatialOsEntityId(), update);
      }

      bool GetRunning() const { return m_running; }

      void UpdateRunning(bool value)
      {
          Component::Update update;
          update.set_running(value);
          m_connection->SendComponentUpdate<Component>(GetSpatialOsEntityId(), update);
      }

      size_t AddMovingCallback(std::function<void(bool)> cb) { return m_moving_callbacks.Add(cb); }
      bool RemoveMovingCallback(size_t key) { return m_moving_callbacks.Remove(key); }

      size_t AddOrientationCallback(std::function<void(automaton::Quaternion)> cb) { return m_orientation_callbacks.Add(cb); }
      bool RemoveOrientationCallback(size_t key) { return m_orientation_callbacks.Remove(key); }

      size_t AddRunningCallback(std::function<void(bool)> cb) { return m_running_callbacks.Add(cb); }
      bool RemoveRunningCallback(size_t key) { return m_running_callbacks.Remove(key); }



  private:
      bool m_moving;
      automaton::Quaternion m_orientation;
      bool m_running;
      CCallbackList<size_t, bool> m_moving_callbacks;
      CCallbackList<size_t, automaton::Quaternion> m_orientation_callbacks;
      CCallbackList<size_t, bool> m_running_callbacks;

};

namespace {
    static void RegisterCSMovement(Schematyc::IEnvRegistrar& registrar)
    {
    	Schematyc::CEnvRegistrationScope scope = registrar.Scope(IEntity::GetEntityScopeGUID());
        Schematyc::CEnvRegistrationScope componentScope = scope.Register(SCHEMATYC_MAKE_ENV_COMPONENT(CSMovement));
        {
            auto pFunction = SCHEMATYC_MAKE_ENV_FUNCTION(&CSMovement::UpdateMoving, "{698541C6-BD7D-4063-B174-E86219B654AE}"_cry_guid, "UpdateMoving");
            pFunction->BindInput(1, 1, "Moving");
            componentScope.Register(pFunction);
        }
        {
            auto pFunction = SCHEMATYC_MAKE_ENV_FUNCTION(&CSMovement::UpdateOrientation, "{9F37D6A3-141B-4D0A-92C8-20410FD6D61C}"_cry_guid, "UpdateOrientation");
            pFunction->BindInput(1, 1, "Orientation");
            componentScope.Register(pFunction);
        }
        {
            auto pFunction = SCHEMATYC_MAKE_ENV_FUNCTION(&CSMovement::UpdateRunning, "{CB0D3781-2C28-4DA3-9FDD-8D2C7D133A74}"_cry_guid, "UpdateRunning");
            pFunction->BindInput(1, 1, "Running");
            componentScope.Register(pFunction);
        }

        componentScope.Register(SCHEMATYC_MAKE_ENV_SIGNAL(SMovementMovingUpdatedSignal));
        componentScope.Register(SCHEMATYC_MAKE_ENV_SIGNAL(SMovementOrientationUpdatedSignal));
        componentScope.Register(SCHEMATYC_MAKE_ENV_SIGNAL(SMovementRunningUpdatedSignal));

    }
    CRY_STATIC_AUTO_REGISTER_FUNCTION(&RegisterCSMovement);
}