#pragma once
#include <CryEntitySystem/IEntityComponent.h>
#include <CrySchematyc/CoreAPI.h>

#include <improbable/worker.h>
#include <improbable/standard_library.h>

#include "SpatialOs/CallbackList.h"
#include "SpatialOs/ScopedViewCallbacks.h"

class ISpatialOs;
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
	SPositionUpdatedSignal(CryTransform::CTransform oldT, CryTransform::CTransform newT)
		: m_oldTransform(oldT)
		, m_newTransform(newT) {}

	CryTransform::CTransform m_oldTransform;
	CryTransform::CTransform m_newTransform;
};

struct SOnPositionAuthoritySignal
{
	SOnPositionAuthoritySignal() : m_authority(false) {}
	SOnPositionAuthoritySignal(bool auth)  : m_authority(auth) {}

	bool m_authority;
};

static void ReflectType(Schematyc::CTypeDesc<SPositionUpdatedSignal>& desc)
{
	desc.SetGUID("{E86FC808-6B76-4E91-8A8B-D5AC49F63449}"_cry_guid);
	desc.SetLabel("OnTransformUpdated");
	desc.AddMember(&SPositionUpdatedSignal::m_oldTransform, 'otra', "OldTransform", "Old Transform", "The old transform of the entity", CryTransform::CTransform());
	desc.AddMember(&SPositionUpdatedSignal::m_newTransform, 'ntra', "NewTransform", "New Transform", "The new transform of the entity", CryTransform::CTransform());
}

static void ReflectType(Schematyc::CTypeDesc<SOnPositionAuthoritySignal>& desc)
{
	desc.SetGUID("{C8E21427-0770-4736-B6AA-D86F3A0128FF}"_cry_guid);
	desc.SetLabel("OnPositionAuthority");
	desc.AddMember(&SOnPositionAuthoritySignal::m_authority, 'aut', "Authority", "Authority", "Whether the worker has authority over this entitys position", false);
}

class CSpatialOsComponent : public IEntityComponent
{

public:
	CSpatialOsComponent();
	virtual ~CSpatialOsComponent() {}

	void Init(worker::EntityId id, worker::View& view, worker::Connection& connection, ISpatialOs& spatialOs);

	void UpdatePosition(Vec3 position) const;
	void FlushPosition() const;

	bool HasPositionAuthority() const { return m_positionAuthority; }
	bool IsReady() const { return (m_readyState == eCRS_All); }
	int GetReadyState() const { return m_readyState; }
	std::string GetMetadata() const { return m_entityInfo.empty() ? GetEntity()->GetClass()->GetName() : std::string(m_entityInfo.c_str());  }
	Coordinates GetSpatialOsCoords() const;
	bool IsPersistant() const { return m_persistent; }
	bool IsWritingPosition() const { return m_writePosition; }

	static void ReflectType(Schematyc::CTypeDesc<CSpatialOsComponent>& desc);

	void OnAddMetadata(worker::AddComponentOp<Metadata> const & op);
	void OnAddPosition(worker::AddComponentOp<Position> const & op);
	void SetWritePosition(bool writePos) { m_writePosition = writePos;  }

private:
	void OnAddPersistence(worker::AddComponentOp<Persistence> const & op);
	void OnPositionAuthorityChange(worker::AuthorityChangeOp const & op);
	void OnUpdatePosition(worker::ComponentUpdateOp<Position> const & op);

	void OnPositionUpdate(Position::Update const & update);

	// SpatialOS fields
	worker::EntityId m_spatialOsEntityId;
	worker::View *m_view;
	worker::Connection *m_connection;
	ISpatialOs *m_spatialOs;
	std::unique_ptr<ScopedViewCallbacks> m_callbacks;

	// Fields from components
	Schematyc::CSharedString m_entityInfo;
	bool m_persistent;

	// Metadata from components
	bool m_positionAuthority;
	int m_readyState;
	bool m_writePosition;

	// Callback lists
	DECLARE_CALLBACK_LIST(m_transformCallbacks, Transform, CryTransform::CTransform)
	DECLARE_CALLBACK_LIST(m_readyCallbacks, ReadyState, int)
	DECLARE_CALLBACK_LIST(m_metadataCallbacks, Metadata, Schematyc::CSharedString)

//	CCallbackList<size_t, int> m_readyCallbacks;
//	CCallbackList<size_t, Vec3> m_positionCallbacks;
//	CCallbackList<size_t, string> m_metadataCallbacks;
};
