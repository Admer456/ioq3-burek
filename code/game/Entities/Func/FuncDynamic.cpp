#include "Maths/Vector.hpp"

#include "Game/g_local.hpp"
#include "Entities/BaseEntity.hpp"
#include "Entities/KeyValueElement.hpp"
#include "Game/GameWorld.hpp"
#include "../qcommon/IEngineExports.h"
#include "Game/IGameImports.h"
#include "Entities/BasePlayer.hpp"

#include "FuncDynamic.hpp"

using namespace Entities;

DefineEntityClass( "func_dynamic", FuncDynamic, BaseEntity );

void FuncDynamic::Spawn()
{
	BaseEntity::Spawn();

	const char* model = spawnArgs->GetCString( "model", nullptr );

	if ( nullptr == model )
	{
		Util::PrintWarning( va( "func_dynamic '%s' has no model!", targetName.c_str() ) );
		Remove();
		return;
	}

	if ( model[0] == '*' )
	{
		Util::PrintWarning( va( "func_dynamic doesn't support brush models ('%s')", targetName.c_str() ) );
		Remove();
		return;
	}

	std::string modelConfig = model;
	modelConfig = modelConfig.substr( 0, modelConfig.size() - 4 );
	modelConfig += ".mcfg";

	anims = Assets::ModelConfigData::GetAnimations( modelConfig.c_str() );

	actionAnimation = GetAnimByName( spawnArgs->GetCString( "animAction", "action" ) );
	idleAnimation = GetAnimByName( spawnArgs->GetCString( "animIdle", "idle" ) );

	GetState()->animation = idleAnimation;
	GetState()->animationFlags = AnimFlag_Loop;
	GetState()->framerate = 1000.0f / anims[idleAnimation].frameLerp;

	//Vector( 32, 32, 32 ).CopyToArray( GetShared()->maxs );
	//Vector( -32, -32, -32 ).CopyToArray( GetShared()->mins );
	//GetShared()->contents |= CONTENTS_SOLID;

	gameImports->LinkEntity( this );
}

void FuncDynamic::Use( IEntity* activator, IEntity* caller, float value )
{
	GetState()->animation = actionAnimation;
	GetState()->animationFlags = AnimFlag_ForceStart;
	GetState()->animationTime = level.time;
	GetState()->framerate = 1000.0f / anims[actionAnimation].frameLerp;

	nextThink = level.time * 0.001f + anims[actionAnimation].Length();
	SetThink( &FuncDynamic::AnimationThink );
}

void FuncDynamic::AnimationThink()
{
	GetState()->animation = idleAnimation;
	GetState()->animationFlags = AnimFlag_ForceStart | AnimFlag_Loop;
	GetState()->animationTime = level.time;
	GetState()->framerate = 1000.0f / anims[idleAnimation].frameLerp;

	SetThink( nullptr );
}

void FuncDynamic::StartAnimation( const char* name )
{
	actionAnimation = GetAnimByName( name );
	
	Use( this, this, 0 );
}

void FuncDynamic::IdleAnimation( const char* name )
{
	idleAnimation = GetAnimByName( name );
}