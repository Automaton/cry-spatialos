#include "StdAfx.h"

#include "SpatialOsEntitySpawner.h"
#include "improbable/collections.h"

#include <CryEntitySystem/IEntitySystem.h>
#include "Components/SpatialOs/Movement.h"
#include "Components/SpatialOs/Bullet.h"
#include "Components/SpatialOs/PlayerScore.h"
#include "Generated/components/automaton/Tree.h"
#include "Generated/components/automaton/Spawner.h"
#include "Components/SpatialOs/Player.h"

SpatialOsEntitySpawner::SpatialOsEntitySpawner(worker::Connection& connection, worker::View& view, ISpatialOs& spatialOs)
	: m_view(view), m_connection(connection), m_spatialOs(spatialOs), m_callbacks(view)
{
	m_callbacks.Add(
		m_view.OnAddEntity(std::bind(&SpatialOsEntitySpawner::OnAddEntity, this, std::placeholders::_1)));
	m_callbacks.Add(
		m_view.OnRemoveEntity(std::bind(&SpatialOsEntitySpawner::OnRemoveEntity, this, std::placeholders::_1)));
	m_callbacks.Add(m_view.OnReserveEntityIdResponse(std::bind(&SpatialOsEntitySpawner::OnReserveEntityId, this, std::placeholders::_1)));
	m_callbacks.Add(m_view.OnCreateEntityResponse(std::bind(&SpatialOsEntitySpawner::OnCreateEntity, this, std::placeholders::_1)));
	m_callbacks.Add(m_view.OnCriticalSection([this] (const worker::CriticalSectionOp& op) 
	{
		bool oldCritSection = m_inCriticalSection;
		m_inCriticalSection = op.InCriticalSection;
		if (oldCritSection && !m_inCriticalSection)
		{
			ProcessEndCriticalSection();
		}
	}));
	m_callbacks.Add(m_view.OnAddComponent<Metadata>([this] (worker::AddComponentOp<Metadata> op)
	{
		auto it = m_bufferedSpawns.find(op.EntityId);
		if (it != m_bufferedSpawns.end())
		{
			worker::AddComponentOp<Metadata> * pOp = new worker::AddComponentOp<Metadata>(op);
			it->second.push_back(std::make_pair<worker::ComponentId, void *>(static_cast<worker::ComponentId>(Metadata::ComponentId), reinterpret_cast<void*>(pOp)));
		}
	}));
	m_callbacks.Add(m_view.OnAddComponent<Position>([this](worker::AddComponentOp<Position> op)
	{
		auto it = m_bufferedSpawns.find(op.EntityId);
		if (it != m_bufferedSpawns.end())
		{
			worker::AddComponentOp<Position> * pOp = new worker::AddComponentOp<Position>(op);
			it->second.push_back(std::make_pair<worker::ComponentId, void *>(static_cast<worker::ComponentId>(Position::ComponentId), reinterpret_cast<void*>(pOp)));
		}
	}));
	RegisterComponents();
}

void SpatialOsEntitySpawner::SpawnCryEntity(worker::EntityId spatialOsEntityId)
{
	if (m_inCriticalSection)
	{
		m_bufferedSpawns.emplace(spatialOsEntityId, std::list<std::pair<worker::ComponentId, void *>>());
		return;
	}
	SEntitySpawnParams params;
	params.pClass = gEnv->pEntitySystem->GetClassRegistry()->GetDefaultClass();
	params.sLayerName = "Main";
	std::string entityName = "SpatialOsEntity-" + std::to_string(spatialOsEntityId);
	params.sName = entityName.c_str();

	IEntity* entity = gEnv->pEntitySystem->SpawnEntity(params, true);
	if (entity == nullptr)
	{
		CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, "Failed to spawn SpatialOS entity: %d. SpawnEntity returned nullptr", spatialOsEntityId);
		return;
	}
	EntityId cryEntityId = entity->GetId();

	m_cryEntityIdToSpatialOsEntityId.emplace(cryEntityId, spatialOsEntityId);
	m_spatialOsEntityIdToCryEntityId.emplace(spatialOsEntityId, cryEntityId);
	CSpatialOsComponent *pSpatialOsComponent = entity->GetOrCreateComponentClass<CSpatialOsComponent>();
	if (pSpatialOsComponent)
	{
		pSpatialOsComponent->Init(spatialOsEntityId, m_view, m_connection, m_spatialOs);
	}
	else
	{
		CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, "Failed to add base SpatialOS component to entity %d, SpatialOS entity %d", cryEntityId, spatialOsEntityId);
	}
}

