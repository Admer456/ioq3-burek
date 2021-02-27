#include "Maths/Vector.hpp"

#include "Game/g_local.hpp"
#include "Entities/BaseEntity.hpp"
#include "Entities/BasePlayer.hpp"
#include "Entities/KeyValueElement.hpp"
#include "Game/GameWorld.hpp"
#include "../qcommon/IEngineExports.h"
#include "Game/IGameImports.h"

#include "../AI_Common.hpp"
#include "Entities/AI/Neander/Mercenary.hpp"
#include "ScriptedSequence.hpp"

using namespace Entities;
using namespace AI;

DefineEntityClass( "ai_scriptedsequence", ScriptedSequence, BaseEntity );

void ScriptedSequence::Spawn()
{
	BaseEntity::Spawn();
	SetUse( &ScriptedSequence::SequenceUse );
}

void ScriptedSequence::PostSpawn()
{
	targetMonster = static_cast<BaseEntity*>( GetTargetEntity() );

	if ( nullptr == targetMonster )
	{
		SetUse( nullptr );
		Remove();
	}

	if ( !targetMonster->IsSubclassOf( Mercenary::ClassInfo ) )
	{
		Util::PrintWarning( va( "%s.PostSpawn: entity '%s' is not an NPC!\n", GetName(), GetTarget() ) );
		SetUse( nullptr );
		Remove();
	}
}

void Entities::ScriptedSequence::SequenceUse( IEntity* activator, IEntity* caller, float value )
{
	Mercenary* npc = static_cast<Mercenary*>(targetMonster);

	
}
