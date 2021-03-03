#include "Maths/Vector.hpp"

#include "Game/g_local.hpp"
#include "Entities/BaseEntity.hpp"
#include "Entities/KeyValueElement.hpp"
#include "Game/GameWorld.hpp"
#include "../qcommon/IEngineExports.h"
#include "Game/IGameImports.h"

#include "Entities/Base/BaseAttachment.hpp"
#include "FuncAttach.hpp"

using namespace Entities;

DefineEntityClass( "func_attach", FuncAttach, BaseEntity );

void FuncAttach::PostSpawn()
{
	auto ents = GetTargetEntities();
	if ( ents.size() != 2 )
	{
		Util::PrintWarning( va( "%s.PostSpawn: invalid number of targets\n", GetName() ) );
		Remove();
		return;
	}
	
	attached = ents.at( 0 );
	attachTo = ents.at( 1 );

	if ( attached->GetState()->otherEntityNum2 && attached->GetState()->otherEntityNum2 < ENTITYNUM_WORLD )
	{
		if ( nullptr != gEntities[attached->GetState()->otherEntityNum2] )
		{
			Util::PrintWarning( va( "%s.PostSpawn: target is already affected by an existing func_attach\n" ) );
			return;
		}
	}

	attached->GetState()->otherEntityNum2 = GetEntityIndex();
}

void FuncAttach::Think()
{
	if ( nullptr == attached || nullptr == attachTo )
		return;

	orientation_t tag;
	int startFrame, endFrame;
	float fraction;
	const char* boneName = spawnArgs->GetCString( "bone", "default" );

	BaseAttachment::CalculateFrame( attachTo, startFrame, endFrame, fraction );

	gameImports->LerpTag( &tag, attachTo->GetState()->modelindex, startFrame, endFrame, fraction, boneName );

	/*
	tag.axis[0] -> bone's local down
	tag.axis[1] -> bone's local forward
	tag.axis[2] -> bone's local right
	*/

	TightOrientation& to = attached->GetState()->apos.axialOrientation;
	Vector forward = tag.axis[1];
	Vector up = Vector(tag.axis[0]) * (attached->GetState()->solid == SOLID_BMODEL ? 1.0f : -1.0f);
	Vector origin = Vector( tag.origin ) + attachTo->GetCurrentOrigin();

	to.SetForward( forward );
	to.SetUp( up );

	attached->SetAngles( Vector::Zero );
	attached->SetCurrentAngles( Vector::Zero );

	attached->SetOrigin( origin );
	attached->SetCurrentOrigin( origin );
	VectorCopy( origin, attached->GetState()->pos.trBase );

	gameImports->LinkEntity( attached );
}

void FuncAttach::Use( IEntity* activator, IEntity* caller, float value )
{
	if ( attached || attachTo )
	{
		attached = attachTo = nullptr;
		return;
	}

	if ( nullptr == attached || nullptr == attachTo )
	{
		return PostSpawn();
	}
}
