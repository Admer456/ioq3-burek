#include "Game/g_local.hpp"
#include "BaseEntity.hpp"
#include "Entities/KeyValueElement.hpp"
#include "Game/GameWorld.hpp"

#include "../qcommon/Maths/Vector.hpp"

#include "LegacyQuakeEntity.hpp"

using namespace Entities;

#define DefineKV( variable, handler ) \
KeyValueElement( #variable, offsetof(LegacyQuakeEntity, variable), handler ),

#define DefineKV_Explicit( variable_map, variable_code, handler ) \
KeyValueElement( #variable_map, offsetof(LegacyQuakeEntity, variable_code), handler ),

#ifdef random
#undef random
#endif

// It ain't the prettiest right now, but it'll work until I get rid of gentity_t
KeyValueElement LegacyQuakeEntity::keyValues[] =
{	//| KV def type		| KV name	| KV code name		| Handler function		|
	DefineKV(			classname,						&KVHandlers::CString )
	DefineKV_Explicit(	origin,		state.origin,		&KVHandlers::Vector )
	DefineKV(			model,							&KVHandlers::CString )
	DefineKV(			model2,							&KVHandlers::CString )
	DefineKV(			speed,							&KVHandlers::Float )
	DefineKV(			target,							&KVHandlers::CString )
	DefineKV(			targetname,						&KVHandlers::CString )
	DefineKV(			message,						&KVHandlers::CString )
	DefineKV(			team,							&KVHandlers::CString )
	DefineKV(			wait,							&KVHandlers::Float )
	DefineKV(			random,							&KVHandlers::Float )
	DefineKV(			count,							&KVHandlers::Int )
	DefineKV(			health,							&KVHandlers::Int )
	DefineKV_Explicit(	dmg,		damage,				&KVHandlers::Int )
	DefineKV_Explicit(	angles,		state.angles,		&KVHandlers::Vector )
	DefineKV(			targetShaderName,				&KVHandlers::CString )
	DefineKV(			targetShaderNewName,			&KVHandlers::CString )

	// Inherit keyvalues from BaseQuakeEntity
	KeyValueElement(	BaseQuakeEntity::keyValues ),
};

LegacyQuakeEntity::LegacyQuakeEntity()
{
	inuse					= true;
	classname				= nullptr;

	flags					= 0;
	model					= nullptr;
	model2					= nullptr;
	
	message					= nullptr;
	target					= nullptr;
	targetname				= nullptr;
	team					= nullptr;
	targetShaderName		= nullptr;
	targetShaderNewName		= nullptr;
	target_ent				= nullptr;

	speed					= 0;
	movedir					= Vector::Zero;

	health					= 0;
	chain					= nullptr;
	enemy					= nullptr;
	activator				= nullptr;
	teamchain				= nullptr;
	teammaster				= nullptr;

	watertype				= 0;
	waterlevel				= 0;
	noise_index				= -1;

	wait					= 0.0f;
	random					= 0.0f;

	item					= nullptr;
}

LegacyQuakeEntity::~LegacyQuakeEntity()
{
	// Meh
}

void LegacyQuakeEntity::Spawn()
{
	BaseQuakeEntity::Spawn();
}

void LegacyQuakeEntity::KeyValue()
{
	auto& map = spawnArgs->GetMap();

	BaseQuakeEntity::KeyValue();

	// For every keyvalue string pair
	for ( auto& keyValue : map )
	{
		// Check to see if we can dispatch any of our keyvalues on it
		for ( auto& element : keyValues )
		{
			// Skip this element if it's already handled
			if ( element.IsHandled() )
				continue;

			// Try dispatching the keyvalue
			int result = element.KeyValue( keyValue.first.c_str(), keyValue.second.c_str(), this );

			// If it's a success, then stop processing this specific keyvalue
			if ( result == KVHandlers::Success )
				break;
		}
	}
}
