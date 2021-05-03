#include "cg_local.hpp"
#include "BaseParticle.hpp"

using namespace Particles;

bool BaseParticle::ForceUpdateAll = false;

void BaseParticle::Create( qhandle_t material, Vector origin, Vector angles, Vector velocity,
						   Vector angularVelocity, float scale, float start, float life,
						   ParticleUpdateFn updateFunction, ParticleRenderFn renderFunction )
{
	update = updateFunction;
	render = renderFunction;

	if ( nullptr == update || nullptr == render )
	{
		// Give up immediately, this particle will probably never be rendered
		active = false;
		return;
	}

	memset( &re, 0, sizeof( re ) );

	particleMaterial = material;
	particleOrigin = origin;
	particleAngles = angles;
	particleVelocity = velocity;
	particleAngularVelocity = angularVelocity;
	particleScale = scale;
	particleStart = start;
	particleLife = life;

	re.reType = RT_SPRITE;
	re.hModel = material;
	re.customShader = material;
	re.shaderRGBA[0] = 255U;
	re.shaderRGBA[1] = 255U;
	re.shaderRGBA[2] = 255U;
	re.shaderRGBA[3] = 200U;
	re.origin << origin;
	re.radius = scale;

	active = true;
}

void BaseParticle::Update( const float& time )
{
	update( this, time );
}

void BaseParticle::Render( const float& time )
{
	render( this, time );
}

void BaseParticle::ParticleUpdate( Particles::BaseParticle* self, const float& time )
{
	if ( self->particleLife == 0.0f )
		return;

	if ( self->particleStart + self->particleLife < time )
		self->active = false;
}

void BaseParticle::ParticleRender( Particles::BaseParticle* self, const float& time )
{
	self->re.origin << self->particleOrigin;
	AnglesToAxis( self->particleAngles, self->re.axis );

	for ( int i = 0; i < 3; i++ )
		for ( int o = 0; o < 3; o++ )
			self->re.axis[i][o] *= self->particleScale;

	trap_R_AddRefEntityToScene( &self->re );
}
