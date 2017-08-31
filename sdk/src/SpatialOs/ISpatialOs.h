#pragma once
#include <improbable/worker.h>
#include <improbable/standard_library.h>
#include "SpatialOsEntitySpawner.h"
#include "Commander.h"

const Quat g_spatialOsToCryRotator(Quat(Ang3(0.0f, -90.0f, -90.0f)));

class ISpatialOs
{

public:
	virtual Commander* SendWorkerCommand() = 0;
	virtual bool IsConnected() const = 0;
	virtual bool IsConnecting() const = 0;

	virtual worker::View * GetView() const = 0;
	virtual worker::Connection * GetConnection() const = 0;
	virtual SpatialOsEntitySpawner * GetEntitySpawner() const = 0;
	virtual Commander * GetCommander() const = 0;

	virtual worker::EntityId GetSpawnerId() const = 0;
	virtual std::string GetWorkerId() const = 0;
	virtual improbable::WorkerRequirementSet GetSnapshotRequirementSet() = 0;
};
