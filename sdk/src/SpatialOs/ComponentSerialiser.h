#pragma once

#include <improbable/worker.h>
#include <improbable/standard_library.h>

class CComponentSerialiser
{
public:
	virtual ~CComponentSerialiser() = default;

	virtual void WriteComponent(worker::Entity& entity, worker::Map<std::uint32_t, improbable::WorkerRequirementSet>& writeAcl) = 0;
};