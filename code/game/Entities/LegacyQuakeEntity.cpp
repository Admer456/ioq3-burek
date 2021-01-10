#include "Game/g_local.hpp"
#include "BaseEntity.hpp"
#include "Entities/KeyValueElement.hpp"
#include "Game/GameWorld.hpp"
#include "Maths/Vector.hpp"

#include "Components/IComponent.hpp"
#include "Components/SharedComponent.hpp"

#include "LegacyQuakeEntity.hpp"

using namespace Entities;

#define DefineKV( variable, handler ) \
KeyValueElement( #variable, offsetof(LegacyQuakeEntity, variable), &KVHandlers::##handler ),

#define DefineKV_Explicit( variable_map, variable_code, handler ) \
KeyValueElement( #variable_map, offsetof(LegacyQuakeEntity, variable_code), &KVHandlers::##handler ),

#ifdef random
#undef random
#endif

// It ain't the prettiest right now, but it'll work until I get rid of gentity_t
KeyValueElement LegacyQuakeEntity::keyValues[] =
{	//| KV def type		| KV name	| KV code name		| Handler function	|
	DefineKV(			model,							CString )
	DefineKV(			model2,							CString )
	DefineKV(			speed,							Float )
	DefineKV(			target,							CString )
	DefineKV(			message,						CString )
	DefineKV(			team,							CString )
	DefineKV(			wait,							Float )
	DefineKV(			random,							Float )
	DefineKV(			count,							Int )
	DefineKV(			health,							Int )
	DefineKV(			targetShaderName,				CString )
	DefineKV(			targetShaderNewName,			CString )
	DefineKV_Explicit(	dmg,		damage,				Int )
	//DefineKV_Explicit(	origin,		state.origin,		Vector )
	//DefineKV_Explicit(	angles,		state.angles,		Vector )

	// Inherit keyvalues from BaseEntity
	KeyValueElement(	BaseEntity::keyValues ),
};

LegacyQuakeEntity::LegacyQuakeEntity()
{
	flags					= 0;
	model					= nullptr;
	model2					= nullptr;
	
	message					= nullptr;
	target					= nullptr;
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
	BaseEntity::Spawn();
}

void LegacyQuakeEntity::KeyValue()
{
	auto& map = spawnArgs->GetMap();

	BaseEntity::KeyValue();

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
