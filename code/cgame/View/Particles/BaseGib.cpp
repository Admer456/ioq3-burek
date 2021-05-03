#include "cg_local.hpp"
#include "View/ParticleManager.hpp"
#include "BaseGib.hpp"

using namespace Particles;

void BaseGib::ParticleForceAffect( BaseParticle* self, const float& time )
{
	ParticleManager::ParticleForce* forces = GetClient()->GetParticleManager()->GetForces();
	const float frameTime = cg.frametime * 0.001f;
	Vector totalForce = Vector::Zero;
	float scalarForce = 0.0f;

	for ( int i = 0; i < ParticleManager::ParticleForce::MaxForces; i++ )
	{
		auto& f = forces[i];

		float force = f.GetForce( self->particleOrigin );
		scalarForce += force;
		totalForce += force * (self->particleOrigin - f.forceOrigin).Normalized();
	}

	if ( fabs( scalarForce ) > 40.0f )
	{
		self->particleVelocity += totalForce * frameTime += self->particleGroundNormal * frameTime * (scalarForce * scalarForce) * 0.01f;
		self->particleVelocity.z += 1200.0f * frameTime; // combat gravity

		self->particleAngularVelocity.x += crandom() * 720.0f;
		self->particleAngularVelocity.y += crandom() * 720.0f;
		self->particleAngularVelocity.z += crandom() * 720.0f;

		if ( self->particleInactivityTimer > 1.0f )
		{
			self->particleOrigin += self->particleGroundNormal;
			self->particleInactivityTimer = 0.0f; // Wake up the particle because it's affected

			// Forcefully bounce the particle off the ground
			Vector reflected = self->particleVelocity.Normalized();
			Vector projection = reflected.Reflect( self->particleGroundNormal, 3.0f );

			self->particleVelocity = projection * self->particleVelocity.Length() * 2.0f;
		}
	}
}

void BaseGib::CreateGib( qhandle_t model, Vector origin, Vector velocity, int materialType, sfxHandle_t sound, bool expensiveCollision )
{
	ParticleManager* particleManager = GetClient()->GetParticleManager();

	uint32_t particleId = particleManager->AddParticle<BaseGib>( 0, origin, 1.0f, 60.0f );
	BaseParticle& particle = particleManager->GetParticle( particleId );

	particle.re.reType = RT_MODEL;
	particle.re.customShader = 0;
	particle.re.hModel = model;
	particle.re.shaderRGBA[3] = 255;

	particle.particleOrigin = origin;
	particle.particleVelocity = velocity;
	particle.particleScale = random()*3.0f + 0.2f;

	particle.particleAngles.x = crandom() * 360.0f;
	particle.particleAngles.y = crandom() * 360.0f;
	particle.particleAngles.z = crandom() * 360.0f;

	particle.particleAngularVelocity.x = crandom() * 1080.0f;
	particle.particleAngularVelocity.y = crandom() * 720.0f;
	particle.particleAngularVelocity.z = crandom() * 1080.0f;

	particle.particleLife -= random() * 40.0f;

	particle.particleFrictionSound = sound;

	if ( expensiveCollision )
	{
		particle.particleFlags |= ParticleFlag_Expensive;
	}

	if ( materialType != Material_Glass )
	{
		particle.particleFlags |= GibFlag_Shrink;
	}
}

