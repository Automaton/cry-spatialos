#include "StdAfx.h"

#include "Bullet.h"
#include "GamePlugin.h"
#include "Components/Bullet.h"
#include "Player.h"


CSBullet::CSBullet() : m_id(INVALID_ENTITYID), m_creator(0), m_transformReady(false), m_bulletReady(false), m_pSpatialOs(nullptr)
{
}

uint64 CSBullet::GetEventMask() const
{
	return BIT64(ENTITY_EVENT_INIT) | BIT64(ENTITY_EVENT_DONE) | BIT64(ENTITY_EVENT_UPDATE) | BIT64(ENTITY_EVENT_COLLISION);
}

void CSBullet::Initialise(worker::Connection& connection, CSpatialOsView& view, worker::EntityId entityId)
{
	RegisterDefaultCallbacks();
	InitPosition();
}

void CSBullet::InitPosition()
{
	if (m_pSpatialOs != nullptr) return;
	m_pSpatialOs = GetEntity()->GetComponent<CSpatialOsComponent>();
	if (m_pSpatialOs != nullptr)
	{
		if (m_pSpatialOs->IsReady())
			SetTransformReady(m_pSpatialOs->GetReadyState());
		else
			m_pSpatialOs->AddReadyStateCallback(std::bind(&CSBullet::SetTransformReady, this, std::placeholders::_1));
	}
}

void CSBullet::ApplyComponentUpdate(const automaton::player::Bullet::Update& update)
{
	if (!update.creator().empty())
	{
		m_creator = *update.creator().data();
	}
	if (!update.rotation().empty())
	{
		auto rot = *update.rotation().data();
		m_rotation = Quat(rot.w(), rot.x(), rot.y(), rot.z());
	}
	SetBulletReady();
}

void CSBullet::WriteComponent(worker::Entity& entity, worker::Map<std::uint32_t, improbable::WorkerRequirementSet>& writeAcl)
{
	entity.Add<Component>(Bullet::Data(m_creator, automaton::Quaternion(m_rotation.w, m_rotation.v.x, m_rotation.v.y, m_rotation.v.z)));
	const improbable::WorkerAttributeSet gameWorkerAttributeSet{ worker::List<std::string>{"GameWorker"} };
	const improbable::WorkerRequirementSet gameWorkerRequirementSet{ { gameWorkerAttributeSet } };
	writeAcl.emplace(Component::ComponentId, gameWorkerRequirementSet);
}

void CSBullet::OnReady()
{
	IEntity *pEntity = GetEntity();
	CSpatialOsComponent *pSpatialOsComponent = pEntity->GetComponent<CSpatialOsComponent>();
	CRY_ASSERT_MESSAGE(pSpatialOsComponent, "Position component should always exist on the entity by this point!");
	// If bullet doesn't already exist, create it!
	if (!pEntity->GetComponent<CBulletComponent>())
	{
		const float bulletScale = 0.05f;
		pEntity->SetPosRotScale(GetEntity()->GetWorldPos(), m_rotation, Vec3(bulletScale));
		// See Bullet.cpp, bullet is propelled in  the rotation and position the entity was spawned with
		pEntity->CreateComponentClass<CBulletComponent>();
	}
}

void CSBullet::SetTransformReady(int readyState)
{
	if ((readyState & eCRS_PositionReady) == 0) return;
	bool wasReady = m_transformReady;
	m_transformReady = true;
	if (!wasReady && m_bulletReady)
	{
		OnReady();
	}
}

void CSBullet::SetBulletReady()
{
	bool wasReady = m_bulletReady;
	m_bulletReady = true;
	m_isComponentReady = true;
	if (!wasReady && m_transformReady)
	{
		OnReady();
	}
}

void CSBullet::ProcessEvent(SEntityEvent& event)
{
	if (event.event == ENTITY_EVENT_DONE)
	{
		worker::EntityId id = GetSpatialOsEntityId();
		if (HasAuthority())
		{
			m_spatialOs->SendWorkerCommand()->DeleteEntity(id,
				[id](bool success, std::string errorMsg)
			{
				if (!success)
				{
					CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "Failed to delete entity %d: %s", id, errorMsg.c_str());
				}
			});
		}
	}
	else if (event.event == ENTITY_EVENT_UPDATE)
	{
		InitPosition();
		if (m_pSpatialOs != nullptr && m_pSpatialOs->HasPositionAuthority() && m_bulletReady && m_pSpatialOs->IsReady())
		{
			m_pSpatialOs->UpdatePosition(GetEntity()->GetPos());
;		}
	}
	else if (event.event == ENTITY_EVENT_COLLISION)
	{
		if (HasAuthority())
		{
			EventPhysCollision* physCollision = reinterpret_cast<EventPhysCollision*>(event.nParam[0]);
			IEntity *pTarget = physCollision->iForeignData[1] == PHYS_FOREIGN_ID_ENTITY ? static_cast<IEntity *>(physCollision->pForeignData[1]) : nullptr;
			if (pTarget != nullptr && pTarget != GetEntity())
			{
				// Look up owner of the bullet
				EntityId ownerId = m_spatialOs->GetEntitySpawner()->GetCryEntityId(m_creator);
				IEntity *pEntity = gEnv->pEntitySystem->GetEntity(ownerId);
				if (pEntity == nullptr)
				{
					CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "Failed to find CRYENGINE entity for bullet creator(%d), cannot mark as a valid hit", m_creator);
					return;
				}
				CSPlayer *pShooter = pEntity->GetComponent<CSPlayer>();
				if (pShooter == nullptr)
				{
					CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "Failed to find player component(%d) for bullet creator(%d), cannot mark as a valid hit", ownerId, m_creator);
					return;
				}
				if (pShooter->HasAuthority())
				{
					CSPlayer *pKilled = pTarget->GetComponent<CSPlayer>();
					if (pKilled == nullptr)
					{
						return;
					}
					pShooter->SendKillEvent(pKilled->GetSpatialOsEntityId());
				}
			}

		}
		gEnv->pEntitySystem->RemoveEntity(GetEntityId());
	}
}
