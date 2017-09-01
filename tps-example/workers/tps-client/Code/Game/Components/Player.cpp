#include "StdAfx.h"
#include "Player.h"

#include "Bullet.h"
#include "SpawnPoint.h"

#include <CryRenderer/IRenderAuxGeom.h>
#include "SpatialOs/Player.h"
#include <CryMath/Random.h>

void CPlayerComponent::Initialize()
{
	
	// The character controller is responsible for maintaining player physics
	m_pCharacterController = m_pEntity->GetOrCreateComponent<Cry::DefaultComponents::CCharacterControllerComponent>();

	// Create the advanced animation component, responsible for updating Mannequin and animating the player
	m_pAnimationComponent = m_pEntity->GetOrCreateComponent<Cry::DefaultComponents::CAdvancedAnimationComponent>();
	
	// Set the player geometry, this also triggers physics proxy creation
	m_pAnimationComponent->SetMannequinAnimationDatabaseFile("Animations/Mannequin/ADB/ThirdPerson.adb");
	m_pAnimationComponent->SetCharacterFile("Objects/Characters/human/sdk_player/sdk_player.cdf");

	m_pAnimationComponent->SetControllerDefinitionFile("Animations/Mannequin/ADB/ThirdPersonControllerDefinition.xml");
	m_pAnimationComponent->SetDefaultScopeContextName("ThirdPersonCharacter");
	// Queue the idle fragment to start playing immediately on next update
	m_pAnimationComponent->SetDefaultFragmentName("Idle");
	
	// Disable movement coming from the animation (root joint offset), we control this entirely via physics
	m_pAnimationComponent->SetAnimationDrivenMotion(false);

//	m_pAnimationComponent->EnableGroundAlignment(true);
	// Load the character and Mannequin data from file
	m_pAnimationComponent->LoadFromDisk();

	// Acquire tag identifiers to avoid doing so each update
	m_rotateTagId = m_pAnimationComponent->GetTagId("Rotate");
	m_walkTagId = m_pAnimationComponent->GetTagId("Walk");

	Revive();
}


void CPlayerComponent::InitialiseLocalPlayer()
{
	// Create the camera component, will automatically update the viewport every frame
	m_pCameraComponent = m_pEntity->GetOrCreateComponent<Cry::DefaultComponents::CCameraComponent>();

	// Get the input component, wraps access to action mapping so we can easily get callbacks when inputs are triggered
	m_pInputComponent = m_pEntity->GetOrCreateComponent<Cry::DefaultComponents::CInputComponent>();

	// Register an action, and the callback that will be sent when it's triggered
	m_pInputComponent->RegisterAction("player", "moveleft", [this](int activationMode, float value) { HandleInputFlagChange((TInputFlags)EInputFlag::MoveLeft, activationMode);  });
	// Bind the 'A' key the "moveleft" action
	m_pInputComponent->BindAction("player", "moveleft", eAID_KeyboardMouse, EKeyId::eKI_A);

	m_pInputComponent->RegisterAction("player", "moveright", [this](int activationMode, float value) { HandleInputFlagChange((TInputFlags)EInputFlag::MoveRight, activationMode);  });
	m_pInputComponent->BindAction("player", "moveright", eAID_KeyboardMouse, EKeyId::eKI_D);

	m_pInputComponent->RegisterAction("player", "moveforward", [this](int activationMode, float value) { HandleInputFlagChange((TInputFlags)EInputFlag::MoveForward, activationMode);  });
	m_pInputComponent->BindAction("player", "moveforward", eAID_KeyboardMouse, EKeyId::eKI_W);

	m_pInputComponent->RegisterAction("player", "moveback", [this](int activationMode, float value) { HandleInputFlagChange((TInputFlags)EInputFlag::MoveBack, activationMode);  });
	m_pInputComponent->BindAction("player", "moveback", eAID_KeyboardMouse, EKeyId::eKI_S);

	m_pInputComponent->RegisterAction("player", "sprint", [this](int activationMode, float value) { HandleInputFlagChange((TInputFlags)EInputFlag::MoveSprint, activationMode);  });
	m_pInputComponent->BindAction("player", "sprint", eAID_KeyboardMouse, EKeyId::eKI_LShift);

	m_pInputComponent->RegisterAction("player", "mouse_rotateyaw", [this](int activationMode, float value) { m_mouseDeltaRotation.x -= value; });
	m_pInputComponent->BindAction("player", "mouse_rotateyaw", eAID_KeyboardMouse, EKeyId::eKI_MouseX);

	m_pInputComponent->RegisterAction("player", "mouse_rotatepitch", [this](int activationMode, float value) { m_mouseDeltaRotation.y -= value; });
	m_pInputComponent->BindAction("player", "mouse_rotatepitch", eAID_KeyboardMouse, EKeyId::eKI_MouseY);

	// Register the shoot action
	m_pInputComponent->RegisterAction("player", "shoot", [this](int activationMode, float value)
	{
		// Only fire on press, not release
		if (activationMode == eIS_Pressed)
		{
			if (ICharacterInstance *pCharacter = m_pAnimationComponent->GetCharacter())
			{
				auto *pBarrelOutAttachment = pCharacter->GetIAttachmentManager()->GetInterfaceByName("barrel_out");

				if (pBarrelOutAttachment != nullptr)
				{
					QuatTS bulletOrigin = pBarrelOutAttachment->GetAttWorldAbsolute();
					SEntitySpawnParams spawnParams;
					spawnParams.pClass = gEnv->pEntitySystem->GetClassRegistry()->GetDefaultClass();

					spawnParams.vPosition = bulletOrigin.t;
					spawnParams.qRotation = bulletOrigin.q;

					const float bulletScale = 0.05f;
					spawnParams.vScale = Vec3(bulletScale);

					// Spawn the entity
					if (IEntity* pEntity = gEnv->pEntitySystem->SpawnEntity(spawnParams))
					{
						// See Bullet.cpp, bullet is propelled in  the rotation and position the entity was spawned with
						pEntity->CreateComponentClass<CBulletComponent>();
						if (CSPlayer* pPlayer = GetEntity()->GetComponent<CSPlayer>())
						{
							pPlayer->FireBullet(bulletOrigin, pEntity->GetId());
						}
					}

				}
			}
		}
	});

	// Bind the shoot action to left mouse click
	m_pInputComponent->BindAction("player", "shoot", eAID_KeyboardMouse, EKeyId::eKI_Mouse1);

	Revive();
}

