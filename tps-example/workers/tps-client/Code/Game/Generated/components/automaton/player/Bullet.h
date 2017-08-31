#pragma once

#include <improbable/worker.h>
#include <CrySchematyc/CoreAPI.h>
#include <automaton/player/bullet.h>

#include "SpatialOs/CallbackList.h"
#include "SpatialOs/SpatialOsComponent.h"
#include "SpatialOs/SpatialOs.h"
#include "Generated/Types.h"
#include <automaton/quaternion.h>


struct SBulletCreatorUpdatedSignal {
    SBulletCreatorUpdatedSignal() = default;
    SBulletCreatorUpdatedSignal(worker::EntityId creator) : m_creator(creator) {}
    worker::EntityId m_creator;
};

static inline void ReflectType(Schematyc::CTypeDesc<SBulletCreatorUpdatedSignal>& desc)
{
    desc.SetGUID("{4BE223C1-6A08-44A4-AB5D-E78765815B66}"_cry_guid);
    desc.SetLabel("OnBulletCreatorChanged");
    desc.AddMember(&SBulletCreatorUpdatedSignal::m_creator, 1, "BulletCreator", "BulletCreator", "", 0);
}

struct SBulletRotationUpdatedSignal {
    SBulletRotationUpdatedSignal() = default;
    SBulletRotationUpdatedSignal(automaton::Quaternion rotation) : m_rotation(rotation) {}
    automaton::Quaternion m_rotation;
};

static inline void ReflectType(Schematyc::CTypeDesc<SBulletRotationUpdatedSignal>& desc)
{
    desc.SetGUID("{8C1686B0-72C7-45F8-B42A-B925F22DB5FD}"_cry_guid);
    desc.SetLabel("OnBulletRotationChanged");
    desc.AddMember(&SBulletRotationUpdatedSignal::m_rotation, 2, "BulletRotation", "BulletRotation", "", automaton::Quaternion::Create());
}


class CSBullet: public ISpatialOsComponent<automaton::player::Bullet>
{
  public:
      CSBullet() : m_creator(0), m_rotation(automaton::Quaternion::Create()) {}
      virtual ~CSBullet() {}

      void Initialise(worker::Connection& connection, worker::View& view, worker::EntityId entityId)
      {
        RegisterDefaultCallbacks();
      }

      static void ReflectType(Schematyc::CTypeDesc<CSBullet>& desc) {
        desc.SetGUID("{EBC53A54-4853-4FB9-B0EE-0CFE35BBD502}"_cry_guid);
        desc.SetLabel("Bullet");
        desc.SetEditorCategory("SpatialOS");
        desc.AddMember(&CSBullet::m_creator, 1, "Creator", "Creator", "", 0);
        desc.AddMember(&CSBullet::m_rotation, 2, "Rotation", "Rotation", "", automaton::Quaternion::Create());

      }

      void ApplyComponentUpdate(const Component::Update& update) override
      {
        if (!update.creator().empty())
        {
            m_creator = *update.creator();
            m_creator_callbacks.Update(m_creator);
        	if (Schematyc::IObject *pObject = GetEntity()->GetSchematycObject())
        	{
        		pObject->ProcessSignal(SBulletCreatorUpdatedSignal(m_creator));
        	}
        }
        if (!update.rotation().empty())
        {
            m_rotation = *update.rotation();
            m_rotation_callbacks.Update(m_rotation);
        	if (Schematyc::IObject *pObject = GetEntity()->GetSchematycObject())
        	{
        		pObject->ProcessSignal(SBulletRotationUpdatedSignal(m_rotation));
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
        entity.Add<Component>(Component::Data(m_creator, m_rotation));
        writeAcl.emplace(Component::ComponentId, m_spatialOs->GetSnapshotRequirementSet());
      }

      worker::EntityId GetCreator() const { return m_creator; }

      void UpdateCreator(worker::EntityId value)
      {
          Component::Update update;
          update.set_creator(value);
          m_connection->SendComponentUpdate<Component>(GetSpatialOsEntityId(), update);
      }

      automaton::Quaternion GetRotation() const { return m_rotation; }

      void UpdateRotation(automaton::Quaternion value)
      {
          Component::Update update;
          update.set_rotation(value);
          m_connection->SendComponentUpdate<Component>(GetSpatialOsEntityId(), update);
      }

      size_t AddCreatorCallback(std::function<void(worker::EntityId)> cb) { return m_creator_callbacks.Add(cb); }
      bool RemoveCreatorCallback(size_t key) { return m_creator_callbacks.Remove(key); }

      size_t AddRotationCallback(std::function<void(automaton::Quaternion)> cb) { return m_rotation_callbacks.Add(cb); }
      bool RemoveRotationCallback(size_t key) { return m_rotation_callbacks.Remove(key); }



  private:
      worker::EntityId m_creator;
      automaton::Quaternion m_rotation;
      CCallbackList<size_t, worker::EntityId> m_creator_callbacks;
      CCallbackList<size_t, automaton::Quaternion> m_rotation_callbacks;

};

namespace {
    static void RegisterCSBullet(Schematyc::IEnvRegistrar& registrar)
    {
    	Schematyc::CEnvRegistrationScope scope = registrar.Scope(IEntity::GetEntityScopeGUID());
        Schematyc::CEnvRegistrationScope componentScope = scope.Register(SCHEMATYC_MAKE_ENV_COMPONENT(CSBullet));
        {
            auto pFunction = SCHEMATYC_MAKE_ENV_FUNCTION(&CSBullet::UpdateCreator, "{D5C87B80-01DF-4158-ADB5-89B7AC06960A}"_cry_guid, "UpdateCreator");
            pFunction->BindInput(1, 1, "Creator");
            componentScope.Register(pFunction);
        }
        {
            auto pFunction = SCHEMATYC_MAKE_ENV_FUNCTION(&CSBullet::UpdateRotation, "{1B4F9CB6-0A6F-4D69-99BB-E641DD9D1CAC}"_cry_guid, "UpdateRotation");
            pFunction->BindInput(1, 1, "Rotation");
            componentScope.Register(pFunction);
        }

        componentScope.Register(SCHEMATYC_MAKE_ENV_SIGNAL(SBulletCreatorUpdatedSignal));
        componentScope.Register(SCHEMATYC_MAKE_ENV_SIGNAL(SBulletRotationUpdatedSignal));

    }
    CRY_STATIC_AUTO_REGISTER_FUNCTION(&RegisterCSBullet);
}