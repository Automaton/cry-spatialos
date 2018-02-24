#pragma once
#include "SpatialOs/SpatialOsComponent.h"
#include <CrySchematyc/CoreAPI.h>
#include <automaton/player/score.h>

struct SKillsUpdatedSignal
{
	SKillsUpdatedSignal() {}
	SKillsUpdatedSignal(uint32_t oldKills, uint32_t newKills)
		: m_oldKills(oldKills)
		, m_newKills(newKills) {}

	uint32_t m_oldKills;
	uint32_t m_newKills;
};

static void ReflectType(Schematyc::CTypeDesc<SKillsUpdatedSignal>& desc)
{
	desc.SetGUID("{EA0F357E-EA0E-4101-82DA-7BD61A3CE72A}"_cry_guid);
	desc.AddMember(&SKillsUpdatedSignal::m_oldKills, 'ok', "OldKills", "Old Kills", "", 0);
	desc.AddMember(&SKillsUpdatedSignal::m_newKills, 'nk', "NewKills", "New Kills", "", 0);
}

struct SDeathsUpdatedSignal
{
	SDeathsUpdatedSignal() {}
	SDeathsUpdatedSignal(uint32_t oldDeaths, uint32_t newDeaths)
		: m_oldDeaths(oldDeaths)
		, m_newDeaths(newDeaths) {}

	uint32_t m_oldDeaths;
	uint32_t m_newDeaths;
};

static void ReflectType(Schematyc::CTypeDesc<SDeathsUpdatedSignal>& desc)
{
	desc.SetGUID("{FDE17FE0-7293-4EE5-8B51-C9E3DDEC873A}"_cry_guid);
	desc.AddMember(&SDeathsUpdatedSignal::m_oldDeaths, 'od', "OldDeaths", "Old Deaths", "", 0);
	desc.AddMember(&SDeathsUpdatedSignal::m_newDeaths, 'nd', "NewDeaths", "New Deaths", "", 0);
}

class CSPlayerScore : public ISpatialOsComponent<automaton::player::Score>
{
public:
	CSPlayerScore();

	static void ReflectType(Schematyc::CTypeDesc<CSPlayerScore>& desc);

	uint64 GetEventMask() const override;
	void Initialise(worker::Connection& connection, CSpatialOsView& view, worker::EntityId entityId) override;
	void ApplyComponentUpdate(const automaton::player::Score::Update& update) override;

	void WriteComponent(worker::Entity& entity, worker::Map<std::uint32_t, WorkerRequirementSet>& writeAcl) override;
	void UpdateKills(uint32_t kills) const;
	void UpdateDeaths(uint32_t deaths) const;
protected:
	void ProcessEvent(SEntityEvent& event) override;

private:
	uint32_t m_kills;
	uint32_t m_deaths;

	DECLARE_CALLBACK_LIST(m_killCallbackList, Kills, uint32_t);
	DECLARE_CALLBACK_LIST(m_deathCallbackList, Deaths, uint32_t);

};
