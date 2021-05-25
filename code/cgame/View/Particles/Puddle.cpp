#include "cg_local.hpp"
#include "View/ParticleManager.hpp"
#include "Puddle.hpp"

using namespace Particles;

void Puddle::CreatePuddle( qhandle_t material, Vector origin, float startSize, float endSize, float life )
{
	ParticleManager* pm = GetClient()->GetParticleManager();

	uint32_t pid = pm->AddParticle<Puddle>( material, origin, startSize, life );
	BaseParticle& p = pm->GetParticle( pid );

	p.re.reType = RT_MODEL;
	p.re.hModel = cgs.media.quadModel;
	p.re.customShader = material;
	p.re.shaderRGBA[3] = 255U;

	p.particleOrigin = origin;
	p.particleAngles = Vector::Zero;

	p.particleAngularVelocity.x = startSize;
	p.particleAngularVelocity.y = endSize;
}

void Puddle::ParticleUpdate( BaseParticle* self, const float& time )
{
	BaseParticle::ParticleUpdate( self, time );
	
	float timeUntilDeath = (self->particleStart + self->particleLife) - GetClient()->Time();
	float fraction = timeUntilDeath / self->particleLife;

	const float& startSize = self->particleAngularVelocity.x;
	const float& endSize = self->particleAngularVelocity.y;

	self->particleScale = (fraction * startSize) + ((1.0f - fraction) * endSize);

	if ( timeUntilDeath < 100.0f )
	{
		float alpha = timeUntilDeath / 101.0f;
		self->re.shaderRGBA[3] = alpha * 255.0f;
	}
}

void Puddle::ParticleRender( BaseParticle* self, const float& time )
{
	BaseParticle::ParticleRender( self, time );
}
