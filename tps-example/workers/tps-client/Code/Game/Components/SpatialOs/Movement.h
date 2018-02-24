#pragma once
#include <automaton/player/movement.h>
#include <improbable/standard_library.h>

#include "SpatialOs/SpatialOsComponent.h"
#include "Components/Player.h"

class CSMovement : public ISpatialOsComponent<automaton::player::Movement>
{
public:
	CSMovement();
	virtual ~CSMovement();

	void ApplyComponentUpdate(const automaton::player::Movement::Update& update) override;
	uint64 GetEventMask() const override;
	void Initialise(worker::Connection& connection, CSpatialOsView& view, worker::EntityId entityId) override;

	void WriteComponent(worker::Entity& entity, worker::Map<std::uint32_t, improbable::WorkerRequirementSet>& writeAcl) override;
protected:
	void ProcessEvent(SEntityEvent& event) override;

	bool m_moving;
	bool m_running;
	Quat m_lookOrientation;
	Vec3 m_spatialOsPosition;

	CCallbackList<size_t, Quat> m_orientationCallbacks;
	CCallbackList<size_t, bool> m_movingCallbacks;

	const float PositionTolerance = 0.01f;
	const float OrientationWTolerance = 0.01f;
	const float OrientationVTolerance = 0.01f;
	const float MovingVelocityThreshold = 0.1f;
	const float TeleportThreshold = 3.0f;
};
