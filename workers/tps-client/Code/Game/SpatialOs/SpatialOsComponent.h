#pragma once

#include <improbable/worker.h>
#include "ScopedViewCallbacks.h"
#include "CallbackList.h"
#include "ComponentSerialiser.h"

class CSpatialOs;

template <typename C>
class ISpatialOsComponent: public IEntityComponent, CComponentSerialiser
{

	friend class SpatialOsEntitySpawner;

public:
	typedef C Component;

	ISpatialOsComponent() : m_connection(nullptr), m_view(nullptr), m_spatialOs(nullptr), m_entityId(0), m_callbacks(nullptr), m_hasAuthority(false), m_isComponentReady(false)
	{
	}

	virtual void Initialise(worker::Connection& connection, worker::View& view, worker::EntityId entityId) {};

	virtual worker::ComponentId GetComponentId() { return C::ComponentId; }

	worker::EntityId GetSpatialOsEntityId() const
	{
		return m_entityId;
	}

	bool HasAuthority() const
	{
		return m_hasAuthority;
	}

	bool IsComponentReady() const
	{
		return m_isComponentReady;
	}

	size_t OnAuthorityChange(std::function<void(bool)> callback)
	{
		return m_authorityChangeCallbacks.Add(callback);
	}

	size_t OnComponentReady(std::function<void()> callback)
	{
		return m_componentReadyCallbacks.Add(callback);
	}

	void RemoveOnAuthorityChange(size_t key)
	{
		m_authorityChangeCallbacks.Remove(key);
	}

	void RemoveOnComponentReady(size_t key)
	{
		m_componentReadyCallbacks.Remove(key);
	}

	void Init(worker::Connection& connection, worker::View& view, CSpatialOs& spatialOs, worker::EntityId entityId)
	{
		m_connection = &connection;
		m_view = &view;
		m_spatialOs = &spatialOs;
		m_entityId = entityId;
		m_callbacks.reset(new ScopedViewCallbacks(view));
		auto it = view.Entities.find(entityId);
		if (it != view.Entities.end())
		{
			m_hasAuthority = it->second.HasAuthority<C>();
			m_authorityChangeCallbacks.Update(m_hasAuthority);
		}
		Initialise(connection, view, entityId);
	}

	virtual void ApplyComponentUpdate(const typename C::Update& update)
	{
	}

protected:
	worker::Connection* m_connection;
	worker::View* m_view;
	CSpatialOs* m_spatialOs;
	worker::EntityId m_entityId;
	std::unique_ptr<ScopedViewCallbacks> m_callbacks;

	bool m_hasAuthority;
	bool m_isComponentReady;

	CCallbackList<size_t, bool> m_authorityChangeCallbacks;
	CCallbackList<size_t> m_componentReadyCallbacks;

	void OnAuthorityChangeDispatcherCallback(const worker::AuthorityChangeOp& op)
	{
		if (op.EntityId != m_entityId)
		{
			return;
		}
		m_hasAuthority = op.HasAuthority;
		m_authorityChangeCallbacks.Update(op.HasAuthority);
	}

	void OnComponentUpdateDispatcherCallback(const worker::ComponentUpdateOp<C>& op)
	{
		if (op.EntityId != m_entityId)
		{
			return;
		}
		auto update = op.Update;
		ApplyComponentUpdate(update);
	}

	// Workaround for removing boilerplate callback registration with component specific classes
	void RegisterDefaultCallbacks()
	{
		m_callbacks->Add(m_view->OnAuthorityChange<C>(std::bind(
			&ISpatialOsComponent::OnAuthorityChangeDispatcherCallback, this, std::placeholders::_1)));
		m_callbacks->Add(m_view->OnComponentUpdate<C>(std::bind(
			&ISpatialOsComponent::OnComponentUpdateDispatcherCallback, this, std::placeholders::_1)));
	}

};
