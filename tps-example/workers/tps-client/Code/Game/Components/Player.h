#pragma once

#include <CryEntitySystem/IEntityComponent.h>
#include <CryMath/Cry_Camera.h>

#include <ICryMannequin.h>

#include <DefaultComponents/Cameras/CameraComponent.h>
#include <DefaultComponents/Physics/CharacterControllerComponent.h>
#include <DefaultComponents/Geometry/AdvancedAnimationComponent.h>
#include <DefaultComponents/Input/InputComponent.h>

////////////////////////////////////////////////////////
// Represents a player participating in gameplay
////////////////////////////////////////////////////////
class CPlayerComponent final : public IEntityComponent
{
	enum class EInputFlagType
	{
		Hold = 0,
		Toggle
	};

	typedef uint8 TInputFlags;

	enum class EInputFlag
		: TInputFlags
	{
		MoveLeft = 1 << 0,
		MoveRight = 1 << 1,
		MoveForward = 1 << 2,
		MoveBack = 1 << 3,
		MoveSprint = 1 << 4
	};

public:
	CPlayerComponent() = default;
	virtual ~CPlayerComponent() {}

	// IEntityComponent
	virtual void Initialize() override;

	virtual uint64 GetEventMask() const override;
	virtual void ProcessEvent(SEntityEvent& event) override;

	static void ReflectType(Schematyc::CTypeDesc<CPlayerComponent>& desc)
	{
		desc.SetGUID("{D251C74C-A188-4D74-9861-38084D0FFA68}"_cry_guid);
	}
	// ~IEntityComponent
	void InitialiseLocalPlayer();

	void Revive();
	Quat GetLookOrientation() const;
	void SetLookOrientation(Quat quat);
	Cry::DefaultComponents::CCharacterControllerComponent* GetCharacterController() const { return m_pCharacterController; }
	bool IsSprinting() const { return m_spriting; }

protected:
	void UpdateMovementRequest(float frameTime);
	void UpdateAnimation(float frameTime);
	void UpdateCamera(float frameTime);

	void SpawnAtSpawnPoint();

	void CreateWeapon(const char *name);

	void HandleInputFlagChange(TInputFlags flags, int activationMode, EInputFlagType type = EInputFlagType::Hold);

protected:
	Cry::DefaultComponents::CCameraComponent* m_pCameraComponent = nullptr;
	Cry::DefaultComponents::CCharacterControllerComponent* m_pCharacterController = nullptr;
	Cry::DefaultComponents::CAdvancedAnimationComponent* m_pAnimationComponent = nullptr;
	Cry::DefaultComponents::CInputComponent* m_pInputComponent = nullptr;

	TagID m_rotateTagId;
	TagID m_walkTagId;

	TInputFlags m_inputFlags;

	Vec2 m_mouseDeltaRotation;
	_smart_ptr<IAction> m_pAimPoseAction;

	// Should translate to head orientation in the future
	Quat m_lookOrientation;
	bool m_spriting;
};
