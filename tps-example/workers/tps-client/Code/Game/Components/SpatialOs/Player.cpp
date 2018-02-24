#include "StdAfx.h"

#include "Player.h"
#include "Components/Player.h"

#include <CryRenderer/IRenderAuxGeom.h>

#include <automaton/player/bullet.h>
#include <improbable/standard_library.h>
#include "GamePlugin.h"
#include "SpatialOs/component/SpatialOsComponent.h"


CSPlayer::CSPlayer(): m_lastHeartbeatTime(0.f), m_authCallback(0), m_pPlayer(nullptr)
{
}

void CSPlayer::ProcessEvent(SEntityEvent& event)
{
	CRY_ASSERT(event.event == ENTITY_EVENT_UPDATE);
	if (!HasAuthority()) return;
	float time = gEnv->pTimer->GetCurrTime();
	if (time - m_lastHeartbeatTime < HeartbeatInterval) return;
	m_lastHeartbeatTime = time;
	Player::Update update;
	update.add_heartbeat(Heartbeat());
	m_connection->SendComponentUpdate<Player>(m_entityId, update);
}

uint64 CSPlayer::GetEventMask() const
{
	return BIT64(ENTITY_EVENT_UPDATE);
}

worker::Entity CreateBulletEntityTemplate(worker::List<std::string> workerAttrs, improbable::Coordinates coords, automaton::Quaternion rot, worker::EntityId owner)
{
	worker::Entity entity;
	improbable::WorkerAttributeSet localWorkerSet{ workerAttrs };
	improbable::WorkerRequirementSet localReqSet(worker::List<improbable::WorkerAttributeSet> {localWorkerSet});

	improbable::WorkerAttributeSet gameWorkerAttr{ worker::List<std::string>{ "GameWorker" } };
	improbable::WorkerAttributeSet clientWorkerAttr{ worker::List<std::string>{ "TPSClient" } };

	improbable::WorkerRequirementSet gameReqSet(worker::List<improbable::WorkerAttributeSet> {gameWorkerAttr});
	improbable::WorkerRequirementSet allRequirements{ worker::List<improbable::WorkerAttributeSet> { gameWorkerAttr, clientWorkerAttr } };

	worker::Map<uint32_t, improbable::WorkerRequirementSet> writeAcl
	{
		{ improbable::Position::ComponentId, localReqSet },
		{ improbable::EntityAcl::ComponentId, gameReqSet },
		{ Bullet::ComponentId, localReqSet }
	};
	entity.Add<improbable::EntityAcl>(improbable::EntityAcl::Data(allRequirements, writeAcl));
	entity.Add<improbable::Position>(improbable::Position::Data(coords));
	entity.Add<Bullet>(Bullet::Data(owner, rot));
	entity.Add<improbable::Metadata>(improbable::Metadata::Data("Bullet"));
	return entity;
}

void CSPlayer::FireBullet(const QuatTS& origin, EntityId id)
{
	worker::List<std::string> attrs { "workerId:" + m_spatialOs->GetWorkerId() };
	Vec3 pos = origin.t;
	improbable::Coordinates coords(pos.x, pos.z, pos.y);
	automaton::Quaternion rot(origin.q.w, origin.q.v.x, origin.q.v.y, origin.q.v.z);
	worker::Entity entity = CreateBulletEntityTemplate(attrs, coords, rot, GetSpatialOsEntityId());
	SEntitySpawnFuture future(id, entity);
	future.m_failure = [this] (EESFailure failure, worker::StatusCode sc, string msg, worker::Option<worker::EntityId> entityId)
	{
		if (failure == eESF_AlreadyDestroyed)
		{
			m_connection->SendDeleteEntityRequest(*entityId, 3000);
		}
	};
	m_spatialOs->GetEntitySpawner()->RequestSpawn(future);
}

void CSPlayer::SendKillEvent(worker::EntityId killedPlayer) const
{
	Component::Update update;
	update.add_killed_player(KilledPlayer(killedPlayer));
	m_connection->SendComponentUpdate<Component>(GetSpatialOsEntityId(), update);
}

void CSPlayer::Respawn()
{
	CPlayerComponent *pPlayer = GetEntity()->GetComponent<CPlayerComponent>();
	CSpatialOsComponent *pSpatialOs = GetEntity()->GetComponent<CSpatialOsComponent>();
	if (pPlayer != nullptr && pSpatialOs != nullptr)
	{
		pPlayer->Revive();
		if (pSpatialOs->HasPositionAuthority()) pSpatialOs->UpdatePosition(GetEntity()->GetPos());
	}
}

void CSPlayer::Initialise(worker::Connection& /*connection*/, CSpatialOsView& /*view*/, worker::EntityId /*entityId*/)
{
	RegisterDefaultCallbacks();
	IEntity *pEntity = GetEntity();
	m_pPlayer = pEntity->CreateComponentClass<CPlayerComponent>();
	if (HasAuthority())
	{
		m_pPlayer->InitialiseLocalPlayer();
	}
	else
	{
		m_authCallback = OnAuthorityChange(std::bind(&CSPlayer::OnAuthority, this, std::placeholders::_1));
	}

}

void CSPlayer::OnAuthority(bool auth)
{
	if (auth)
	{
		CryLog("Received authority on player component over %s", GetEntity()->GetName());
		if (m_pPlayer != nullptr)
		{
			m_pPlayer->InitialiseLocalPlayer();
			// Remove the callback as this won't work if it happens again
			RemoveOnAuthorityChange(m_authCallback);
			m_authCallback = 0;
		}
	}
}

void CSPlayer::WriteComponent(worker::Entity& entity, worker::Map<std::uint32_t, WorkerRequirementSet>& writeAcl)
{

}

void CSPlayer::ApplyComponentUpdate(const Player::Update& update)
{

}

