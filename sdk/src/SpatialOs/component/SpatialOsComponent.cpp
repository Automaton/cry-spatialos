#include "StdAfx.h"

#include "SpatialOsComponent.h"
#include <improbable/standard_library.h>
#include "SpatialOs/SpatialOs.h"

namespace {
	static void RegisterSpatialOsComponent(Schematyc::IEnvRegistrar& registrar)
	{
		Schematyc::CEnvRegistrationScope scope = registrar.Scope(IEntity::GetEntityScopeGUID());
		{
			Schematyc::CEnvRegistrationScope componentScope = scope.Register(SCHEMATYC_MAKE_ENV_COMPONENT(CSpatialOsComponent));
			{
				auto pFunction = SCHEMATYC_MAKE_ENV_FUNCTION(&CSpatialOsComponent::UpdatePosition, "{1093AEE3-577D-430A-81FC-C3C666159666}"_cry_guid, "UpdatePosition");
				pFunction->SetDescription("Update the SpatialOS position of this entity");
				pFunction->BindInput(1, 'pos', "Position");
				componentScope.Register(pFunction);
			}
			componentScope.Register(SCHEMATYC_MAKE_ENV_SIGNAL(SPositionUpdatedSignal));
		}
	}

	CRY_STATIC_AUTO_REGISTER_FUNCTION(&RegisterSpatialOsComponent);
}


CSpatialOsComponent::CSpatialOsComponent():
	m_spatialOsEntityId(0),
	m_view(nullptr),
	m_connection(nullptr),
	m_spatialOs(nullptr),
	m_persistent(false),
	m_positionAuthority(false),
	m_readyState(eCRS_None)
{
}

void CSpatialOsComponent::Init(worker::EntityId entityId, worker::View& view, worker::Connection& connection, ISpatialOs& spatialOs)
{
	m_spatialOsEntityId = entityId;
	m_view = &view;
	m_connection = &connection;
	m_spatialOs = &spatialOs;

	m_callbacks.reset(new ScopedViewCallbacks(view));
	auto it = view.Entities.find(entityId);
	if (it != view.Entities.end())
	{
		m_positionAuthority = it->second.HasAuthority<Position>();
	}

	m_callbacks->Add(m_view->OnAddComponent<Position>(std::bind(&CSpatialOsComponent::OnAddPosition, this, std::placeholders::_1)));
	m_callbacks->Add(m_view->OnAddComponent<Metadata>(std::bind(&CSpatialOsComponent::OnAddMetadata, this, std::placeholders::_1)));
	m_callbacks->Add(m_view->OnAddComponent<Persistence>(std::bind(&CSpatialOsComponent::OnAddPersistence, this, std::placeholders::_1)));
	m_callbacks->Add(m_view->OnAuthorityChange<Position>(std::bind(&CSpatialOsComponent::OnPositionAuthorityChange, this, std::placeholders::_1)));
	m_callbacks->Add(m_view->OnComponentUpdate<Position>(std::bind(&CSpatialOsComponent::OnUpdatePosition, this, std::placeholders::_1)));

	m_readyState |= eCRS_Initialised;
	m_readyCallbacks.Update(m_readyState);
}

void CSpatialOsComponent::UpdatePosition(Vec3 position) const
{
	Position::Update posUpdate;
	Vec3 pos = g_spatialOsToCryRotator.GetInverted() * position;
	posUpdate.set_coords(Coordinates(pos.x, pos.y, pos.z));
	m_connection->SendComponentUpdate<Position>(m_spatialOsEntityId, posUpdate);
}

Coordinates CSpatialOsComponent::GetSpatialOsCoords() const
{
	Vec3 pos = g_spatialOsToCryRotator.GetInverted() * GetEntity()->GetWorldPos();
	return Coordinates(pos.x, pos.y, pos.z);
}

void CSpatialOsComponent::ReflectType(Schematyc::CTypeDesc<CSpatialOsComponent>& desc)
{
	desc.SetGUID("{DC93D0AF-5AE1-491C-B109-E0409C47040E}"_cry_guid);
	desc.SetEditorCategory("Base");
	desc.SetLabel("SpatialOS");
	desc.AddMember(&CSpatialOsComponent::m_position, 'pos', "Position", "Position", "The SpatialOS position of the entity", Vec3(ZERO));
	desc.AddMember(&CSpatialOsComponent::m_entityInfo, 'einf', "EntityInfo", "Entity Info", "The metadata associated with this entity", "");
	desc.AddMember(&CSpatialOsComponent::m_readyState, 'rdy', "ReadyState", "Ready State", "The ready state of this component, metadata and position", 0);
	desc.AddMember(&CSpatialOsComponent::m_persistent, 'pers', "Persistent", "Persistent", "Whether this entity should be written to a snapshot", false);
}

void CSpatialOsComponent::OnAddMetadata(worker::AddComponentOp<Metadata> const& op)
{
	if (op.EntityId != m_spatialOsEntityId) return;
	const Metadata::Data& data = op.Data;
	std::string name = data.entity_type();
	m_entityInfo = name.c_str();

	string newName;
	newName.Format("%s_%d", name, op.EntityId);
	CryComment("Renaming entity %d to %s", GetEntityId(), newName.c_str());
	GetEntity()->SetName(newName);

	m_metadataCallbacks.Update(m_entityInfo);

	m_readyState |= eCRS_MetadataReady;
	m_readyCallbacks.Update(m_readyState);
}

void CSpatialOsComponent::OnAddPosition(worker::AddComponentOp<Position> const& op)
{
	if (op.EntityId != m_spatialOsEntityId) return;
	Position::Update update = Position::Update::FromInitialData(op.Data);
	m_readyState |= eCRS_PositionReady;
	OnPositionUpdate(update);
	m_readyCallbacks.Update(m_readyState);
}

void CSpatialOsComponent::OnAddPersistence(worker::AddComponentOp<Persistence> const& op)
{
	if (op.EntityId != m_spatialOsEntityId) return;
	m_persistent = true;
}

void CSpatialOsComponent::OnPositionAuthorityChange(worker::AuthorityChangeOp const& op)
{
	if (op.EntityId != m_spatialOsEntityId) return;
	m_positionAuthority = op.HasAuthority;
}

void CSpatialOsComponent::OnUpdatePosition(worker::ComponentUpdateOp<Position> const& op)
{
	if (op.EntityId != m_spatialOsEntityId) return;
	OnPositionUpdate(op.Update);
}

void CSpatialOsComponent::OnPositionUpdate(Position::Update const & update)
{
	if (!update.coords().empty())
	{
		auto pos = update.coords();
		Vec3 oldPos = m_position;
		m_position = g_spatialOsToCryRotator * Vec3(static_cast<float>(pos->x()), static_cast<float>(pos->y()), static_cast<float>(pos->z()));
		m_positionCallbacks.Update(m_position);
		if (Schematyc::IObject * pObject = GetEntity()->GetSchematycObject())
		{
			pObject->ProcessSignal(SPositionUpdatedSignal(oldPos, m_position), GetGUID());
		}
	}
}