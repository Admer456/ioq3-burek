#include "Game/g_local.hpp"
#include "Entities/BaseEntity.hpp"
#include "Game/GameWorld.hpp"
#include "../qcommon/IEngineExports.h"
#include "Game/IGameImports.h"

#include "Audio2D.hpp"

using namespace Entities;

DefineEntityClass( "audio_2d", Audio2D, BaseEntity );

void Audio2D::Spawn()
{
	sound = gameWorld->PrecacheSound( spawnArgs->GetCString( "sound", "sound/humans/pain2.wav" ) );
}

void Audio2D::Use( IEntity* activator, IEntity* caller, float value )
{
	if ( played )
	{
		return;
	}

	gameWorld
		->CreateTempEntity( GetOrigin(), EV_GLOBAL_SOUND )
		->GetState()->eventParm = sound;

	played = true;
}
