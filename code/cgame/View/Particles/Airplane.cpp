#include "cg_local.hpp"
#include "View/ParticleManager.hpp"
#include "Airplane.hpp"
#include "Smoke.hpp"

using namespace Particles;

void Airplane::CreateAirplane( Vector origin, float yaw )
{
	ParticleManager* pm = GetClient()->GetParticleManager();

	uint32_t pid = pm->AddParticle<Airplane>( 0, origin, 1.0f, 100.0f );
	BaseParticle& p = pm->GetParticle( pid );

	p.particleOrigin = origin;
	p.particleOrigin.x -= cosf( yaw ) * 12000.0f;
	p.particleOrigin.y -= sinf( yaw ) * 12000.0f;

	p.particleVelocity.x = cosf( yaw ) * 600.0f;
	p.particleVelocity.y = sinf( yaw ) * 600.0f;

	p.particleAngles = p.particleVelocity.ToAngles();

	p.particleScale = 25.0f;

	p.re.reType = RT_MODEL;
	p.re.hModel = trap_R_RegisterModel( "models/props/airplane.iqm" );
	p.re.customShader = 0;
}

void Airplane::ParticleUpdate( BaseParticle* self, const float& time )
{
	BaseParticle::ParticleUpdate( self, time );

	const float frameTime = cg.frametime * 0.001f;

	// Update origin
	self->particleOrigin += self->particleVelocity * frameTime;

	// Emit smoke every once in a while
	if ( self->particleGenericTimer > 1.5f )
	{
		Smoke::CreateSmoke( cgs.media.smokeSprites[rand() % 3], self->particleOrigin, Vector::Zero, Vector( 0.68f, 0.89f, 1.00f ), 6.0f, 5.0f, 15.0f, 30.0f, false, true );
		self->particleGenericTimer = 0.0f;
	}

	self->particleGenericTimer += frameTime;
}

void Airplane::ParticleRender( BaseParticle* self, const float& time )
{
	BaseParticle::ParticleRender( self, time );
}
