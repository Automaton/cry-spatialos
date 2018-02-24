#include "StdAfx.h"

#include "PlayerScore.h"
#include <CryRenderer/IRenderAuxGeom.h>
#include "Player.h"

namespace {
	static void RegisterCSPlayerScore(Schematyc::IEnvRegistrar& registrar)
	{
		Schematyc::CEnvRegistrationScope scope = registrar.Scope(IEntity::GetEntityScopeGUID());
		{
			Schematyc::CEnvRegistrationScope componentScope = scope.Register(SCHEMATYC_MAKE_ENV_COMPONENT(CSPlayerScore));
			{
				auto pFunction = SCHEMATYC_MAKE_ENV_FUNCTION(&CSPlayerScore::UpdateKills, "{E30FAF42-A721-4604-8006-5BE781972AFF}"_cry_guid, "UpdateKills");
				pFunction->BindInput(1, 'kill', "Kills");
				componentScope.Register(pFunction);
			}
			{
				auto pFunction = SCHEMATYC_MAKE_ENV_FUNCTION(&CSPlayerScore::UpdateDeaths, "{0418DB84-8AE3-44F2-9858-85D5FF4E857A}"_cry_guid, "UpdateDeaths");
				pFunction->BindInput(1, 'deat', "Deaths");
				componentScope.Register(pFunction);
			}
			componentScope.Register(SCHEMATYC_MAKE_ENV_SIGNAL(SDeathsUpdatedSignal));
			componentScope.Register(SCHEMATYC_MAKE_ENV_SIGNAL(SKillsUpdatedSignal));
		}
	}

	CRY_STATIC_AUTO_REGISTER_FUNCTION(&RegisterCSPlayerScore);
}

void CSPlayerScore::ProcessEvent(SEntityEvent& event)
{
	CSPlayer *pPlayer = GetEntity()->GetComponent<CSPlayer>();
	if (pPlayer == nullptr || !pPlayer->HasAuthority()) return;
	gEnv->pAuxGeomRenderer->Draw2dLabel(25, 10, 3.0f, ColorF(1.f, 1.f, 1.f, 1.f), false, "Kills: %d", m_kills);
	gEnv->pAuxGeomRenderer->Draw2dLabel(25, 30, 3.0f, ColorF(1.f, 0.f, 0.f, 1.f), false, "Deaths: %d", m_deaths);
}

CSPlayerScore::CSPlayerScore(): m_kills(0),
                                m_deaths(0)
{
}

void CSPlayerScore::ReflectType(Schematyc::CTypeDesc<CSPlayerScore>& desc)
{
	desc.SetGUID("{183EFD45-A3AA-4D23-9C4E-4C209B68B459}"_cry_guid);
	desc.SetLabel("PlayerScore");
	desc.SetEditorCategory("SpatialOS");
	desc.AddMember(&CSPlayerScore::m_deaths, 'deat', "Deaths", "Deaths", "Number of deaths", 0);
	desc.AddMember(&CSPlayerScore::m_kills, 'kill', "Kills", "Kills", "Number of kills", 0);
}

uint64 CSPlayerScore::GetEventMask() const
{
	return BIT64(ENTITY_EVENT_UPDATE);
}

void CSPlayerScore::Initialise(worker::Connection& connection, CSpatialOsView& view, worker::EntityId entityId)
{
	RegisterDefaultCallbacks();
}

void CSPlayerScore::ApplyComponentUpdate(const automaton::player::Score::Update& update)
{
	if (!update.deaths().empty())
	{
		uint32_t oldDeaths = m_deaths;
		m_deaths = *update.deaths();
		m_deathCallbackList.Update(m_deaths);
		if (Schematyc::IObject *pObject = GetEntity()->GetSchematycObject())
		{
			pObject->ProcessSignal(SDeathsUpdatedSignal(oldDeaths, m_deaths), GetGUID());
		}
		if ((m_deaths - oldDeaths) > 0)
		{
			if (CSPlayer *pPlayer = GetEntity()->GetComponent<CSPlayer>())
			{
				pPlayer->Respawn();
			}
		}
	}
	if (!update.kills().empty())
	{
		uint32_t oldKills = m_kills;
		m_kills = *update.kills();
		m_killCallbackList.Update(m_kills);
		if (Schematyc::IObject *pObject = GetEntity()->GetSchematycObject())
		{
			pObject->ProcessSignal(SKillsUpdatedSignal(oldKills, m_kills), GetGUID());
		}
	}
}

void CSPlayerScore::UpdateKills(uint32_t kills) const
{
	Component::Update update;
	update.set_kills(kills);
	m_connection->SendComponentUpdate<Component>(GetSpatialOsEntityId(), update);
}

void CSPlayerScore::UpdateDeaths(uint32_t deaths) const
{
	Component::Update update;
	update.set_deaths(deaths);
	m_connection->SendComponentUpdate<Component>(GetSpatialOsEntityId(), update);
}

void CSPlayerScore::WriteComponent(worker::Entity& entity, worker::Map<std::uint32_t, improbable::WorkerRequirementSet>& writeAcl)
{
	entity.Add<Score>(Score::Data(m_kills, m_deaths));
	const improbable::WorkerAttributeSet gameWorkerAttributeSet{ worker::List<std::string>{"GameWorker"} };
	const improbable::WorkerRequirementSet gameWorkerRequirementSet{ { gameWorkerAttributeSet } };
	writeAcl.emplace(Component::ComponentId, gameWorkerRequirementSet);
}