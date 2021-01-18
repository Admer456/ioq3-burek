#include "Maths/Vector.hpp"

#include "Game/g_local.hpp"
#include "Entities/BaseEntity.hpp"
#include "Entities/KeyValueElement.hpp"
#include "Game/GameWorld.hpp"
#include "../qcommon/IEngineExports.h"
#include "Game/IGameImports.h"

#include "AudioMusicSet.hpp"

using namespace Entities;

DefineEntityClass( "audio_music_set", AudioMusicSet, BaseEntity );

void AudioMusicSet::Spawn()
{
	BaseEntity::Spawn();
	Q_strncpyz( label, spawnArgs->GetCString( "label", "action" ), sizeof( label ) );
}

void AudioMusicSet::Use( IEntity* activator, IEntity* caller, float value )
{
	gameImports->SetConfigString( CS_MUSIC, label );
}
