#pragma once

namespace Particles
{
	class BaseParticle;

	class Puddle : public BaseParticle
	{
	public:
		static void CreatePuddle( qhandle_t material, Vector origin, float startSize, float endSize, float life );

		static void ParticleUpdate( BaseParticle* self, const float& time );
		static void ParticleRender( BaseParticle* self, const float& time );
	};
}