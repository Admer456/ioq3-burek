#include "Maths/Vector.hpp"

#include "Game/g_local.hpp"
#include "Entities/BaseEntity.hpp"
#include "Entities/KeyValueElement.hpp"
#include "Game/GameWorld.hpp"
#include "../qcommon/IEngineExports.h"
#include "Game/IGameImports.h"
#include "../BasePlayer.hpp"
#include "../Base/BaseMover.hpp"

#include "FuncDoor.hpp"
#include "FuncDoorRotating.hpp"

using namespace Entities;

DefineEntityClass( "func_door_rotating", FuncDoorRotating, BaseMover );

void FuncDoorRotating::Spawn()
{
	BaseMover::Spawn();

	GetState()->pos.trType = TR_INTERPOLATE;

	rotationSpeed = spawnArgs->GetFloat( "speed", 15.0f );
	rotationAngle = spawnArgs->GetFloat( "distance", 90.0f );
	rotationDir = spawnArgs->GetVector( "dir", Vector(0,1,0) );

	if ( spawnFlags & SF_Reverse )
		rotationDir = rotationDir * -1.0f;

	anglesStart = GetCurrentAngles();
	anglesEnd = anglesStart + rotationDir * rotationAngle;

	Enable();

	if ( spawnFlags & SF_StartOpen )
	{
		// Open the area portal
		gameImports->AdjustAreaPortalState( this, true );
		
		SetAngles( anglesEnd );
		SetCurrentAngles( anglesEnd );
		doorState = Door_Opened;
	}
}

void FuncDoorRotating::DoorThink()
{
	UpdateDoorState();
	SetThink( nullptr );
}

void FuncDoorRotating::DoorUse( IEntity* activator, IEntity* caller, float value )
{
	if ( caller->IsClass( BasePlayer::ClassInfo ) && spawnFlags & SF_TriggerOnly )
		return;
	
	UpdateDoorState();
	Disable();
}

void FuncDoorRotating::Blocked( IEntity* other )
{
	Vector curAngles = GetCurrentAngles();
	Vector targetAngles{ Vector::Zero };
	Vector deltaAngles{ Vector::Zero };

	if ( doorState == Door_Opened || doorState == Door_Closed )
		return;

	// If it WAS opening, then let's shift it in reverse
	if ( doorState == Door_Opening )
	{
		angularVelocity = rotationDir * -rotationSpeed;
		targetAngles = anglesStart;
		doorState = Door_Closing;
	}
	else //if ( doorState == Door_Closing )
	{
		angularVelocity = rotationDir * rotationSpeed;
		targetAngles = anglesEnd;
		doorState = Door_Opening;
	}

	deltaAngles = targetAngles - curAngles;

	float requiredTime = deltaAngles.y / rotationSpeed;
	nextThink = level.time * 0.001f + requiredTime;
}

void FuncDoorRotating::UpdateDoorState()
{
	switch ( doorState )
	{
	case Door_Closed: OnOpen(); break; // When the door is closed, the player opens it
	case Door_Opened: OnClose(); break; // When the door is opened, the player closes it
	case Door_Closing: OnCloseFinished(); break; // When the door is closing, it becomes closed
	case Door_Opening: OnOpenFinished(); break; // When the door is opening, it becomes open
	}
}

void FuncDoorRotating::OnClose()
{
	float requiredTime = rotationAngle / rotationSpeed;

	trajectory_t* tr = &GetState()->apos;
	anglesEnd.CopyToArray( tr->trBase );
	(rotationDir * -rotationAngle).CopyToArray( tr->trDelta );
	tr->trTime = level.time;
	tr->trDuration = requiredTime * 1000;
	tr->trType = TR_INTERPOLATE;

	angularVelocity = rotationDir * -rotationSpeed;
	nextThink = (level.time * 0.001f) + requiredTime;

	SetThink( &FuncDoorRotating::DoorThink );

	doorState = Door_Closing;
}

void FuncDoorRotating::OnCloseFinished()
{
	Enable();

	// Close any areaportals if assigned to
	gameImports->AdjustAreaPortalState( this, false );

	anglesStart.CopyToArray( GetState()->apos.trBase );
	GetState()->apos.trTime = TR_INTERPOLATE;

	angularVelocity = Vector::Zero;

	doorState = Door_Closed;
}

void FuncDoorRotating::OnOpen()
{
	float requiredTime = rotationAngle / rotationSpeed;
	
	trajectory_t* tr = &GetState()->apos;
	anglesStart.CopyToArray( tr->trBase );
	(rotationDir * rotationAngle).CopyToArray( tr->trDelta );
	tr->trTime = level.time;
	tr->trDuration = requiredTime * 1000;
	tr->trType = TR_INTERPOLATE;

	angularVelocity = rotationDir * rotationSpeed;
	nextThink = (level.time * 0.001f) + requiredTime;

	SetThink( &FuncDoorRotating::DoorThink );

	doorState = Door_Opening;
}

void FuncDoorRotating::OnOpenFinished()
{
	Enable();

	anglesEnd.CopyToArray( GetState()->apos.trBase );
	GetState()->apos.trTime = TR_INTERPOLATE;

	angularVelocity = Vector::Zero;

	doorState = Door_Opened;
}

void FuncDoorRotating::Enable()
{
	SetUse( &FuncDoorRotating::DoorUse );
}

void FuncDoorRotating::Disable()
{
	SetUse( nullptr );
}
