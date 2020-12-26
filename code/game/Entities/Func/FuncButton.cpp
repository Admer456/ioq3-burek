#include "Maths/Vector.hpp"

#include "Game/g_local.hpp"
#include "Entities/BaseEntity.hpp"
#include "Entities/KeyValueElement.hpp"
#include "Game/GameWorld.hpp"
#include "../qcommon/IEngineExports.h"
#include "Game/IGameImports.h"

#include "FuncButton.hpp"

using namespace Entities;

DefineEntityClass( "func_button", FuncButton, BaseQuakeEntity );

void FuncButton::Spawn()
{
	BaseQuakeEntity::Spawn();

	// This thing moves
	GetState()->eType = ET_MOVER;

	SetUse( &FuncButton::ButtonUse );
	SetTouch( &FuncButton::ButtonTouch );
	GetShared()->contents |= CONTENTS_TRIGGER;

	if ( spawnFlags & SF_NoTouch )
	{
		SetTouch( nullptr );
		GetShared()->contents &= ~CONTENTS_TRIGGER;
	}

	if ( spawnFlags & SF_NoUse )
	{
		SetUse( nullptr );
	}
}

void FuncButton::Precache()
{
	const char* soundFile = spawnArgs->GetCString( "sound", "sound/buttons/default.wav" );
	buttonSound = G_SoundIndex( const_cast<char*>(soundFile) );
}

void FuncButton::ButtonUse( IEntity* activator, IEntity* caller, float value )
{
	UseTargets( activator );
	SetTouch( nullptr );
	SetUse( nullptr );

	AddEvent( EV_GENERAL_SOUND, buttonSound );
}

void FuncButton::ButtonTouch( IEntity* other, trace_t* trace )
{
	engine->Print( "FuncButton::ButtonTouch\n" );

	ButtonUse( other, other, 0 );
}
