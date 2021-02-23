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

Node* Node::GetNearest( const Vector& origin )
{
	Node* node = nullptr;
	float closest = 65536.0f;

	for ( Node*& n : GlobalNodes )
	{
		// It can happen
		if ( nullptr == n )
			continue;

		float distance = (origin - n->GetCurrentOrigin()).Length();

		// Also check for any solids in between
		trace_t tr;
		gameImports->Trace( &tr, origin, nullptr, nullptr, n->GetCurrentOrigin(), n->GetEntityIndex(), CONTENTS_SOLID );

		if ( tr.entityNum == ENTITYNUM_WORLD )
			continue;

		if ( distance < closest )
		{
			node = n;
			closest = distance;
		}
	}

	return node;
}

Node* Node::GetNearestIntersection( const Vector& origin )
{
	Node* node = nullptr;
	float closest = 65536.0f;

	for ( Node*& n : GlobalNodes )
	{
		// It can happen
		if ( nullptr == n )
			continue;

		if ( !n->IsIntersection() )
			continue;

		float distance = (origin - n->GetCurrentOrigin()).Length();

		// Also check for any solids in between
		trace_t tr;
		gameImports->Trace( &tr, origin, nullptr, nullptr, n->GetCurrentOrigin(), n->GetEntityIndex(), CONTENTS_SOLID );

		if ( tr.entityNum == ENTITYNUM_WORLD )
			continue;

		if ( distance < closest )
		{
			node = n;
			closest = distance;
		}
	}

	return node;
}

std::vector<Node*> Node::GetInRadius( const Vector& origin, const float& radius )
{
	std::vector<Node*> nodes;

	for ( Node* n : GlobalNodes )
	{
		if ( nullptr == n )
			continue;

		float distance = (origin - n->GetCurrentOrigin()).Length();
		if ( distance < radius )
			nodes.push_back( n );
	}

	return nodes;
}

std::vector<Node*> Node::GlobalNodes;

//===========================
// Node::Spawn
//===========================
void Node::Spawn()
{
	BaseEntity::Spawn();
	GlobalNodes.push_back( this );
}

//===========================
// Node::PostSpawn
//===========================
void Node::PostSpawn()
{
	Relink();
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
	for ( IEntity*& ent : targetEntities )
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
// Node::GetNumLinks
//===========================
uint8_t Node::GetNumLinks() const
{
	uint8_t num = 0U;
	for ( const Node* n : links )
	{
		if ( nullptr == n )
			continue;

		num++;
	}

	return num;
}

//===========================
// Node::IsAllowed
//===========================
bool Node::IsAllowed( AI::Species species, AI::Faction faction ) const
{
	return !(1 << species & speciesBlockMask) && !(1 << faction & factionBlockMask);
}

//===========================
// Node::Next
//===========================
Node* Node::Next( Node* cameFrom, Node* destination )
{
	if ( this == destination )
		return this;

	// Step 1: try finding the destination directly until an intersection
	Node* tryUntilIntersection = TryNextUntilIntersection( destination );
	if ( nullptr != tryUntilIntersection )
		return tryUntilIntersection;

	// Step 2: collect nearby intersections
	std::vector<Node*> intersections;
	for ( Node* n : links )
	{
		if ( nullptr == n )
			continue;

		Node* hit = n->HitUntilIntersection( this, destination );
		if ( nullptr == hit ) // dead end
			continue;

		intersections.push_back( n );
	}

	// In case we don't find any intersections, oh well, the NPC will use auto pathfinding
	if ( !intersections.size() )
		return nullptr;

	// Step 3: find the nearest intersection
	float distance = 99999.0f;
	Node* closest = nullptr;
	for ( Node* n : intersections )
	{
		float dist = (n->GetCurrentOrigin() - destination->GetCurrentOrigin()).Length();
		if ( dist < distance )
		{
			closest = n;
			distance = dist;
		}
	}

	return closest;
}

Node* Node::TryNextUntilIntersection( Node* destination )
{
	for ( Node* n : links )
	{
		if ( nullptr == n )
			continue;

		Node* hit = n->HitUntilIntersection( this, destination );
		if ( hit == destination )
			return n;
	}

	return nullptr;
}

Node* Node::HitUntilIntersection( Node* cameFrom, Node* destination )
{
	for ( Node* n : links )
	{
		if ( nullptr == n )
			continue;

		if ( n == cameFrom )
			continue;

		if ( n == destination )
			return destination;

		if ( n->IsIntersection() )
			return n;

		return n->HitUntilIntersection( this, destination );
	}

	// dead end
	return nullptr;
}
