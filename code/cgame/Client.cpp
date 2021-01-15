#include "cg_local.hpp"
#include "Events/ComplexEventHandler.hpp"
#include <string>

Client client;

Client::Client()
{
	complexEventHandler = new ComplexEventHandler();
}

ClientEntities::BaseClientWeapon* Client::GetCurrentWeapon()
{
	ClientEntities::BaseClientWeapon* weapon = gWeapons[cg.snap->ps.weapon];
	if ( weapon )
		return weapon;

	return nullptr;
}

void Client::ParseComplexEvent( int id, centity_t* cent, Vector position )
{
	if ( !complexEventHandler->ParseComplexEvent( id, cent, position ) )
		CG_Error( "Unknown complex event %i, from ent %i\n", id, cent->currentState.number );
}

void Client::RegisterModelConfigData( int id, const char* modelName )
{
	if ( nullptr == modelName )
	{
		Util::PrintError( va( "Client::RegisterModelConfigData\nmodelName = nullptr\n" ) );
		return;
	}

	if ( id < 0 || id >= MAX_MODELS )
	{
		Util::PrintError( va( "Client::RegisterModelConfigData:\nout of range (%i) for '%s'\n", id, modelName ) );
		return;
	}

	std::string modelConfig = modelName;
	modelConfig = modelConfig.substr( 0, modelConfig.size() - 4U );
	modelConfig += ".mcfg";

	fileHandle_t f;
	if ( Util::FileOpen( modelConfig.c_str(), &f, FS_READ ) == -1 )
	{
		return; // file doesn't exist, let's silently return
	}

	anims[id] = Assets::ModelConfigData::GetAnimations( modelConfig.c_str() );
}

std::vector<Assets::ModelAnimation>& Client::GetAnimationsForModel( qhandle_t modelindex )
{
	return anims[modelindex];
}

Client* GetClient()
{
	return &client;
}
