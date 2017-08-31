#pragma once
#include <CryEntitySystem/IEntityComponent.h>
#include <CrySchematyc/CoreAPI.h>

#include <improbable/worker.h>
#include <improbable/standard_library.h>

#include "SpatialOs/CallbackList.h"
#include "SpatialOs/ScopedViewCallbacks.h"

class CSpatialOs;
using namespace improbable;

enum EComponentReadyState
{
	eCRS_None = 0,

	eCRS_Initialised = BIT(0),
	eCRS_MetadataReady = BIT(1),
	eCRS_PositionReady = BIT(2),

	eCRS_All = eCRS_Initialised | eCRS_MetadataReady | eCRS_PositionReady
};

struct SPositionUpdatedSignal
{
	SPositionUpdatedSignal() {}
	SPositionUpdatedSignal(Vec3 oldPosition, Vec3 newPosition)
		: m_oldPosition(oldPosition)
		, m_newPosition(newPosition) {}

	Vec3 m_oldPosition;
	Vec3 m_newPosition;
};

static void ReflectType(Schematyc::CTypeDesc<SPositionUpdatedSignal>& desc)
{
	desc.SetGUID("{E86FC808-6B76-4E91-8A8B-D5AC49F63449}"_cry_guid);
	desc.SetLabel("OnPositionUpdated");
	desc.AddMember(&SPositionUpdatedSignal::m_oldPosition, 'opos', "OldPosition", "Old Position", "The old position of the entity", Vec3(ZERO));
	desc.AddMember(&SPositionUpdatedSignal::m_newPosition, 'npos', "NewPosition", "New Position", "The new position of the entity", Vec3(ZERO));
}

class CSpatialOsComponent : public IEntityComponent
{

public:
	CSpatialOsComponent();
	virtual ~CSpatialOsComponent() {}

	void Init(worker::EntityId id, worker::View& view, worker::Connection& connection, CSpatialOs& spatialOs);

	void UpdatePosition(Vec3 position) const;
	Vec3 GetPosition() const { return m_position; }

	bool HasPositionAuthority() const { return m_positionAuthority; }
	bool IsReady() const { return (m_readyState == eCRS_All); }
	int GetReadyState() const { return m_readyState; }
	std::string GetMetadata() const { return m_entityInfo.empty() ? GetEntity()->GetClass()->GetName() : std::string(m_entityInfo.c_str());  }
	Coordinates GetSpatialOsCoords() const;
	bool IsPersistant() const { return m_persistent; }

	static void ReflectType(Schematyc::CTypeDesc<CSpatialOsComponent>& desc);

	void OnAddMetadata(worker::AddComponentOp<Metadata> const & op);
	void OnAddPosition(worker::AddComponentOp<Position> const & op);

private:
	void OnAddPersistence(worker::AddComponentOp<Persistence> const & op);
	void OnPositionAuthorityChange(worker::AuthorityChangeOp const & op);
	void OnUpdatePosition(worker::ComponentUpdateOp<Position> const & op);

	void OnPositionUpdate(Position::Update const & update);

	// SpatialOS fields
	worker::EntityId m_spatialOsEntityId;
	worker::View *m_view;
	worker::Connection *m_connection;
	CSpatialOs *m_spatialOs;
	std::unique_ptr<ScopedViewCallbacks> m_callbacks;

	// Fields from components
	Vec3 m_position;
	Schematyc::CSharedString m_entityInfo;
	bool m_persistent;

	// Metadata from components
	bool m_positionAuthority;
	int m_readyState;

	// Callback lists
	DECLARE_CALLBACK_LIST(m_readyCallbacks, ReadyState, int)
	DECLARE_CALLBACK_LIST(m_positionCallbacks, Position, Vec3)
	DECLARE_CALLBACK_LIST(m_metadataCallbacks, Metadata, Schematyc::CSharedString)

//	CCallbackList<size_t, int> m_readyCallbacks;
//	CCallbackList<size_t, Vec3> m_positionCallbacks;
//	CCallbackList<size_t, string> m_metadataCallbacks;
};
