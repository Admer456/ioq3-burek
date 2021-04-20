#include "Game/g_local.hpp"
#include "Entities/BaseEntity.hpp"
#include "Entities/KeyValueElement.hpp"
#include "Game/GameWorld.hpp"
#include "../qcommon/IEngineExports.h"
#include "Game/IGameImports.h"

#include "AudioAmbient.hpp"

using namespace Entities;

DefineEntityClass( "audio_ambient", AudioAmbient, BaseEntity );

void AudioAmbient::Spawn()
{
	BaseEntity::Spawn();

	// Unfortunately, there's no way of defining a sound radius...
	//GetState()->eType = ET_SPEAKER;

	if ( spawnFlags & SF_StartOff )
	{
		GetState()->loopSound = 0;
	}
	else
	{
		GetState()->loopSound = sound;
		GetShared()->plow = NetPlow_ForceAll;
	}

	SetUse( &AudioAmbient::AmbientUse );
}

void AudioAmbient::Precache()
{
	BaseEntity::Precache();

	const char* audioFile = spawnArgs->GetCString( "sound", nullptr );
	if ( nullptr == audioFile )
	{
		Remove();
		return;
	}

	sound = gameWorld->PrecacheSound( audioFile );
}

void AudioAmbient::AmbientUse( IEntity* activator, IEntity* caller, float value )
{
	if ( GetState()->loopSound )
	{
		GetState()->loopSound = 0;
		GetShared()->plow = NetPlow_None;
	}
	else
	{
		GetState()->loopSound = sound;
		GetShared()->plow = NetPlow_ForceAll;
	}
}
