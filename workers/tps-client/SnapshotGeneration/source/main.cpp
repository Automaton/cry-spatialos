#include <iostream>

#include <improbable/worker.h>
#include <improbable/collections.h>
#include <improbable/standard_library.h>
#include <automaton/spawner.h>

using namespace improbable;

static WorkerAttributeSet gameWorkerAttributeSet{ worker::List<std::string>{"GameWorker"} };
static WorkerAttributeSet clientAttributeSet{ worker::List<std::string>{"TPSClient"} };

static WorkerRequirementSet gameWorkerRequirementSet { { gameWorkerAttributeSet } };
static WorkerRequirementSet clientWorkerRequirementSet { { clientAttributeSet } };
static WorkerRequirementSet anyWorkerReadPermission{ { clientAttributeSet, gameWorkerAttributeSet } };

worker::Entity CreateSpawnerSnapshotEntity()
{
	const Coordinates initialPosition{ 0, 10, 0 };

	worker::Entity snapshotEntity;
	snapshotEntity.Add<Metadata>(Metadata::Data{"Spawner"});
	snapshotEntity.Add<Position>(Position::Data{ initialPosition });
	snapshotEntity.Add<Persistence>(Persistence::Data());
	snapshotEntity.Add<automaton::Spawner>(automaton::Spawner::Data{});

	worker::Map<std::uint32_t, WorkerRequirementSet> writeAcl;
	writeAcl.emplace(Metadata::ComponentId, gameWorkerRequirementSet);
	writeAcl.emplace(Position::ComponentId, clientWorkerRequirementSet);
	writeAcl.emplace(automaton::Spawner::ComponentId, gameWorkerRequirementSet);

	snapshotEntity.Add<EntityAcl>(EntityAcl::Data(anyWorkerReadPermission, writeAcl));
	return snapshotEntity;
}

void GenerateSnapshot()
{
	const std::string fullPath = "default.snapshot";

	std::unordered_map<worker::EntityId, worker::Entity> snapshotEntities;
	snapshotEntities.emplace(1, CreateSpawnerSnapshotEntity());
	//for (auto npcId = 1; npcId <= 5; npcId++)
	//{
	//	snapshotEntities.emplace(npcId, CreateNPCSnapshotEntity());
	//}
	worker::SaveSnapshot(fullPath, snapshotEntities);
}

int main()
{
	std::cout << "Snapshot generation for third person SpatialOS integration sample" << std::endl;
	GenerateSnapshot();

	std::cout << "Snapshot generated" << std::endl;
	system("Pause");
	return 0;
}