#include "Maths/Vector.hpp"

#include "Game/g_local.hpp"
#include "Entities/BaseEntity.hpp"
#include "Entities/KeyValueElement.hpp"
#include "Game/GameWorld.hpp"
#include "../qcommon/IEngineExports.h"
#include "Game/IGameImports.h"
#include "Entities/Base/BaseTrigger.hpp"

#include "TriggerMultiple.hpp"

using namespace Entities;

DefineEntityClass( "trigger_multiple", TriggerMultiple, BaseTrigger );

void TriggerMultiple::Spawn()
{
	BaseTrigger::Spawn();

	resetDelay = spawnArgs->GetFloat( "delay", 1.0f );

	SetTouch( &TriggerMultiple::MultipleTouch );
	SetUse( &TriggerMultiple::MultipleUse );
}

void TriggerMultiple::ResetThink()
{
	SetThink( nullptr );
	SetTouch( &TriggerMultiple::MultipleTouch );
	SetUse( &TriggerMultiple::MultipleUse );
}

void TriggerMultiple::MultipleTouch( IEntity* other, trace_t* trace )
{
	MultipleUse( other, other, 0 );
}

void TriggerMultiple::MultipleUse( IEntity* activator, IEntity* caller, float value )
{
	UseTargets( activator );

	// When used, disable MultipleTouch and MultipleUse
	SetTouch( nullptr );
	SetUse( nullptr );

	// The moment when MultipleTouch and MultipleUse are enabled
	nextThink = (level.time * 0.001f) + resetDelay;

	// When current time > nextThink, then enable them
	SetThink( &TriggerMultiple::ResetThink );
}
