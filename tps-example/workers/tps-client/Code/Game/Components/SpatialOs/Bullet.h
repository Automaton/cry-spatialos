#pragma once
#include <CryEntitySystem/IEntitySystem.h>

#include <automaton/player/bullet.h>
#include "SpatialOs/component/SpatialOsComponent.h"
#include "SpatialOs/SpatialOsComponent.h"

class CSBullet : public ISpatialOsComponent<automaton::player::Bullet>
{
public:
	CSBullet();

	static void ReflectType(Schematyc::CTypeDesc<CSBullet>& desc)
	{
		desc.SetGUID("{B1A7459F-2B25-407E-8F86-5539E7155902}"_cry_guid);
	}

	void Initialise(worker::Connection& connection, CSpatialOsView& view, worker::EntityId entityId) override;
	void ProcessEvent(SEntityEvent& event) override;
	uint64 GetEventMask() const override;
	void ApplyComponentUpdate(const automaton::player::Bullet::Update& update) override;

	void WriteComponent(worker::Entity& entity, worker::Map<std::uint32_t, WorkerRequirementSet>& writeAcl) override;
private:
	void OnReady();
	void SetTransformReady(int readyState);
	void SetBulletReady();
	void InitPosition();

	EntityId m_id;
	worker::EntityId m_creator;
	Quat m_rotation;
	bool m_transformReady, m_bulletReady;
	CSpatialOsComponent *m_pSpatialOs;
};
