#include "Maths/Vector.hpp"

#include "Game/g_local.hpp"
#include "Entities/BaseEntity.hpp"
#include "Entities/BasePlayer.hpp"
#include "Entities/KeyValueElement.hpp"
#include "Game/GameWorld.hpp"
#include "../qcommon/IEngineExports.h"
#include "Game/IGameImports.h"

#include "../AI_Common.hpp"
#include "Node.hpp"

using namespace Entities;

DefineAbstractEntityClass( Node, BaseEntity );

//===========================
// Node::Spawn
//===========================
void Node::Spawn()
{
	gameImports->UnlinkEntity( this );
	SetModel( nullptr );
}

//===========================
// Node::Relink
//===========================
void Node::Relink()
{
	// Step 1: add the nodes from the targetN fields
	auto targetEntities = GetTargetEntities();
	
	// Step 2: add all nodes that are targeting this node
	auto targetsOf = GetAllTargetOf();
	
	// Step 3: append
	targetEntities.insert( targetEntities.end(), targetsOf.begin(), targetsOf.end() );
	
	// Step 4: link
	for ( IEntity* ent : targetEntities )
	{
		if ( ent->IsSubclassOf( Node::ClassInfo ) )
		{
			AddLink( static_cast<Node*>(ent) );
		}
		else
		{
			Util::PrintWarning( va( "%s.Relink: entity '%s' is not an AI node\n", GetName(), ent->GetName() ) );
		}
	}
}

//===========================
// Node::AddLink
//===========================
bool Node::AddLink( Node* node )
{
	if ( nullptr == node )
	{
		Util::PrintWarning( va( "%s.AddLink: node doesn't exist!\n", GetName() ) );
		return false;
	}

	for ( Node*& link : links )
	{
		if ( link == node )
		{	// Do not give warnings if people link two nodes to each other cyclically
			return true;
		}

		if ( nullptr == link )
		{
			link = node;
			return true;
		}
	}

	// Exceeded Node::MaxLinks
	Util::PrintWarning( va( "Exceeded Node::MaxLinks for node '%s'\n", GetName() ) );
	return false;
}

//===========================
// Node::IsAllowed
//===========================
bool Node::IsAllowed( AI::Species species, AI::Faction faction ) const
{
	return !(1 << species & speciesBlockMask) && !(1 << faction & factionBlockMask);
}
