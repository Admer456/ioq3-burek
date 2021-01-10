#include "Maths/Vector.hpp"

#include "Game/g_local.hpp"
#include "Entities/BaseEntity.hpp"
#include "Entities/KeyValueElement.hpp"
#include "Game/GameWorld.hpp"
#include "../qcommon/IEngineExports.h"
#include "Game/IGameImports.h"

#include "FuncToggle.hpp"

using namespace Entities;

DefineEntityClass( "func_toggle", FuncToggle, BaseEntity );

void FuncToggle::Spawn()
{
	BaseEntity::Spawn();

	SetUse( &FuncToggle::ToggleUse );
	originalModelIndex = GetState()->modelindex;

	if ( spawnFlags & SF_StartOff )
	{
		visible = true;
		ToggleUse( nullptr, nullptr, 0 );
	}
}

void FuncToggle::ToggleUse( IEntity* activator, IEntity* caller, float value )
{
	if ( visible )
	{
		GetState()->modelindex = 0;
		GetState()->solid = 0;
		gameImports->UnlinkEntity( this );
	}
	else
	{
		GetState()->modelindex = originalModelIndex;
		GetState()->solid |= SOLID_BMODEL;
		gameImports->LinkEntity( this );
	}

	visible = !visible;
}
