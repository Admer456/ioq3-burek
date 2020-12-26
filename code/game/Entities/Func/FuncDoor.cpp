#include "Maths/Vector.hpp"

#include "Game/g_local.hpp"
#include "Entities/BaseEntity.hpp"
#include "Entities/KeyValueElement.hpp"
#include "Game/GameWorld.hpp"
#include "../qcommon/IEngineExports.h"
#include "Game/IGameImports.h"
#include "Entities/BasePlayer.hpp"
#include "Entities/Base/BaseMover.hpp"

#include "FuncDoor.hpp"

using namespace Entities;

DefineEntityClass( "func_door", FuncDoor, BaseMover );

void FuncDoor::Spawn()
{
	BaseMover::Spawn();

	expand = spawnArgs->GetFloat( "expand", 1.0f );
	distance = spawnArgs->GetFloat( "distance", 64.0f );
	axis = spawnArgs->GetVector( "axis", Vector( 0, 0, 1 ) );
	speed = spawnArgs->GetFloat( "speed", 256.0f );

	openedPos = GetOrigin();
	closedPos = openedPos + axis * distance;

	if ( openedPos == closedPos )
	{
		closedPos = openedPos + Vector( 0, 0, 80 );
		engine->Print( "WARNING: door at %3.2f %3.2f %3.2f has the same open and closed position, check its 'axis' and 'distance'!\n" );
	}

	SetThink( &FuncDoor::DoorThink );

	if ( spawnFlags & SF_StartOpen )
	{
		doorState = Door_Opened;
	}
}

void FuncDoor::DoorThink()
{
	UpdateDoorState();
	SetThink( nullptr );
}

void FuncDoor::DoorUse( IEntity* activator, IEntity* caller, float value )
{
	UpdateDoorState();
	Disable();
}

void FuncDoor::DoorTouch( IEntity* other, trace_t* trace )
{
	UpdateDoorState();
	Disable();
}

void FuncDoor::OnClose()
{
	float requiredTime = distance / speed;

	trajectory_t* tr = &GetState()->pos;
	closedPos.CopyToArray( tr->trBase );
	(axis*-1).CopyToArray( tr->trDelta );
	tr->trDuration = requiredTime * 1000;
	tr->trTime = level.time;
	tr->trType = TR_LINEAR_STOP;

	nextThink = (level.time * 0.001f) + requiredTime;

	SetThink( &FuncDoor::DoorThink );

	doorState = Door_Closing;
}

void FuncDoor::OnCloseFinished()
{
	Enable();

	GetState()->pos.trType = TR_STATIONARY;

	doorState = Door_Closed;
}

void FuncDoor::OnOpen()
{
	float requiredTime = distance / speed;

	trajectory_t* tr = &GetState()->pos;
	openedPos.CopyToArray( tr->trBase );
	axis.CopyToArray( tr->trDelta );
	tr->trDuration = requiredTime * 1000;
	tr->trTime = level.time;
	tr->trType = TR_LINEAR_STOP;

	nextThink = (level.time * 0.001f) + requiredTime;

	SetThink( &FuncDoor::DoorThink );

	doorState = Door_Opening;
}

void FuncDoor::OnOpenFinished()
{
	Enable();

	GetState()->pos.trType = TR_STATIONARY;

	doorState = Door_Opened;
}

void FuncDoor::UpdateDoorState()
{
	switch ( doorState )
	{
	case Door_Closed: OnOpen(); break;
	case Door_Opened: OnClose(); break;
	case Door_Closing: OnCloseFinished(); break;
	case Door_Opening: OnOpenFinished(); break;
	}
}

void FuncDoor::Enable()
{
	SetTouch( &FuncDoor::DoorTouch );
	SetUse( &FuncDoor::DoorUse );
}

void FuncDoor::Disable()
{
	SetTouch( nullptr );
	SetUse( nullptr );
}
