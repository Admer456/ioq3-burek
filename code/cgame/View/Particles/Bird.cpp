#include "cg_local.hpp"
#include "View/ParticleManager.hpp"
#include "Bird.hpp"

using namespace Particles;

void Bird::CreateBird( Vector origin, Vector birthPlace )
{
	ParticleManager* pm = GetClient()->GetParticleManager();
	
	uint32_t pid = pm->AddParticle<Bird>( 0, origin, 1.0f, 0.0f );
	BaseParticle& p = pm->GetParticle( pid );

	p.re.hModel = trap_R_RegisterModel( "models/props/bird_lasta.iqm" );
	p.re.customShader = 0;
	p.re.reType = RT_MODEL;

	p.particleOrigin = origin;
	p.particleVelocity = Vector( crandom() * 200.0f, crandom() * 200.0f, 50.0f );
	p.particleAngularVelocity = Vector::Identity;
	p.particleScale = (0.6f + random() * 0.9f) * 2.0f;

	p.particleGenericTimer = 0;
	p.particleInactivityTimer = 0;

	// Encode the "birth" place into particleColour
	p.particleColour = birthPlace;
}

void Bird::ParticleUpdate( BaseParticle* self, const float& time )
{
	const float frameTime = cg.frametime * 0.001f;

	BaseParticle::ParticleUpdate( self, time );

	if ( time > self->particleGenericTimer )
	{
		// Catapult around the birth place
		Vector bounceBack = self->particleColour - self->particleOrigin;
		bounceBack.z = crandom() * 0.08f;
		self->particleAngularVelocity = bounceBack.Normalized();

		self->particleGenericTimer = time + 3.0f + random() * 4.0f;
	}

	if ( time > self->particleInactivityTimer )
	{
		// Check ground'n'stuff
		trace_t tr;
		CG_Trace( &tr, self->particleOrigin, nullptr, nullptr, self->particleOrigin + self->particleVelocity * 1.5f, -1, MASK_SOLID );
		
		if ( !tr.startsolid )
		{
			if ( tr.fraction != 1.0f )
			{
				// Bounce off
				Vector bounceDirection = self->particleVelocity.Normalized().Reflect( tr.plane.normal );
				self->particleAngularVelocity = bounceDirection;

				self->particleInactivityTimer = time + 0.05f;
			}
			else
			{
				self->particleInactivityTimer = time + 0.2f;
			}

			CG_Trace( &tr, self->particleOrigin, nullptr, nullptr, self->particleOrigin - Vector( 0, 0, 512 ), -1, MASK_SOLID );

			if ( tr.fraction == 1.0f )
			{
				self->particleAngularVelocity.z = -0.2f;
				self->particleAngularVelocity.Normalize();

				self->particleInactivityTimer = time + 0.1f;
			}
			else if ( tr.fraction < 0.15625f ) // 80 units above floor
			{
				self->particleAngularVelocity.z = 0.6f;
			}
		}
		else // If in ground, well...
		{
			self->particleOrigin = self->particleColour;
		}
	}

	// Determine angles
	self->particleAngles = self->particleVelocity.ToAngles();
	self->particleAngles.z += sin( time * 0.333f * self->particleScale ) * 15.0f;

	// Update origin
	self->particleOrigin += self->particleVelocity * frameTime;

	// Determine movement direction
	Vector currentDirection = self->particleVelocity.Normalized();
	Vector wishDirection = self->particleAngularVelocity; // we "encode" this into particleAngularVelocity...
	
	self->particleVelocity = (currentDirection * 0.99f + wishDirection * 0.01f) * 400.0f;
}

void Bird::ParticleRender( BaseParticle* self, const float& time )
{
	BaseParticle::ParticleRender( self, time );
}