void SpatialOsEntitySpawner::OnCreateEntity(const worker::CreateEntityResponseOp& op)
{
	auto it = m_deferredSpatialSpawnsEntity.find(op.RequestId);
	if (it == m_deferredSpatialSpawnsEntity.end())
		return;
	SEntitySpawnFuture future = it->second;
	EntityId id = future.id;
	m_deferredSpatialSpawnsEntity.erase(it);
	if (gEnv->pEntitySystem->GetEntity(id) == nullptr)
	{
		// Entity was deleted by the time the response was received
		CryComment("%d was deleted before the SpatialOS entity was created, SpatialOS response: %d", id, op.StatusCode);
		if (future.m_failure != nullptr)
		{
			future.m_failure(eESF_AlreadyDestroyed, op.StatusCode, string(op.Message.c_str()), op.EntityId);
		}
		return;
	}
	if (op.StatusCode == worker::StatusCode::kSuccess)
	{
		if (future.m_success != nullptr)
			future.m_success(*op.EntityId);
	}
	else
	{
		CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "Failed to create SpatialOS entity for entity ID: %d. Status: %d, Reason: %s", id, op.StatusCode, op.Message);
		if (future.m_failure != nullptr)
		{
			future.m_failure(eESF_SpatialOsError, op.StatusCode, string(op.Message.c_str()), op.EntityId);
		}
		// Remove entity from maps
		auto iter = m_cryEntityIdToSpatialOsEntityId.find(id);
		if (iter != m_cryEntityIdToSpatialOsEntityId.end())
		{
			m_spatialOsEntityIdToCryEntityId.erase(iter->second);
			m_cryEntityIdToSpatialOsEntityId.erase(iter);
		}
	}
}

void SpatialOsEntitySpawner::RegisterComponents()
{
	Register<CSPlayerScore>();
	Register<CSPlayer>();
	Register<CSMovement>();
	Register<CSBullet>();
	Register<CSTree>();
	Register<CSSpawner>();
}

EntityId SpatialOsEntitySpawner::GetCryEntityId(worker::EntityId spatialEntityId) const
{
	auto it = m_spatialOsEntityIdToCryEntityId.find(spatialEntityId);
	if (it == m_spatialOsEntityIdToCryEntityId.end())
	{
		return INVALID_ENTITYID;
	}
	return it->second;
}

void SpatialOsEntitySpawner::OnAddEntity(const worker::AddEntityOp& Op)
{
	CryLog("SpawnEntity: %d", Op.EntityId);
	// Check if this entity already exists i.e. it was created by us and preempted
	auto it = m_spatialOsEntityIdToCryEntityId.find(Op.EntityId);
	if (it == m_spatialOsEntityIdToCryEntityId.end())
		SpawnCryEntity(Op.EntityId);
	else
	{
		CryComment("%d was already spawned", Op.EntityId);
	}
}

void SpatialOsEntitySpawner::OnRemoveEntity(const worker::RemoveEntityOp& Op)
{
	auto iterator = m_spatialOsEntityIdToCryEntityId.find(Op.EntityId);
	if (iterator == m_spatialOsEntityIdToCryEntityId.end()) {
		return;
	}
	EntityId cryEntityId = iterator->second;
	gEnv->pEntitySystem->RemoveEntity(cryEntityId);
	m_cryEntityIdToSpatialOsEntityId.erase(cryEntityId);
	m_spatialOsEntityIdToCryEntityId.erase(Op.EntityId);
}

void SpatialOsEntitySpawner::OnReserveEntityId(const worker::ReserveEntityIdResponseOp& op)
{
	auto it = m_deferredSpatialSpawnsId.find(op.RequestId);
	if (it == m_deferredSpatialSpawnsId.end())
		return;
	SEntitySpawnFuture future = it->second;
	EntityId id = future.id;
	m_deferredSpatialSpawnsId.erase(it);
	if (gEnv->pEntitySystem->GetEntity(id) == nullptr)
	{
		// Entity was deleted by the time the response was received
		CryComment("%d was deleted before the SpatialOS entity ID was reserved, SpatialOS response: %d", id, op.StatusCode);
		if (future.m_failure != nullptr)
		{
			future.m_failure(eESF_AlreadyDestroyed, op.StatusCode, string(op.Message.c_str()), op.EntityId);
		}
		return;
	}
	if (op.StatusCode == worker::StatusCode::kSuccess)
	{
		PreemptEntitySpawn(id, *op.EntityId);
		auto request = m_connection.SendCreateEntityRequest(future.entity, op.EntityId, 3000);
		m_deferredSpatialSpawnsEntity.emplace(request, future);	
	}
	else
	{
		CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "Failed to reserve SpatialOS entity for entity ID: %d. Status: %d, Reason: %s", id, op.StatusCode, op.Message);
		if (future.m_failure != nullptr)
		{
			future.m_failure(eESF_SpatialOsError, op.StatusCode, string(op.Message.c_str()), op.EntityId);
		}
	}
}

