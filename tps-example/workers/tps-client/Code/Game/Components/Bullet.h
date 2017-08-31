#pragma once
#include "GamePlugin.h"

////////////////////////////////////////////////////////
// Physicalized bullet shot from weaponry, expires on collision with another object
////////////////////////////////////////////////////////
class CBulletComponent final : public IEntityComponent
{
public:
	virtual ~CBulletComponent() {}

	static void ReflectType(Schematyc::CTypeDesc<CBulletComponent>& desc)
	{
		desc.SetGUID("{8625C882-F443-46ED-B7CA-BC7815814C1D}"_cry_guid);
	}

	// IEntityComponent
	virtual void Initialize() override
	{
		
		// Set the model
		const int geometrySlot = 0;
		m_pEntity->LoadGeometry(geometrySlot, "Objects/Default/primitive_sphere.cgf");

		// Load the custom bullet material.
		// This material has the 'mat_bullet' surface type applied, which is set up to play sounds on collision with 'mat_default' objects in Libs/MaterialEffects
		auto *pBulletMaterial = gEnv->p3DEngine->GetMaterialManager()->LoadMaterial("Materials/bullet");
		m_pEntity->SetMaterial(pBulletMaterial);

		// Now create the physical representation of the entity
		SEntityPhysicalizeParams physParams;
		physParams.type = PE_PARTICLE;
		physParams.mass = 5.f;
		pe_params_particle ppart;
		ppart.flags = particle_traceable | particle_no_roll | pef_log_collisions;
		ppart.mass = 5.f;
		ppart.size = 0.1f;
		ppart.thickness = 0.1f;
		ppart.gravity = Vec3(0.0f, 0.0f, -9.81f);
		ppart.kAirResistance = 0.1f;
		ppart.surface_idx = 0;
		ppart.velocity = 20.0f;
		ppart.heading = GetEntity()->GetWorldRotation().GetColumn1();
		physParams.pParticle = &ppart;
		m_pEntity->Physicalize(physParams);

		// Make sure that bullets are always rendered regardless of distance
		// Ratio is 0 - 255, 255 being 100% visibility
		GetEntity()->SetViewDistRatio(255);

		// Apply an impulse so that the bullet flies forward
		if (auto *pPhysics = GetEntity()->GetPhysics())
		{
			pe_action_impulse impulseAction;

			const float initialVelocity = 250.f;

			// Set the actual impulse, in this cause the value of the initial velocity CVar in bullet's forward direction
			impulseAction.impulse = GetEntity()->GetWorldRotation().GetColumn1() * initialVelocity;

			// Send to the physical entity
			pPhysics->Action(&impulseAction);
		}
	}

	// ~IEntityComponent
};