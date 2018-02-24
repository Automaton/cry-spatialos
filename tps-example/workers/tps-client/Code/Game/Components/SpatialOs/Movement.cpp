#include "StdAfx.h"

#include "Movement.h"
#include "Player.h"

CSMovement::CSMovement(): m_moving(false), m_running(false)
{
}

CSMovement::~CSMovement()
{
}

void CSMovement::ApplyComponentUpdate(const automaton::player::Movement::Update& update)
{
	if (!update.moving().empty())
	{
		m_moving = *update.moving();
		m_movingCallbacks.Update(m_moving);
	}
	if (!update.orientation().empty())
	{
		const automaton::Quaternion& quat = *update.orientation();
		m_lookOrientation = Quat(quat.w(), quat.x(), quat.y(), quat.z());
		m_orientationCallbacks.Update(m_lookOrientation);
	}
	if (!update.running().empty())
	{
		m_running = *update.running();
	}
}

void CSMovement::Initialise(worker::Connection& connection, CSpatialOsView& view, worker::EntityId entityId)
{
	RegisterDefaultCallbacks();
	if (CSpatialOsComponent *pComp = GetEntity()->GetComponent<CSpatialOsComponent>())
	{
		pComp->SetWritePosition(false);
		pComp->AddTransformCallback([this] (CryTransform::CTransform transform)
		{
			m_spatialOsPosition = transform.GetTranslation();
		});
	}
}

void CSMovement::WriteComponent(worker::Entity& entity, worker::Map<std::uint32_t, WorkerRequirementSet>& writeAcl)
{
	entity.Add<Movement>(Movement::Data(m_moving, automaton::Quaternion(m_lookOrientation.w, m_lookOrientation.v.x, m_lookOrientation.v.y, m_lookOrientation.v.z), m_running));
	const improbable::WorkerAttributeSet gameWorkerAttributeSet{ worker::List<std::string>{"GameWorker"} };
	const improbable::WorkerRequirementSet gameWorkerRequirementSet{ { gameWorkerAttributeSet } };
	writeAcl.emplace(Component::ComponentId, gameWorkerRequirementSet);
}

uint64 CSMovement::GetEventMask() const
{
	return BIT64(ENTITY_EVENT_UPDATE);
}

void CSMovement::ProcessEvent(SEntityEvent& event)
{
	CRY_ASSERT_MESSAGE(event.event == ENTITY_EVENT_UPDATE, "Only expected ENTITY_EVENT_UPDATE");
	// Do not send movement updates unless we have authority over the movement component
	IEntity *pEntity = GetEntity();
	IPhysicalEntity *pPhysicalEntity = pEntity->GetPhysics();
	if (!pEntity || !pPhysicalEntity) return;
	CSPlayer *pSPlayer = pEntity->GetComponent<CSPlayer>();
	CSpatialOsComponent *pSpatialOs = pEntity->GetComponent<CSpatialOsComponent>();
	if (pSPlayer == nullptr || pSpatialOs == nullptr)
	{
		// Cannot update until the player component exists and the position exists
		return;
	}
	CPlayerComponent *pPlayer = pSPlayer->GetPlayerComponent();
	Vec3 position = pEntity->GetWorldPos();
	float posDist = position.GetDistance(m_spatialOsPosition);
	// Can only update position with authority
	if (!pSpatialOs->HasPositionAuthority() || !HasAuthority())
	{
		pPlayer->SetLookOrientation(m_lookOrientation);
		Vec3 targetPos = m_spatialOsPosition;
		if (targetPos.IsZeroFast()) return;
		if (posDist > TeleportThreshold)
		{
			GetEntity()->SetPos(targetPos);
			return;
		}
		float moveSpeed = 20.5f;
		if (m_running) moveSpeed *= 2;
		float fTime = gEnv->pTimer->GetFrameTime();
		if (posDist > moveSpeed * fTime)
		{
			// Request movement in the required direction for the player
			Vec3 direction = (targetPos - position) * moveSpeed * fTime;
			pPlayer->GetCharacterController()->AddVelocity(direction);
		}
		return;
	}
#pragma region Update movement component
	bool movementDirty = false;
	automaton::player::Movement::Update movementUpdate;
	pe_status_living living;
	if (pPhysicalEntity->GetStatus(&living) != 0)
	{
		bool moving = living.vel.len() > MovingVelocityThreshold;
		if (moving != m_moving)
		{
			movementUpdate.set_moving(moving);
			movementDirty = true;
		}
	}
	Quat orientation = pPlayer->GetLookOrientation();
	if (abs(orientation.w - m_lookOrientation.w) > OrientationWTolerance && orientation.v.GetDistance(m_lookOrientation.v) > OrientationVTolerance)
	{
		movementUpdate.set_orientation(automaton::Quaternion(orientation.w, orientation.v.x, orientation.v.y, orientation.v.z));
		movementDirty = true;
	}
	if (pPlayer->IsSprinting() != m_running)
	{
		movementDirty = true;
		movementUpdate.set_running(pPlayer->IsSprinting());
	}
	if (movementDirty)
	{
		m_connection->SendComponentUpdate<automaton::player::Movement>(GetSpatialOsEntityId(), movementUpdate);
	}
#pragma endregion
#pragma region Update position component
	if (posDist  > PositionTolerance)
	{
		pSpatialOs->UpdatePosition(position);
	}
#pragma endregion
}

