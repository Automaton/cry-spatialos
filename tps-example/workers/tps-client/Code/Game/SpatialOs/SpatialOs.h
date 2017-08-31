#pragma once
#include <improbable/worker.h>
#include <improbable/standard_library.h>
#include "SpatialOs/SpatialOsEntitySpawner.h"
#include "SpatialOs/Commander.h"
#include "SpatialOs/ISpatialOs.h"

class CSpatialOs : public ISpatialOs
{
public:
	CSpatialOs();
	virtual ~CSpatialOs();

	Commander* SendWorkerCommand() override;
	void ProcessEvents() const;

	void ConnectToSpatialOs();
	void DisconnectFromSpatialOs();

	bool IsConnected() const override;
	bool IsConnecting() const override;

	worker::View * GetView() const override { return m_view.get(); }
	worker::Connection * GetConnection() const override { return m_connection.get(); }
	SpatialOsEntitySpawner * GetEntitySpawner() const override { return m_entitySpawner.get(); }
	Commander * GetCommander() const override { return m_workerCommander.get(); }

	worker::EntityId GetSpawnerId() const override { return m_spawnerId; }
	std::string GetWorkerId() const override { return m_workerId; }
	improbable::WorkerRequirementSet GetSnapshotRequirementSet() override;

	static void RemoveSpatialOsEntities();
	static void CmdCreateSnapshot(IConsoleCmdArgs* args);
	static void CreateSnapshot(const char *pPath);

private:
	void ConnectToReceptionist(const std::string& hostName, uint16 port, worker::ConnectionParameters& params, uint32 timeoutMillis);

	void OnSpatialOsConnected();
	void OnSpawnerFound(const worker::EntityId& entityId);

	std::unique_ptr<worker::View> m_view;
	std::unique_ptr<worker::Connection> m_connection;
	std::unique_ptr<SpatialOsEntitySpawner> m_entitySpawner;
	std::unique_ptr<Commander> m_workerCommander;

	bool m_isConnecting;
	bool m_isConnected;
	CryCriticalSection m_connectionLock;
	worker::detail::CallbackKey m_entityQueryCallback;
	worker::EntityId m_spawnerId;
	std::string m_workerId;
};
