#include "cg_local.hpp"

#include "ComplexEventHandler.hpp"

std::vector<EventParser*> ComplexEventHandler::parsers = std::vector<EventParser*>();

// ===================
// ComplexEventHandler::RegisterAssets
// ===================
void ComplexEventHandler::RegisterAssets()
{
	for ( EventParser* ep : parsers )
	{
		ep->RegisterAssets();
	}
}

// ===================
// ComplexEventHandler::ParseComplexEvent
// ===================
bool ComplexEventHandler::ParseComplexEvent( int id, centity_t* cent, Vector position )
{
	for ( EventParser* ep : parsers )
	{
		if ( ep->id == id )
		{
			ep->Parse( cent, position );
			return true;
		}
	}

	return false;
}