void SpatialOsEntitySpawner::PreemptEntitySpawn(EntityId entityId, worker::EntityId id)
{	
	m_cryEntityIdToSpatialOsEntityId.emplace(entityId, id);
	m_spatialOsEntityIdToCryEntityId.emplace(id, entityId);
}

void SpatialOsEntitySpawner::RequestSpawn(SEntitySpawnFuture const & future)
{
	m_deferredSpatialSpawnsId.emplace(m_connection.SendReserveEntityIdRequest(3000), future);
}

worker::EntityId SpatialOsEntitySpawner::GetSpatialOsEntityId(EntityId cryEntityId) const
{
	auto iterator = m_cryEntityIdToSpatialOsEntityId.find(cryEntityId);
	if (iterator == m_cryEntityIdToSpatialOsEntityId.end())
	{
		return -1;
	}
	return iterator->second;
}

void SpatialOsEntitySpawner::ProcessEndCriticalSection()
{
	for (auto & pair : m_bufferedSpawns)
	{
		worker::EntityId id = pair.first;
		std::list<std::pair<worker::ComponentId, void*>>& list = pair.second;
		// First look for a metadata component
		IEntityClass *pClass = gEnv->pEntitySystem->GetClassRegistry()->GetDefaultClass();
		string name;
		name.Format("SpatialOS-%d", id);
		auto it = std::find_if(list.begin(), list.end(), [](std::pair<worker::ComponentId, void*> p)
		{
			return p.first == improbable::Metadata::ComponentId;
		});
		worker::AddComponentOp<improbable::Metadata>* metadataOp = nullptr;
		worker::AddComponentOp<improbable::Position>* positionOp = nullptr;
		if (it != list.end())
		{
			// Metadata component exists, try to look up metadata + class
			void *ptr = it->second;
			metadataOp = reinterpret_cast<worker::AddComponentOp<improbable::Metadata> *>(ptr);
			const Metadata::Data& data = metadataOp->Data;
			IEntityClass *lookupClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass(data.entity_type().c_str());
			CryLog("Spawning with entity class %s", data.entity_type().c_str());
			if (lookupClass != nullptr)
			{
				pClass = lookupClass;
			} 
			else
			{
				CryLog("Failed to find entity class: %s", data.entity_type().c_str());
			}
			name.Format("%s-%d", data.entity_type().c_str(), id);
			list.erase(it);
		}
		it = std::find_if(list.begin(), list.end(), [](std::pair<worker::ComponentId, void*> p)
		{
			return p.first == improbable::Position::ComponentId;
		});
		if (it != list.end())
		{
			// Position component exists, save this for the SpatialOsComponent
			void *ptr = it->second;
		    positionOp = reinterpret_cast<worker::AddComponentOp<improbable::Position> *>(ptr);
			list.erase(it);
		}
		SEntitySpawnParams params;
		params.sName = name;
		params.pClass = pClass;

		IEntity* entity = gEnv->pEntitySystem->SpawnEntity(params, true);
		if (entity == nullptr)
		{
			CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, "Failed to spawn SpatialOS entity: %d. SpawnEntity returned nullptr", id);
			return;
		}
		EntityId cryEntityId = entity->GetId();

		m_cryEntityIdToSpatialOsEntityId.emplace(cryEntityId, id);
		m_spatialOsEntityIdToCryEntityId.emplace(id, cryEntityId);

		// First spawn the SpatialOsComponent if it didn't exist
		if (CSpatialOsComponent *pSpatialOs = entity->GetOrCreateComponentClass<CSpatialOsComponent>())
		{
			pSpatialOs->Init(id, m_view, m_connection, m_spatialOs);
			if (metadataOp) pSpatialOs->OnAddMetadata(*metadataOp);
			if (positionOp) pSpatialOs->OnAddPosition(*positionOp);
			else CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, "Did not add position component to: %d. Did not receive a AddComponentOp for Position component", id);
		}
		if (metadataOp) delete metadataOp;
		if (positionOp) delete positionOp;
		// Now process all other components
		for (auto& listPair : list)
		{
			worker::ComponentId compId = listPair.first;
			auto compIt = m_componentSpawns.find(compId);
			if (compIt != m_componentSpawns.end())
			{
				compIt->second(listPair.second);
			}
		}
		list.clear();
	}
	m_bufferedSpawns.clear();
}
