#include "Maths/Vector.hpp"

#include "Game/g_local.hpp"
#include "Entities/BaseEntity.hpp"
#include "Entities/KeyValueElement.hpp"
#include "Game/GameWorld.hpp"
#include "../qcommon/IEngineExports.h"
#include "Game/IGameImports.h"

#include "EnvSprite.hpp"

using namespace Entities;

DefineEntityClass( "env_sprite", EnvSprite, BaseEntity );

void EnvSprite::Spawn()
{
	BaseEntity::Spawn();

	GetState()->eType = ET_SPRITE;
	GetState()->modelindex = spriteHandle;

	SetUse( &EnvSprite::SpriteUse );

	if ( spawnFlags & SF_StartOff )
	{
		SpriteToggle();
	}

	// Sprite scale and opacity encoded in state origin2
	// I know, it's bad, but I don't wanna add new stuff to entityState_t yet
	GetState()->origin2[0] = spriteRadius = spawnArgs->GetFloat( "radius", 64.0f );
	GetState()->origin2[1] = spriteOpacity = spawnArgs->GetInt( "opacity", 255 );
}

void EnvSprite::Precache()
{
	spriteHandle = gameWorld->PrecacheMaterial( spawnArgs->GetCString( "sprite", "sprites/default" ) );
}

void EnvSprite::SpriteUse( IEntity* activator, IEntity* caller, float value )
{
	SpriteToggle();

	if ( !spriteState && spawnFlags & SF_Once )
		Remove();
}

void EnvSprite::SpriteToggle()
{
	spriteState = !spriteState;

	GetState()->modelindex = spriteState ? spriteHandle : 0;
}
