#pragma once

namespace Particles
{
	class BaseParticle;
}

// Particles are still somewhat OOP, but not in a typical way
// This way, you can't have extra variables for new particle "classes", but you can have new functionality
// This was all done so I don't have to use new and delete, performance reasons
// You are free to implement your own, and I'm certain it may be better than mine, which I might gladly accept in a PR
using ParticleUpdateFn = void( Particles::BaseParticle* self, const float& time );
using ParticleRenderFn = void( Particles::BaseParticle* self, const float& time );

namespace Particles
{
	class BaseParticle
	{
	public:
		static constexpr uint32_t ParticleFlag_Expensive = 1 << 0U;
		
		void				Create( qhandle_t material, Vector origin, Vector angles, Vector velocity, 
									Vector angularVelocity, float scale, float start, float life,
									ParticleUpdateFn updateFunction, ParticleRenderFn renderFunction );
		void				Update( const float& time );
		void				Render( const float& time );

		static void			ParticleUpdate( Particles::BaseParticle* self, const float& time );
		static void			ParticleRender( Particles::BaseParticle* self, const float& time );

		static bool			ForceUpdateAll;

		bool				active{ false };

		qhandle_t			particleMaterial{ 0U };
		Vector				particleOrigin{ Vector::Zero };
		Vector				particleAngles{ Vector::Zero };
		Vector				particleVelocity{ Vector::Zero };
		Vector				particleAngularVelocity{ Vector::Zero };
		float				particleScale{ 1.0f };

		float				particleStart{ 0.0f };
		float				particleLife{ 1.0f };
		float				particleInactivityTimer{ 0.0f };
		uint32_t			particleFlags{ 0U };

		Vector				particleGroundNormal{ Vector::Zero };
		sfxHandle_t			particleFrictionSound{ 0U };
		refEntity_t			re;

		ParticleUpdateFn*	update;
		ParticleRenderFn*	render;
	};
}
