#include "cg_local.hpp"
#include "ComplexEventHandler.hpp"
#include "View/ParticleManager.hpp"
#include "View/Particles/BaseGib.hpp"
#include "View/Particles/Smoke.hpp"

class BloodPuff final : public EventParser
{
public:
	BloodPuff( int ev );

	void RegisterAssets() override
	{

	}

	void Parse( centity_t* cent, Vector position ) override
	{
		for ( int i = 0; i < 6; i++ )
		{
			Vector r = Vector( crandom(), crandom(), crandom() );

			Particles::Smoke::CreateSmoke
			(
				cgs.media.bloodPuff, // material
				cent->currentState.origin, // position
				r * 120.0f, // velocity
				Vector::Identity, // tint
				0.05f + random() * 0.15f, // scale
				0.1f, // fade in time
				0.5f, // fade out time
				0.7f // life time
			);
		}
	}
};

RegisterEventParser( CE_BloodPuff, BloodPuff )