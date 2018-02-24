#pragma once
#include <improbable/worker.h>
#include "ScopedViewCallbacks.h"

class Commander {
public:
	Commander(worker::Connection& connection, CSpatialOsView& view)
		: m_connection(connection), m_view(view), m_callbacks(view)
	{
		m_callbacks.Add(m_view.OnReserveEntityIdResponse(std::bind(&Commander::OnReserveEntityIdResponseDispatcherCallback, this, std::placeholders::_1)));
		m_callbacks.Add(m_view.OnCreateEntityResponse(std::bind(&Commander::OnCreateEntityResponseDispatcherCallback, this, std::placeholders::_1)));
		m_callbacks.Add(m_view.OnDeleteEntityResponse(std::bind(&Commander::OnDeleteEntityResponseDispatcherCallback, this, std::placeholders::_1)));
	}

	void ReserveEntityId(std::function<void(bool, std::string, worker::EntityId)> callback)
	{
		auto requestId = m_connection.SendReserveEntityIdRequest(0);
		m_requestIdToCreateEntityCallback.emplace(requestId.Id, callback);
	}

	void CreateEntity(worker::Entity entityTemplate, worker::EntityId entityId, std::function<void(bool, std::string, worker::EntityId)> callback)
	{
		auto requestId = m_connection.SendCreateEntityRequest(entityTemplate, entityId, 0);
		m_requestIdToCreateEntityCallback.emplace(requestId.Id, callback);
	}

	void DeleteEntity(worker::EntityId entityId, std::function<void(bool, std::string)> callback)
	{
		auto requestId = m_connection.SendDeleteEntityRequest(entityId, 0);
		m_requestIdToDeleteEntityCallback.emplace(requestId.Id, callback);
	}

private:
	worker::Connection& m_connection;
	CSpatialOsView& m_view;
	ScopedViewCallbacks m_callbacks;

	std::map <std::uint32_t, std::function<void(bool, std::string, worker::EntityId)>> m_requestIdToReserveEntityIdCallback;
	std::map <std::uint32_t, std::function<void(bool, std::string, worker::EntityId)>> m_requestIdToCreateEntityCallback;
	std::map <std::uint32_t, std::function<void(bool, std::string)>> m_requestIdToDeleteEntityCallback;

	void OnReserveEntityIdResponseDispatcherCallback(const worker::ReserveEntityIdResponseOp& op)
	{
		auto iterator = m_requestIdToReserveEntityIdCallback.find(op.RequestId.Id);
		if (iterator == m_requestIdToReserveEntityIdCallback.end())
		{
			return;
		}
		auto callback = iterator->second;
		auto success = op.StatusCode == worker::StatusCode::kSuccess;
		auto entityId = static_cast<int>((op.EntityId.empty() ? -1 : *(op.EntityId.data())));
		callback(success, op.Message, entityId);
	}

	void OnCreateEntityResponseDispatcherCallback(const worker::CreateEntityResponseOp& op)
	{
		auto iterator = m_requestIdToCreateEntityCallback.find(op.RequestId.Id);
		if (iterator == m_requestIdToCreateEntityCallback.end())
		{
			return;
		}
		auto callback = iterator->second;
		auto success = op.StatusCode == worker::StatusCode::kSuccess;
		auto entityId = static_cast<int>((op.EntityId.empty() ? -1 : *(op.EntityId.data())));
		callback(success, op.Message, entityId);
	}

	void OnDeleteEntityResponseDispatcherCallback(const worker::DeleteEntityResponseOp& op)
	{
		auto iterator = m_requestIdToDeleteEntityCallback.find(op.RequestId.Id);
		if (iterator == m_requestIdToDeleteEntityCallback.end())
		{
			return;
		}
		auto callback = iterator->second;
		auto success = op.StatusCode == worker::StatusCode::kSuccess;
		callback(success, op.Message);
	}
};