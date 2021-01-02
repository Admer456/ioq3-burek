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

	GetShared()->contents |= CONTENTS_TRIGGER;

	expand = spawnArgs->GetFloat( "expand", 1.0f );
	distance = spawnArgs->GetFloat( "distance", 64.0f );
	axis = spawnArgs->GetVector( "axis", Vector( 0, 0, 1 ) );
	speed = spawnArgs->GetFloat( "speed", 256.0f );

	// Expand the absolute bounding box by expand's value,
	// so the door can open before the player runs into it
	Vector expandVector( expand, expand, expand );
	Vector absmax = GetShared()->absmax;
	Vector absmin = GetShared()->absmin;
	Vector maxs = GetShared()->maxs;
	Vector mins = GetShared()->mins;

	absmax += expandVector;
	absmin -= expandVector;
	maxs += expandVector;
	mins -= expandVector;

	absmax.CopyToArray( GetShared()->absmax );
	absmin.CopyToArray( GetShared()->absmin );
	maxs.CopyToArray( GetShared()->maxs );
	mins.CopyToArray( GetShared()->mins );

	closedPos = GetOrigin();
	openedPos = closedPos + axis * distance;

	if ( openedPos == closedPos )
	{
		openedPos = closedPos + Vector( 0, 0, 80 );
		engine->Print( "WARNING: door at %3.2f %3.2f %3.2f has the same open and closed position, check its 'axis' and 'distance'!\n" );
	}

	Enable();
	SetThink( nullptr );

	if ( spawnFlags & SF_StartOpen )
	{
		// Open the area portal!
		gameImports->AdjustAreaPortalState( this, true );
		// Get out of my way!
		SetOrigin( openedPos );
		// OPEN SESAMEEEEEEEEEE!!!!!!!
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
	if ( spawnFlags & SF_NoUse )
		return;

	UpdateDoorState();
	Disable();
}

void FuncDoor::DoorTouch( IEntity* other, trace_t* trace )
{
	if ( spawnFlags & SF_NoTouch )
		return;

	UpdateDoorState();
	Disable();
}

void FuncDoor::OnClose()
{
	float requiredTime = distance / speed;

	trajectory_t* tr = &GetState()->pos;
	openedPos.CopyToArray( tr->trBase );
	(distance*axis*-1.333f).CopyToArray( tr->trDelta );
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

	// Close any areaportals if assigned to
	gameImports->AdjustAreaPortalState( this, false );

	closedPos.CopyToArray( GetState()->pos.trBase );
	GetState()->pos.trType = TR_INTERPOLATE;

	doorState = Door_Closed;
}

void FuncDoor::OnOpen()
{
	float requiredTime = distance / speed;

	// Open any areaportals if assigned to
	gameImports->AdjustAreaPortalState( this, true );

	trajectory_t* tr = &GetState()->pos;
	closedPos.CopyToArray( tr->trBase );
	(distance*axis*1.333f).CopyToArray( tr->trDelta );
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

	openedPos.CopyToArray( GetState()->pos.trBase );
	GetState()->pos.trType = TR_INTERPOLATE;

	doorState = Door_Opened;
}

void FuncDoor::UpdateDoorState()
{
	switch ( doorState )
	{
	case Door_Closed: OnOpen(); break; // When the door is closed, the player opens it
	case Door_Opened: OnClose(); break; // When the door is opened, the player closes it
	case Door_Closing: OnCloseFinished(); break; // When the door is closing, it becomes closed
	case Door_Opening: OnOpenFinished(); break; // When the door is opening, it becomes open
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