uint64 CPlayerComponent::GetEventMask() const
{
	return BIT64(ENTITY_EVENT_START_GAME) | BIT64(ENTITY_EVENT_UPDATE);
}

void CPlayerComponent::ProcessEvent(SEntityEvent& event)
{
	switch (event.event)
	{
	case ENTITY_EVENT_UPDATE:
	{
		SEntityUpdateContext* pCtx = (SEntityUpdateContext*)event.nParam[0];

		// Start by updating the movement request we want to send to the character controller
		// This results in the physical representation of the character moving
		if (m_pInputComponent != nullptr) UpdateMovementRequest(pCtx->fFrameTime);

		// Update the animation state of the character
		UpdateAnimation(pCtx->fFrameTime);

		// Update the camera component offset
		if (m_pCameraComponent != nullptr) UpdateCamera(pCtx->fFrameTime);
	}
	break;
	}
}

void CPlayerComponent::UpdateMovementRequest(float frameTime)
{
	// Don't handle input if we are in air
	if (!m_pCharacterController->IsOnGround())
		return;

	Vec3 velocity = ZERO;

	float moveSpeed = 20.5f;

	if (m_inputFlags & (TInputFlags)EInputFlag::MoveLeft)
	{
		velocity.x -= moveSpeed * frameTime;
	}
	if (m_inputFlags & (TInputFlags)EInputFlag::MoveRight)
	{
		velocity.x += moveSpeed * frameTime;
	}
	if (m_inputFlags & (TInputFlags)EInputFlag::MoveForward)
	{
		velocity.y += moveSpeed * frameTime;
	}
	if (m_inputFlags & (TInputFlags)EInputFlag::MoveBack)
	{
		velocity.y -= moveSpeed * frameTime;
	}
	m_spriting = m_inputFlags & (TInputFlags)EInputFlag::MoveSprint;
	if (m_spriting)
	{
		velocity *= 2;
	}
	m_pCharacterController->AddVelocity(GetEntity()->GetWorldRotation() * velocity);
}

void CPlayerComponent::UpdateAnimation(float frameTime)
{
	Ang3 ypr = CCamera::CreateAnglesYPR(Matrix33(m_lookOrientation));

	// Update entity rotation as the player turns
	// We only want to affect Z-axis rotation, zero pitch and roll
	ypr.y = 0;

	// Re-calculate the quaternion based on the corrected look orientation
	Quat correctedOrientation = Quat(CCamera::CreateOrientationYPR(ypr));

	ICharacterInstance *pCharacter = m_pAnimationComponent->GetCharacter();

	// Update the Mannequin tags
	m_pAnimationComponent->SetTagWithId(m_rotateTagId, m_pAnimationComponent->IsTurning());
	m_pAnimationComponent->SetTagWithId(m_walkTagId, m_pCharacterController->IsWalking());

	// Send updated transform to the entity, only orientation changes
	GetEntity()->SetPosRotScale(GetEntity()->GetWorldPos(), correctedOrientation, Vec3(1, 1, 1));

	const Vec3 playerEyePosition = GetEntity()->GetWorldTM().TransformPoint(Vec3(0, 0, 1.6f));
	ISkeletonPose * pSkeletonPose = pCharacter->GetISkeletonPose();
	{
		IAnimationPoseBlenderDir *pIPoseBlenderAim = pSkeletonPose->GetIPoseBlenderAim();
		Vec3 aimDirection = playerEyePosition + (Vec3Constants<float>::fVec3_OneY * (m_lookOrientation).GetInverted());
		if (pIPoseBlenderAim)
		{
			pIPoseBlenderAim->SetTarget(aimDirection);
			pIPoseBlenderAim->SetPolarCoordinatesSmoothTimeSeconds(0.f);
		}
	}
}

