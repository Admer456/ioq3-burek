#include "Maths/Vector.hpp"

#include "Game/g_local.hpp"
#include "Entities/BaseEntity.hpp"
#include "Entities/KeyValueElement.hpp"
#include "Game/GameWorld.hpp"
#include "../qcommon/IEngineExports.h"
#include "Game/IGameImports.h"
#include "Entities/Base/BaseTrigger.hpp"

#include "TriggerOnce.hpp"

using namespace Entities;

DefineEntityClass( "trigger_once", TriggerOnce, BaseTrigger );

void TriggerOnce::Spawn()
{
	BaseTrigger::Spawn();

	GetShared()->contents |= CONTENTS_TRIGGER;

	SetTouch( &TriggerOnce::OnceTouch );
	SetUse( &TriggerOnce::OnceUse );
}

void TriggerOnce::OnceTouch( IEntity* other, trace_t* trace )
{
	OnceUse( other, other, 0 );
}

void TriggerOnce::OnceUse( IEntity* activator, IEntity* caller, float value )
{
	SetTouch( nullptr );
	SetUse( nullptr );

	UseTargets( activator );

	Remove();
}
