#pragma once
#include <improbable/worker.h>
#include <improbable/standard_library.h>
#include "SpatialOsEntitySpawner.h"
#include "Commander.h"

const Quat g_spatialOsToCryRotator(Quat(Ang3(0.0f, -90.0f, -90.0f)));

class CSpatialOs
{
public:
	CSpatialOs();
	virtual ~CSpatialOs();

	Commander* SendWorkerCommand();
	void ProcessEvents() const;

	void ConnectToSpatialOs();
	void DisconnectFromSpatialOs();

	bool IsConnected() const;
	bool IsConnecting() const;

	worker::View * GetView() const { return m_view.get(); }
	worker::Connection * GetConnection() const { return m_connection.get(); }
	SpatialOsEntitySpawner * GetEntitySpawner() const { return m_entitySpawner.get(); }
	Commander * GetCommander() const { return m_workerCommander.get(); }

	worker::EntityId GetSpawnerId() const { return m_spawnerId; }
	std::string GetWorkerId() const { return m_workerId; }
	improbable::WorkerRequirementSet GetSnapshotRequirementSet();

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
