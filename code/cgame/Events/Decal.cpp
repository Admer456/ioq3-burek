#include "cg_local.hpp"
#include "ComplexEventHandler.hpp"

class Decal final : public EventParser
{
public:
	Decal( int ev );

	void Parse( centity_t* cent, Vector position ) override
	{
		EventData ed( cent->currentState );
		
		qhandle_t material = cgs.gameMaterials[ed.model];
		Vector direction = ed.vparm;
		float orientation = ed.fparm;
		float radius = ed.fparm2;

		if ( direction.Length() > 0.1f )
		{
			CG_ImpactMark( material, cent->currentState.origin, direction, orientation, 1.0f, 1.0f, 1.0f, 1.0f, true, radius, false );
		}
	}
};

RegisterEventParser( CE_Decal, Decal );
