#pragma once

#include "ScopedViewCallbacks.h"
#include <improbable/worker.h>
#include "CryEntitySystem/IEntity.h"
#include <CryEntitySystem/IEntitySystem.h>

class ISpatialOs;

enum EESFailure
{
	eESF_None = 0,
	eESF_AlreadyDestroyed = 1,
	eESF_SpatialOsError = 2,
	eESF_Last
};

struct SEntitySpawnFuture
{
	explicit SEntitySpawnFuture(EntityId id, worker::Entity& entity) :
		id(id), entity(entity), m_failure(nullptr), m_success(nullptr)
	{
	}

	EntityId id;
	worker::Entity entity;
	std::function<void(EESFailure, worker::StatusCode, string, worker::Option<worker::EntityId>)> m_failure;
	std::function<void(worker::EntityId)> m_success;
};

class SpatialOsEntitySpawner
{
public:
	SpatialOsEntitySpawner(worker::Connection& connection, worker::View& view, ISpatialOs& spatialOs);

	worker::EntityId GetSpatialOsEntityId(EntityId cryEntityId) const;
	EntityId GetCryEntityId(worker::EntityId spatialEntityId) const;

	template <class T>
	void OnAddComponent(worker::AddComponentOp<typename T::Component> op)
	{
		worker::EntityId weid = op.EntityId;

		CryLog("Add component %d on to %d", T::Component::ComponentId, op.EntityId);
		IEntity *pEntity = gEnv->pEntitySystem->GetEntity(GetCryEntityId(weid));
		if (pEntity != nullptr)
		{
			T* t = pEntity->GetOrCreateComponentClass<T>();
			t->Init(m_connection, m_view, m_spatialOs, op.EntityId);
			auto update = T::Component::Update::FromInitialData(op.Data);
			t->ApplyComponentUpdate(update);
		}
	}

	template <class T>
	void OnRemoveComponent(worker::RemoveComponentOp op)
	{
		worker::EntityId weid = op.EntityId;
		IEntity *pEntity = gEnv->pEntitySystem->GetEntity(GetCryEntityId(weid));
		if (pEntity != nullptr)
		{
			T* t = pEntity->GetComponent<T>();
			if (t != nullptr)
			{
				pEntity->RemoveComponent(t);
			}
		}
	}

	template <typename T>
	void Register()
	{
		
		m_componentSpawns.emplace(T::Component::ComponentId,  [this] (void * op)
		{
			worker::AddComponentOp<typename T::Component>* pOp = reinterpret_cast<worker::AddComponentOp<typename T::Component> *>(op);
			OnAddComponent<T>(*pOp);
			delete pOp;
		});
		m_callbacks.Add(m_view.OnAddComponent<typename T::Component>([this] (const worker::AddComponentOp<typename T::Component>& op)
		{
			auto it = m_bufferedSpawns.find(op.EntityId);
			if (it == m_bufferedSpawns.end())
			{
				OnAddComponent<T>(op);
				return;
			}
			std::list<std::pair<worker::ComponentId, void *>>& list = it->second;
			worker::AddComponentOp<typename T::Component>* comp = new worker::AddComponentOp<typename T::Component>(op);
			list.push_back(std::make_pair<worker::ComponentId, void*>(static_cast<worker::ComponentId>(T::Component::ComponentId), reinterpret_cast<void *>(comp)));
		}));
		//m_callbacks.Add(m_view.OnAddComponent<typename T::Component>(std::bind(&SpatialOsEntitySpawner::OnAddComponent<T>, this, std::placeholders::_1)));
		m_callbacks.Add(m_view.OnRemoveComponent<typename T::Component>(std::bind(&SpatialOsEntitySpawner::OnRemoveComponent<T>, this, std::placeholders::_1)));
	}

	void RequestSpawn(SEntitySpawnFuture const & future);
	void ProcessEndCriticalSection();


private:
	template<typename T>
	struct cmp_req {
		bool operator()(const worker::RequestId<T>& a, const worker::RequestId<T>& b) const {
			return a.Id < b.Id;
		}
	};

	void OnAddEntity(const worker::AddEntityOp& Op);
	void OnRemoveEntity(const worker::RemoveEntityOp& Op);
	void OnReserveEntityId(const worker::ReserveEntityIdResponseOp& op);
	void PreemptEntitySpawn(EntityId entityId, worker::EntityId id);
	void SpawnCryEntity(worker::EntityId spatialOsEntityId);
	void OnCreateEntity(const worker::CreateEntityResponseOp& op);

	worker::View& m_view;
	worker::Connection& m_connection;
	ISpatialOs& m_spatialOs;
	ScopedViewCallbacks m_callbacks;

	bool m_inCriticalSection;
	std::map<EntityId, worker::EntityId> m_cryEntityIdToSpatialOsEntityId;
	std::map<worker::EntityId, EntityId> m_spatialOsEntityIdToCryEntityId;

	std::map<worker::RequestId<worker::ReserveEntityIdRequest>, SEntitySpawnFuture, cmp_req<worker::ReserveEntityIdRequest>> m_deferredSpatialSpawnsId;
	std::map<worker::RequestId<worker::CreateEntityRequest>, SEntitySpawnFuture, cmp_req<worker::CreateEntityRequest>> m_deferredSpatialSpawnsEntity;

	std::map<worker::ComponentId, std::function<void(void *)>> m_componentSpawns;
	std::map<worker::EntityId, std::list<std::pair<worker::ComponentId, void*>>> m_bufferedSpawns;

};
