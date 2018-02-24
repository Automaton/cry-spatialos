#pragma once
#include <improbable/worker.h>
#include <improbable/standard_library.h>
#include <automaton/spawner.h>

#include "SpatialOs/SpatialOsEntitySpawner.h"
#include "SpatialOs/Commander.h"
#include "SpatialOs/ISpatialOs.h"

#include "Components/SpatialOs/Bullet.h"
#include "Components/SpatialOs/Player.h"
#include "Components/SpatialOs/PlayerScore.h"
#include "Components/SpatialOs/Movement.h"

#include "Generated/components/automaton/Tree.h"

#define COMPONENT_LIST(f) \
	f(CSPlayer) \
	f(CSMovement) \
	f(CSBullet) \
	f(CSPlayerScore)  \
	f(CSTree)

#define DEFAULT_COMPONENTS(f) \
	f(improbable::EntityAcl) \
	f(automaton::Spawner)

SETUP_COMPONENTS(SSpatialOsComponents, COMPONENT_LIST, DEFAULT_COMPONENTS)

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

	CSpatialOsView* GetView() const override { return m_view.get(); }
	worker::Connection * GetConnection() const override { return m_connection.get(); }
	SpatialOsEntitySpawner * GetEntitySpawner() const override { return m_entitySpawner.get(); }
	Commander * GetCommander() const override { return m_workerCommander.get(); }

	worker::EntityId GetSpawnerId() const { return m_spawnerId; }
	std::string GetWorkerId() const override { return m_workerId; }
	improbable::WorkerRequirementSet GetSnapshotRequirementSet() override;

	static void RemoveSpatialOsEntities();
	static void CmdCreateSnapshot(IConsoleCmdArgs* args);
	static void CreateSnapshot(const char *pPath);

private:
	void ConnectToReceptionist(const std::string& hostName, uint16 port, worker::ConnectionParameters& params, uint32 timeoutMillis);

	void RegisterComponents();
	void OnSpatialOsConnected();
	void OnSpawnerFound(const worker::EntityId& entityId);

	SSpatialOsComponents m_components;

	std::unique_ptr<CSpatialOsView> m_view;
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
