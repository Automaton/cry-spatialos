#include "StdAfx.h"

#include <objbase.h>
#include <CryCore/ToolsHelpers/GuidUtil.h>
#include <automaton/spawner.h>
#include "SpatialOs.h"
#include "SpatialOs/ComponentSerialiser.h"
#include "Generated/components/automaton/Spawner.h"
#include "Generated/components/automaton/Tree.h"

CSpatialOs::CSpatialOs()
	: m_components()
	  , m_view(new CSpatialOsView(m_components.components))
	  , m_isConnecting(false)
	  , m_isConnected(false)
	  , m_connectionLock()
	  , m_entityQueryCallback(0)
	  , m_spawnerId(0)
{
	REGISTER_COMMAND("save_snapshot", CmdCreateSnapshot, 0, "Dumps current persistent SpatialOS entities to a Snapshot");
}

CSpatialOs::~CSpatialOs()
{
	if (gEnv->pConsole)
	{
		gEnv->pConsole->UnregisterVariable("save_snapshot", true);
	}
	if (IsConnected())
	{
		DisconnectFromSpatialOs();
	}
}

void CSpatialOs::CmdCreateSnapshot(IConsoleCmdArgs* args)
{
	const char *pPath = "default.snapshot";
	if (args->GetArgCount() > 1)
	{
		pPath = args->GetArg(1);
	}

	CreateSnapshot(pPath);
}

void CSpatialOs::CreateSnapshot(const char *pPath)
{
	std::string path = pPath;
	IEntityIt* pIterator = gEnv->pEntitySystem->GetEntityIterator();
	pIterator->MoveFirst();
	std::unordered_map<worker::EntityId, worker::Entity> snapshotEntities;

	const WorkerAttributeSet gameWorkerAttributeSet{ worker::List<std::string>{"GameWorker"} };
	const WorkerAttributeSet clientAttributeSet{ worker::List<std::string>{"TPSClient"} };

	const WorkerRequirementSet gameWorkerRequirementSet{ { gameWorkerAttributeSet } };
	const WorkerRequirementSet clientWorkerRequirementSet{ { clientAttributeSet } };
	const WorkerRequirementSet anyWorkerReadPermission{ { clientAttributeSet, gameWorkerAttributeSet } };
	worker::SnapshotOutputStream snapOut(CGamePlugin::GetSpatialOs().m_components.components, path);
	int id = 1;
	// Add spawner entity
	{
		const Coordinates initialPosition{ 0, 10, 0 };
		CryComment("Adding spawner entity to snapshot");
		worker::Entity snapshotEntity;
		snapshotEntity.Add<Metadata>(Metadata::Data{ "Spawner" });
		snapshotEntity.Add<Position>(Position::Data{ initialPosition });
		snapshotEntity.Add<Persistence>(Persistence::Data());
		snapshotEntity.Add<automaton::Spawner>(automaton::Spawner::Data{});

		worker::Map<std::uint32_t, WorkerRequirementSet> writeAcl;
		writeAcl.emplace(Metadata::ComponentId, gameWorkerRequirementSet);
		writeAcl.emplace(Position::ComponentId, gameWorkerRequirementSet);
		writeAcl.emplace(automaton::Spawner::ComponentId, gameWorkerRequirementSet);

		snapshotEntity.Add<EntityAcl>(EntityAcl::Data(anyWorkerReadPermission, writeAcl));
		auto option = snapOut.WriteEntity(id++, snapshotEntity);
		if (!option.empty())
		{
			CryLog("Save snapshot error: %s", option->c_str());
		}
	}
	while (!pIterator->IsEnd())
	{
		IEntity *pEntity = pIterator->Next();
		if (stricmp(pEntity->GetName(), "Schematyc Preview Entity") == 0) continue;
		if (CSpatialOsComponent *pComponent = pEntity->GetComponent<CSpatialOsComponent>())
		{
			if (pComponent->IsPersistent())
			{
				CryComment("Persistent entity being added to SpatialOS id: %d, name: %s", pEntity->GetId(), pEntity->GetName());
				worker::Entity entity;
				worker::Map<std::uint32_t, WorkerRequirementSet> writeAcl;
				
				entity.Add<Metadata>(Metadata::Data(pComponent->GetMetadata()));
				entity.Add<Position>(Position::Data(pComponent->GetSpatialOsCoords()));
				entity.Add<Persistence>(Persistence::Data());

				writeAcl.emplace(Metadata::ComponentId, gameWorkerRequirementSet);
				writeAcl.emplace(Position::ComponentId, gameWorkerRequirementSet);

				// Look through all components for SpatialOS
				DynArray<IEntityComponent*> components;
				pEntity->GetComponents(components);
				for (auto* pComp : components)
				{
					CEntityComponentClassDesc const & desc = pComp->GetClassDesc();
					const char *szEditorCategory = desc.GetEditorCategory();
					if (szEditorCategory != nullptr && stricmp(szEditorCategory, "SpatialOS") == 0)
					{
						CryComment("Component added to entity. Name: %s", desc.GetLabel());
						CComponentSerialiser *pSerialiser = reinterpret_cast<CComponentSerialiser *>(pComp);
						pSerialiser->WriteComponent(entity, writeAcl);
					}
				}
				entity.Add<EntityAcl>(EntityAcl::Data(anyWorkerReadPermission, writeAcl));
				auto option = snapOut.WriteEntity(id++, entity);
				if (!option.empty())
				{
					CryLog("Save snapshot error: %s", option->c_str());
				}
			}
		}
	}
}

