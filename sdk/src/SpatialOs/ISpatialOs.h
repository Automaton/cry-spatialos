#pragma once
#include <improbable/worker.h>
#include <improbable/standard_library.h>
#include "SpatialOsEntitySpawner.h"
#include "Commander.h"
#include "improbable/vector3.h"
#include "automaton/quaternion.h"

template <typename ...Args>
struct SRegisterComponents
{
	static void Register(SpatialOsEntitySpawner& spawner)
	{
		spawner.RegisterComponents<Args...>();
	}
};

#define COMPONENT_NAME_LIST(NAME)  NAME::Component,
#define DEFAULT_COMPONENT_NAME_LIST(NAME) NAME,

#define SETUP_COMPONENTS(structName, componentList, defaultComponents)  \
	struct structName                                                   \
    {                                                                   \
		structName() : components(), ce_components() {}                 \
        worker::Components<componentList(COMPONENT_NAME_LIST) defaultComponents(DEFAULT_COMPONENT_NAME_LIST) improbable::Metadata, improbable::Persistence, improbable::Position> components;\
		SRegisterComponents<componentList(DEFAULT_COMPONENT_NAME_LIST) void> ce_components;																\
	};                                                                  \

class ISpatialOs
{

public:
	virtual Commander* SendWorkerCommand() = 0;
	virtual bool IsConnected() const = 0;
	virtual bool IsConnecting() const = 0;

	virtual CSpatialOsView * GetView() const = 0;
	virtual worker::Connection * GetConnection() const = 0;
	virtual SpatialOsEntitySpawner * GetEntitySpawner() const = 0;
	virtual Commander * GetCommander() const = 0;

	virtual std::string GetWorkerId() const = 0;
	virtual improbable::WorkerRequirementSet GetSnapshotRequirementSet() = 0;

	static automaton::Quaternion ISpatialOs::CreateQuat(const Quat& quat)
	{
		return { quat.w, quat.v.x, quat.v.y, quat.v.z };
	}

	static Quat ISpatialOs::CreateQuat(const automaton::Quaternion& quat)
	{
		return { quat.w(), quat.x(), quat.y(), quat.z() };
	}

	static Vec3 ISpatialOs::CreateVec3(const improbable::Vector3f& vec)
	{
		return Vec3(vec.x(), vec.y(), vec.z());
	}

	static improbable::Vector3f ISpatialOs::CreateVec3(const Vec3& vec)
	{
		return { vec.x, vec.y, vec.z };
	}
};