void CPlayerComponent::UpdateCamera(float frameTime)
{
	// Start with updating look orientation from the latest input
	Ang3 ypr = CCamera::CreateAnglesYPR(Matrix33(m_lookOrientation));

	const float rotationSpeed = 0.002f;

	ypr.x += m_mouseDeltaRotation.x * rotationSpeed;

	const float rotationLimitsMinPitch = -0.84f;
	const float rotationLimitsMaxPitch = 1.5f;

	// TODO: Perform soft clamp here instead of hard wall, should reduce rot speed in this direction when close to limit.
	ypr.y = CLAMP(ypr.y + m_mouseDeltaRotation.y * rotationSpeed, rotationLimitsMinPitch, rotationLimitsMaxPitch);
	// Skip roll
	ypr.z = 0;

	m_lookOrientation = Quat(CCamera::CreateOrientationYPR(ypr));

	// Reset every frame
	m_mouseDeltaRotation = ZERO;

	// Ignore z-axis rotation, that's set by CPlayerAnimations
	ypr.x = 0;

	// Start with changing view rotation to the requested mouse look orientation
	Matrix34 localTransform = IDENTITY;
	localTransform.SetRotation33(CCamera::CreateOrientationYPR(ypr));

	const float viewOffsetForward = -1.5f;
	const float viewOffsetUp = 2.f;

	// Offset the player along the forward axis (normally back)
	// Also offset upwards
	localTransform.SetTranslation(Vec3(0, viewOffsetForward, viewOffsetUp));

	m_pCameraComponent->SetTransformMatrix(localTransform);
}

void CPlayerComponent::Revive()
{
	// Find a spawn point and move the entity there
	SpawnAtSpawnPoint();

	// Unhide the entity in case hidden by the Editor
	GetEntity()->Hide(false);

	// Make sure that the player spawns upright
	GetEntity()->SetWorldTM(Matrix34::Create(Vec3(1, 1, 1), IDENTITY, GetEntity()->GetWorldPos()));

	// Apply character to the entity
	m_pAnimationComponent->ResetCharacter();
	m_pCharacterController->Physicalize();

	// Reset input now that the player respawned
	m_inputFlags = 0;
	m_mouseDeltaRotation = ZERO;
	m_lookOrientation = IDENTITY;

	// Current animation component does not support passing extra flags to queue a fragment, so cannot set up aimpose through there
	IActionController *pActionController = gEnv->pGameFramework->GetMannequinInterface().FindActionController(*GetEntity());
	if (pActionController != nullptr)
	{
		FragmentID aimPoseFragmentId = pActionController->GetContext().controllerDef.m_fragmentIDs.Find("aimPose");
		m_pAimPoseAction = new TAction<SAnimationContext>(1, aimPoseFragmentId, TAG_STATE_EMPTY, IAction::NoAutoBlendOut | IAction::Interruptable);
		pActionController->Queue(*m_pAimPoseAction.get());
	}
}

Quat CPlayerComponent::GetLookOrientation() const
{
	return m_lookOrientation;
}

void CPlayerComponent::SetLookOrientation(Quat quat)
{
	m_lookOrientation = quat;
}

void CPlayerComponent::SpawnAtSpawnPoint()
{
	// Spawn at first default spawner
	auto *pEntityIterator = gEnv->pEntitySystem->GetEntityIterator();
	pEntityIterator->MoveFirst();

	std::vector<CSpawnPointComponent  *> spawners;
	while (!pEntityIterator->IsEnd())
	{
		IEntity *pEntity = pEntityIterator->Next();

		if (auto* pSpawner = pEntity->GetComponent<CSpawnPointComponent>())
		{
			spawners.push_back(pSpawner);
		}
	}
	if (spawners.size() > 0)
	{
		int index = cry_random<int>(0, spawners.size() - 1);
		spawners[index]->SpawnEntity(m_pEntity);
	}
	
}

void CPlayerComponent::HandleInputFlagChange(TInputFlags flags, int activationMode, EInputFlagType type)
{
	switch (type)
	{
	case EInputFlagType::Hold:
	{
		if (activationMode == eIS_Released)
		{
			m_inputFlags &= ~flags;
		}
		else
		{
			m_inputFlags |= flags;
		}
	}
	break;
	case EInputFlagType::Toggle:
	{
		if (activationMode == eIS_Released)
		{
			// Toggle the bit(s)
			m_inputFlags ^= flags;
		}
	}
	break;
	}
}