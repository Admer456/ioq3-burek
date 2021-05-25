#include "Maths/Vector.hpp"

#include "Game/g_local.hpp"
#include "Entities/BaseEntity.hpp"
#include "Entities/KeyValueElement.hpp"
#include "Game/GameWorld.hpp"
#include "../qcommon/IEngineExports.h"
#include "Game/IGameImports.h"

#include "LogicTimer.hpp"

using namespace Entities;

DefineEntityClass( "logic_timer", LogicTimer, BaseEntity );

void LogicTimer::Spawn()
{
	delayMin = spawnArgs->GetFloat( "delayMin", 1.0f );
	delayMax = spawnArgs->GetFloat( "delayMax", 0.0f );

	SetUse( &LogicTimer::TimerUse );

	if ( spawnFlags & SF_StartOn )
	{
		SetThink( &LogicTimer::TimerThink );
		nextThink = level.time * 0.001f + CalculateDelay();
		activator = this;
	}
}

void LogicTimer::TimerUse( IEntity* activator, IEntity* caller, float value )
{
	if ( nullptr == thinkFunction || spawnFlags & SF_Impulse )
	{
		SetThink( &LogicTimer::TimerThink );
		nextThink = level.time * 0.001f + CalculateDelay();
	}
	else if ( !(spawnFlags & SF_Impulse) )
	{
		SetThink( nullptr );
		nextThink = 0;
	}

	if ( spawnFlags & SF_Impulse )
	{// Prevent being used until the delay passes
		SetUse( nullptr );
	}
}

void LogicTimer::TimerThink()
{
	UseTargets( activator );

	nextThink = level.time * 0.001f + CalculateDelay();

	if ( spawnFlags & SF_Impulse )
	{
		SetThink( nullptr );
		SetUse( &LogicTimer::TimerUse );
		nextThink = 0;
	}

	Util::PrintDev( va( "LogicTimer::TimerThink() at %3.2f\n", level.time*0.001f ), 1 );
}

float LogicTimer::CalculateDelay()
{
	if ( delayMin > delayMax )
		return delayMin;

	float delta = delayMax - delayMin;
	return delayMin + crandom() * delta;
}
