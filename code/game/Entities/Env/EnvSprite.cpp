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

	GetState()->pos.trType = TR_INTERPOLATE;

	GetState()->eType = ET_SPRITE;
	GetState()->modelindex = spriteHandle;

	gameImports->LinkEntity( this );

	SetUse( &EnvSprite::SpriteUse );

	if ( spawnFlags & SF_StartOff )
	{
		SpriteToggle();
	}

	// Sprite scale and opacity encoded in state origin2
	// Tint colour is encoded in state angles2
	// I know, it's bad, but I don't wanna add new stuff to entityState_t yet
	GetState()->origin2[0] = spriteRadius = spawnArgs->GetFloat( "radius", 64.0f );
	GetState()->origin2[1] = spriteOpacity = spawnArgs->GetFloat( "opacity", 1.0f );
	GetState()->angles2 << spawnArgs->GetVector( "color", Vector::Identity );

	// The mapper must've input a range from 0 to 255
	if ( spriteOpacity > 1.01f )
	{
		if ( spriteOpacity > 255.0f )
			spriteOpacity = 255.0f;

		spriteOpacity /= 255.0f;
		GetState()->origin[1] = spriteOpacity;
	}
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
