#include "cg_local.hpp"
#include "ComplexEventHandler.hpp"
#include "View/ParticleManager.hpp"
#include "View/Particles/Smoke.hpp"

class SmokePuff final : public EventParser
{
public:
	SmokePuff( int ev );

	void Parse( centity_t* cent, Vector position ) override
	{
		EventData ed( cent->currentState );
		auto pm = GetClient()->GetParticleManager();

		Vector origin = cent->currentState.origin;
		Vector velocity = ed.vparm * ed.fparm;
		float scale = ed.fparm2;

		for ( int i = 0; i < 3; i++ )
		{
			Vector randomVector = Vector( crandom(), crandom(), crandom() ) * velocity.Length() * 0.35f;

			Particles::Smoke::CreateSmoke( 
				cgs.media.smokeSprites[i], // use 3 smoke sprites
				origin, // start from the same spot
				velocity + randomVector, // randomise the initial velocity a lil bit 
				Vector( 0.7f, 0.73f, 0.78f ), // metal grey tint
				scale * 0.25f + i * 0.08f,
				0.2f + i * 0.1f, // fade in 0.1s
				2.3f, // fade out 1.0s
				3.0f + i * 0.5f  // lifetime 3.0s
			);
		}
	}
};

RegisterEventParser( CE_SmokePuff, SmokePuff );