void CSpatialOs::ConnectToSpatialOs()
{
	const std::string defaultReceptionistIp{ "127.0.0.1" };
	const int32 defaultReceptionistPort{ 7777 };
	const std::string defaultWorkerType{ "TPSClient" };
	const std::string defaultLinkProtocol{ "RakNet" };
	const std::string noDefault{ "" };
	GUID gidReference;
	HRESULT hCreateGuid = CoCreateGuid(&gidReference);

	worker::ConnectionParameters params;
	params.Network.ConnectionType = worker::NetworkConnectionType::kRaknet;
	params.Network.UseExternalIp = false;
	params.WorkerType = defaultWorkerType;
	std::string guid = GuidUtil::ToString(gidReference);
	guid.erase(std::remove(guid.begin(), guid.end(), '{'), guid.end());
	guid.erase(std::remove(guid.begin(), guid.end(), '}'), guid.end());
	m_workerId = defaultWorkerType + guid;

	ConnectToReceptionist(defaultReceptionistIp, defaultReceptionistPort, params, 10000);
}

improbable::WorkerRequirementSet CSpatialOs::GetSnapshotRequirementSet()
{
	const improbable::WorkerAttributeSet gameWorkerAttributeSet{ worker::List<std::string>{"GameWorker"} };
	const improbable::WorkerRequirementSet gameWorkerRequirementSet{ { gameWorkerAttributeSet } };
	return gameWorkerRequirementSet;
}

void CSpatialOs::RemoveSpatialOsEntities()
{
	auto *pEntityIterator = gEnv->pEntitySystem->GetEntityIterator();
	pEntityIterator->MoveFirst();

	std::vector<EntityId> spatialOsEntities;
	while (!pEntityIterator->IsEnd())
	{
		IEntity *pEntity = pEntityIterator->Next();
		if (pEntity->GetComponent<CSpatialOsComponent>())
		{
			spatialOsEntities.push_back(pEntity->GetId());
		}
	}

	for (EntityId id : spatialOsEntities)
	{
		gEnv->pEntitySystem->RemoveEntity(id);
	}
}

void CSpatialOs::ConnectToReceptionist(const std::string& hostName, uint16 port, worker::ConnectionParameters& params, uint32 timeoutMillis)
{
	m_isConnecting = true;

	auto connectionFuture = worker::Connection::ConnectAsync(m_components.components, hostName, port, m_workerId, params);

	if (connectionFuture.Wait(timeoutMillis))
	{
		m_connectionLock.Lock();
		m_connection.reset(new worker::Connection(connectionFuture.Get()));
		m_connectionLock.Unlock();
		m_isConnected = true;
		CryLog("Successfully connected to SpatialOS");

		OnSpatialOsConnected();
	}
	else
	{
		m_isConnected = false;
		CryLog("Failed to connect to SpatialOS");
	}

	m_isConnecting = false;

}

void CSpatialOs::RegisterComponents()
{
	m_components.ce_components.Register(*m_entitySpawner.get());
}

void CSpatialOs::DisconnectFromSpatialOs()
{
	m_connectionLock.Lock();
	CryLog("Disconnecting from SpatialOs");
	if (m_connection)
	{
		if (m_entityQueryCallback)
		{
			m_view->Remove(m_entityQueryCallback);
			m_entityQueryCallback = 0;
		}
		ProcessEvents();
		m_connection.reset();
	}
	m_isConnected = false;
	m_connectionLock.Unlock();
}

void CSpatialOs::OnSpatialOsConnected()
{
	if (m_connection)
	{
		m_entitySpawner.reset(new SpatialOsEntitySpawner(*(m_connection.get()), *(m_view.get()), *this));
		RegisterComponents();
	}
	const worker::query::EntityQuery& entityQuery = {
		worker::query::ComponentConstraint{ automaton::Spawner::ComponentId },
		worker::query::SnapshotResultType{}
	};

	auto requestId = m_connection->SendEntityQueryRequest(entityQuery, 1000);

	m_entityQueryCallback = m_view->OnEntityQueryResponse([this, requestId](const worker::EntityQueryResponseOp& op)
	{
		CryLog("Entity Query Response");
		if (op.RequestId != requestId) return;
		if (op.StatusCode != worker::StatusCode::kSuccess)
		{
			CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, "Failed to query for spawner entity.");
			return;
		}
		if (op.ResultCount == 0)
		{
			CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "Found 0 spawner entities.");
			return;
		}
		OnSpawnerFound(op.Result.begin()->first);
	});
	ProcessEvents();
}


void CSpatialOs::OnSpawnerFound(const worker::EntityId& entityId)
{
	//if (!gEnv->IsEditor())
	{
		CryLog("Spawner found, sending command request");
		automaton::SpawnPlayerRequest request(improbable::Coordinates(0, 0, 0));
		m_connection->SendCommandRequest<automaton::Spawner::Commands::SpawnPlayer>(entityId, request, 1000);
	}
	m_spawnerId = entityId;
	if (m_entityQueryCallback)
	{
		m_view->Remove(m_entityQueryCallback);
		m_entityQueryCallback = 0;
	}
}


void CSpatialOs::ProcessEvents() const
{
	if (m_connection)
	{
		auto ops = m_connection->GetOpList(0);
		m_view->Process(ops);
	}
}

bool CSpatialOs::IsConnected() const
{
	return m_isConnected;
}

bool CSpatialOs::IsConnecting() const
{
	return m_isConnecting;
}

Commander* CSpatialOs::SendWorkerCommand()
{
	if (!m_connection)
	{
		return nullptr;
	}

	if (m_workerCommander == nullptr) {
		m_workerCommander.reset(new Commander(*m_connection, *m_view));
	}
	return m_workerCommander.get();
}
