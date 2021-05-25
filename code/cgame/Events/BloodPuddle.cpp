#include "cg_local.hpp"
#include "ComplexEventHandler.hpp"
#include "View/ParticleManager.hpp"
#include "View/Particles/BaseGib.hpp"
#include "View/Particles/Puddle.hpp"

class BloodPuddle final : public EventParser
{
public:
	BloodPuddle( int ev );

	void RegisterAssets() override
	{

	}

	void Parse( centity_t* cent, Vector position ) override
	{
		Vector start = cent->currentState.origin;
		start += Vector( 0, 0, 32 );
		Vector end = start - Vector( 0, 0, 256 );
		trace_t tr;

		CG_Trace( &tr, start, nullptr, nullptr, end, -1, MASK_SOLID );

		end = tr.endpos;
		Particles::Puddle::CreatePuddle
		( 
			cgs.media.bloodPuddle, // material
			end + Vector( 0, 0, 0.5f ), // position 
			10.0f,  // start size
			60.0f, // end size
			120.0f // life
		);
	}
};

RegisterEventParser( CE_BloodPuddle, BloodPuddle )
