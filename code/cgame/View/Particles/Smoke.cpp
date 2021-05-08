#include "cg_local.hpp"
#include "View/ParticleManager.hpp"
#include "Smoke.hpp"

using namespace Particles;

void Smoke::CreateSmoke( qhandle_t smokeSprite, Vector origin, Vector velocity, 
						 Vector tintColour, float scale, float fadeInTime, 
						 float fadeOutTime, float life, bool expensiveCollision )
{
	ParticleManager* pm = GetClient()->GetParticleManager();

	uint32_t particleId = pm->AddParticle<Smoke>( smokeSprite, origin, scale, life );
	BaseParticle& particle = pm->GetParticle( particleId );

	particle.particleAngularVelocity.x = fadeInTime;
	particle.particleAngularVelocity.y = fadeOutTime;

	if ( fadeInTime > 0.0f )
	{
		particle.re.shaderRGBA[3] = 0;
		particle.particleFlags |= SmokeFlag_FadingIn;
	}

	particle.particleVelocity = velocity;
	particle.particleAngularVelocity.z = crandom() * 360.0f;

	particle.particleTintColour = tintColour;
}

void Smoke::ForceEffect( BaseParticle* self, const float& time )
{

}

void Smoke::ParticleUpdate( BaseParticle* self, const float& time )
{
	BaseParticle::ParticleUpdate( self, time );

	const float frameTime = cg.frametime * 0.001f;
	float timeFromStart = time - self->particleStart;
	float timeUntilDeath = (self->particleStart + self->particleLife) - time;

	self->particleOrigin += self->particleVelocity * frameTime;
	self->particleAngles.z += self->particleAngularVelocity.z * frameTime;

	// Smaller smoke will go up quicker, really big smoke will go down
	float gravityFactor = 1.0f - 0.333f * self->particleScale;
	gravityFactor = gravityFactor * gravityFactor * gravityFactor;

	self->particleVelocity.z += 10.0f * frameTime * gravityFactor;

	self->particleVelocity -= self->particleVelocity * frameTime * 0.4f;
	self->particleAngularVelocity.z -= self->particleAngularVelocity.z * frameTime * 0.7f;
	self->particleScale += frameTime * 0.15f;

	if ( self->particleFlags & SmokeFlag_FadingIn )
	{
		float fadeInTime = self->particleAngularVelocity.x;

		float fraction = timeFromStart / fadeInTime;
		self->re.shaderRGBA[3] = fraction * 255.0f;

		if ( fraction * 255.0f > 255.0f )
			self->re.shaderRGBA[3] = 255U;

		if ( fraction > 0.98f )
		{
			self->particleFlags &= ~(SmokeFlag_FadingIn);
		}
	}

	// Fade out time is encoded into y
	if ( timeUntilDeath <= self->particleAngularVelocity.y )
	{
		float fraction = timeUntilDeath / self->particleAngularVelocity.y;

		self->re.shaderRGBA[3] = fraction * 255.0f;
	}
}

void Smoke::ParticleRender( BaseParticle* self, const float& time )
{
	self->re.rotation = self->particleAngles.z;
	self->re.radius = self->particleScale * 64.0f;

	self->particleGenericTimer += cg.frametime * 0.001f;

	if ( self->particleGenericTimer > 0.25f )
	{
		Vector directLight, ambientLight, lightDir;
		trap_R_LightForPoint( self->particleOrigin, ambientLight, directLight, lightDir );

		self->particleTargetColour = directLight;

		self->particleGenericTimer = 0.0f;
	}

	self->particleColour = self->particleColour * 0.98f + self->particleTargetColour * 0.02f;

	self->re.shaderRGBA[0] = self->particleColour[0] * self->particleTintColour[0];
	self->re.shaderRGBA[1] = self->particleColour[1] * self->particleTintColour[1];
	self->re.shaderRGBA[2] = self->particleColour[2] * self->particleTintColour[2];

	BaseParticle::ParticleRender( self, time );
}
