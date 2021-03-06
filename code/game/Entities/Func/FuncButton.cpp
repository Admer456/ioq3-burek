#include "Maths/Vector.hpp"

#include "Game/g_local.hpp"
#include "Entities/BaseEntity.hpp"
#include "Entities/KeyValueElement.hpp"
#include "Game/GameWorld.hpp"
#include "../qcommon/IEngineExports.h"
#include "Game/IGameImports.h"

#include "FuncButton.hpp"

using namespace Entities;

DefineEntityClass( "func_button", FuncButton, BaseEntity );

void FuncButton::Spawn()
{
	BaseEntity::Spawn();

	// This thing moves
	GetState()->eType = ET_MOVER;

	SetUse( &FuncButton::ButtonUse );
	SetTouch( &FuncButton::ButtonTouch );
	GetShared()->contents |= CONTENTS_TRIGGER;

	resetTime = spawnArgs->GetFloat( "wait", 0.5f );
}

void FuncButton::Precache()
{
	const char* soundFile = spawnArgs->GetCString( "sound", "sound/buttons/default.wav" );
	buttonSound = gameWorld->PrecacheSound( soundFile );
}

void FuncButton::ButtonThink()
{
	SetUse( &FuncButton::ButtonUse );
	SetTouch( &FuncButton::ButtonTouch );
}

void FuncButton::ButtonUse( IEntity* activator, IEntity* caller, float value )
{
	if ( spawnFlags & SF_NoUse && caller != this )
		return;

	UseTargets( activator );
	SetTouch( nullptr );
	SetUse( nullptr );

	gameWorld
		->CreateTempEntity( GetCurrentOrigin() + GetAverageOrigin(), EV_GENERAL_SOUND )
		->GetState()->eventParm = buttonSound;

	SetThink( &FuncButton::ButtonThink );
	nextThink = level.time * 0.001f + resetTime;

	if ( resetTime <= 0.0f )
	{
		SetThink( nullptr );
		nextThink = 0.0f;
	}
}

void FuncButton::ButtonTouch( IEntity* other, trace_t* trace )
{
	if ( spawnFlags & SF_NoTouch )
		return;

	Use( other, this, 0 );
}