void BaseGib::ParticleUpdate( BaseParticle* self, const float& time )
{
	const float frameTime = cg.frametime * 0.001f;
	trace_t tr;

	BaseParticle::ParticleUpdate( self, time );

	// If I'm expensive, then you may wake me up again
	if ( self->particleFlags & ParticleFlag_Expensive )
	{	
		if ( BaseParticle::ForceUpdateAll )
		{
			self->particleInactivityTimer = 0.97f; // Approx. 1 frame of movement allowed
		}
	}

	// Scale down if near death
	float timeUntilDeath = (self->particleStart + self->particleLife) - time;
	if ( timeUntilDeath < 8.0f )
	{
		if ( self->particleFlags & GibFlag_Shrink )
		{
			self->particleScale -= self->particleScale * frameTime * 0.33f;
		}
		else
		{
			self->re.shaderRGBA[3] = (timeUntilDeath * 0.125f) * 255.f;
		}
	}

	// Apply forces
	ParticleForceAffect( self, time );

	// If I've been sitting here long enough, then stop updating me
	if ( self->particleInactivityTimer > 1.0f )
	{
		return;
	}

	// Apply gravity
	self->particleVelocity.z -= 981.0f * frameTime;

	// Slow down if underwater
	int contents;
	if ( self->particleFlags & ParticleFlag_Expensive ) 
	{
		contents = CG_PointContents( self->particleOrigin, -1 );
	}
	else
	{
		contents = trap_CM_PointContents( self->particleOrigin, 0 );
	}

	if ( contents & (CONTENTS_SLIME | CONTENTS_WATER | CONTENTS_LAVA) )
	{
		// Simulate "weight"
		float buoyancyFactor = 3.5f - (self->particleScale * 1.0f);

		self->particleVelocity -= 14.5f * buoyancyFactor * self->particleVelocity * frameTime;
		self->particleVelocity.z -= 25.5f * buoyancyFactor * self->particleVelocity.z * frameTime;
		self->particleAngularVelocity -= 0.6f * self->particleAngularVelocity * frameTime;
	}

	Vector newOrigin = self->particleOrigin + self->particleVelocity * frameTime;
	Vector bbox = self->particleScale * Vector( 2.5, 2.5, 2.5 );

	if ( self->particleFlags & ParticleFlag_Expensive )
	{	// Trace against all
		CG_Trace( &tr, self->particleOrigin, bbox * -1.0f, bbox, newOrigin, 0, MASK_SOLID );
	}
	else
	{	// Trace against world only
		trap_CM_BoxTrace( &tr, self->particleOrigin, newOrigin, bbox * -1.0f, bbox, 0, MASK_SOLID );
	}

	// If we're inside something, then push back
	if ( tr.startsolid )
	{
		newOrigin = self->particleOrigin - self->particleVelocity * frameTime;
	}

	// If we hit something
	if ( tr.fraction != 1.0f )
	{
		// Store the surface normal
		self->particleGroundNormal = tr.plane.normal;

		// Play a random friction sound if fast enough + given the 6% chance
		// TODO: maybe have different sounds for different velocities?
		// TODO 2: customisable sound pitch pls
		if ( self->particleVelocity.Length() > 200.0f && !(rand() % 16) )
		{
			trap_S_StartSound( self->particleOrigin, 0, CHAN_AUTO, self->particleFrictionSound );
		}

		// Bigger, heavier gibs will bounce a bit more along the normal than usual, but with more dampening
		//float reflection = 1.5f + (self->particleScale * 1.5f);
		float reflection = 1.5f;
		float bounceFactor = 0.85f - (self->particleScale * 0.10f);

		// Bounce!
		self->particleVelocity = self->particleVelocity.Normalized().Reflect( tr.plane.normal, reflection ).Normalized() * self->particleVelocity.Length() * bounceFactor;
		newOrigin = self->particleOrigin;

		// Dampen the spinning
		self->particleAngularVelocity = self->particleAngularVelocity * -0.8f;

		if ( contents & (CONTENTS_SLIME | CONTENTS_WATER | CONTENTS_LAVA) )
		{
			self->particleAngles.x /= 1.005f;
			self->particleAngles.z /= 1.005f;
		}
		else
		{
			self->particleAngles.x /= 1.15f;
			self->particleAngles.z /= 1.15f;
		}

		if ( self->particleVelocity.Length() < 30.0f )
		{
			self->particleInactivityTimer += frameTime;
		}
		else
		{
			self->particleInactivityTimer = 0;
		}
	}

	if ( self->particleFlags & ParticleFlag_Expensive )
	{
		centity_t& touchEnt = cg_entities[tr.entityNum];
		if ( touchEnt.currentValid && tr.entityNum != ENTITYNUM_WORLD && tr.entityNum != ENTITYNUM_NONE )
		{	// If it's a mover, compensate for it
			CG_AdjustPositionForMover( newOrigin, tr.entityNum, cg.physicsTime, cg.oldTime, newOrigin,
									   self->particleAngles, self->particleAngles );
		}
	}

	// Update
	self->particleOrigin = newOrigin;
	self->particleAngles += self->particleAngularVelocity * frameTime;
}

void BaseGib::ParticleRender( BaseParticle* self, const float& time )
{
	BaseParticle::ParticleRender( self, time );
}
