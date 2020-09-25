#include "Maths/Vector.hpp"

#include "Game/g_local.hpp"
#include "BaseEntity.hpp"
#include "Entities/KeyValueElement.hpp"
#include "Game/GameWorld.hpp"
#include "../qcommon/IEngineExports.h"
#include "Game/IGameImports.h"
#include "Components/IComponent.hpp"
#include "Components/SharedComponent.hpp"

using namespace Entities;
using namespace Components;

KeyValueElement BaseQuakeEntity::keyValues[] =
{
	//KeyValueElement( "origin",	0,												KVHandlers::Vector ),
	//KeyValueElement( "angles",	0,												KVHandlers::Vector ),
	KeyValueElement( "spawnflags",	offsetof( BaseQuakeEntity, spawnFlags ),		KVHandlers::Int ),
	KeyValueElement( "spawnflags2", offsetof( BaseQuakeEntity, spawnFlagsExtra ),	KVHandlers::Int ),
	KeyValueElement()
};

void BaseQuakeEntity::Spawn()
{
	auto comp = CreateComponent<SharedComponent>();
	const char* model = spawnArgs->GetCString( "model", nullptr );

	comp->origin = spawnArgs->GetVector( "origin", Vector::Zero );
	comp->angles = spawnArgs->GetVector( "angles", Vector::Zero );

	if ( !model )
	{
		engine->Print( "This entity has no model\n" );
	}
	else
	{
		engine->Print( "Setting model " );
		engine->Print( model );
		engine->Print( "\n" );

		if ( model[0] == '*' )
		{
			gameImports->SetBrushModel( this, model );
		}

		else
		{
			engine->Print( "This is not a brush model, not yet supported\n" );
		}
	}
}

void BaseQuakeEntity::PreKeyValue()
{

}

void BaseQuakeEntity::PostKeyValue()
{
	
}

void BaseQuakeEntity::KeyValue()
{
	auto& map = spawnArgs->GetMap();

	PreKeyValue();

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

	PostKeyValue();
}

void BaseQuakeEntity::Think()
{
	if ( nullptr == thinkFunction )
	{
		return;
	}

	if ( nextThink > (level.time * 0.001f) )
	{
		return;
	}

	(this->*thinkFunction)();
}

void BaseEntity_Test::Spawn()
{
	SetThink( &BaseEntity_Test::MyThink );
}

void BaseEntity_Test::MyThink()
{
	static float cycle = 0.0f;
	static float originalZ = 0;
	static float iterator = 0.008f;

	cycle += iterator;
	if ( fabs( cycle ) > 1.000f )
		iterator *= -1.0f;
}
