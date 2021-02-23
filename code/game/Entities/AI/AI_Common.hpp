#pragma once

namespace AI
{
	enum Species : uint8_t
	{
		Species_Object = 0,
		Species_Human,
		Species_Alien,
		Species_Animal,
		Species_Plant,

		Species_MAX
	};

	enum Faction : uint8_t
	{
		Faction_None = 0, // pedestrians, shopkeepers etc.
		Faction_Mafia,
		Faction_Criminals,
		Faction_Police,
		Faction_Agency,
		Faction_Aliens, // Aliens have their own faction, but individual aliens can belong to others too

		Faction_Rogue, // If I see anyone from my faction, they're my enemies too

		Faction_MAX
	};

	enum class Relationship
	{
		None,
		BestAlly,
		Ally,
		Neutral,
		Enemy,
		WorstEnemy
	};

	enum class DynamismState
	{
		// The NPC is free
		FreeWill,
		// The NPC is performing an action
		Performing,
		// The NPC is following a scripted sequence
		FollowingScript,
		// The NPC isn't doing anything
		Frozen
	};

	enum class MovementTaskState
	{
		// Standing still, doing nothing
		StandingStill,
		// Following targetEntity
		FollowingTarget,
		// Getting away from targetEntity
		RunningAwayFromTarget,
		// Moving to random nodes
		Roaming,
		// Roaming but running
		RunningBlindly
	};

	enum class MovementState
	{
		Idling,
		MovingNormally,
		Jumping
	};

	// Each NPC has a usercmd_t, and inside of that are forwardmove, rightmove etc.
	// These are used to snap forward/rightmove variables
	enum MovementSpeeds
	{
		Idle = 0,
		Walking = 60,
		Running = 127
	};

	enum class Feeling : uint8_t
	{
		Indifferent,
		Angry,
		Happy,
		Scared,
	};

	enum Awareness : uint8_t
	{
		// "Casually minding my business
		// Returns to: Can't return lower
		// NPC does its activity
		None = 0,
		
		// "Must've been my imagination"
		// Returns to: None soon
		// NPC stops doing its activity for a moment
		Imagining,

		// "Hmm... maybe I should check that out"
		// Returns to: Imagining after enough time
		// NPC might start investigating
		Suspicious,
		
		// "Okay, I DEFINITELY saw something"
		// Returns to: Suspicious after a good amount of time
		// NPC starts investigating
		Certain,
		
		// "I KNOW YOU'RE THERE"
		// Returns to: Certain after a LONG time
		// NPC hunts the entity of interest til death
		MaximumCertainty,

		Awareness_MAX
	};

	// Types for ai_node* entities
	enum class NodeType : uint8_t
	{
		Generic = 0,

		// Navigation nodes for pathfinding 
 		Navigation,
		
		// Can hide here
		// (also provides cover direction)
		Cover,
		
		// Used for func_breakable
		Conditional,
		
		// Special case of Conditional for doors
		Door,
		
		// Special node type for ventilation entries & exits
		Vent,
		
		// Some cowardly NPCs can hide in corners
		Hide,
		
		// Some special NPCs can sneak
		Sneak
	};

	// Types for ai_volume* entities
	enum class VolumeType : uint8_t
	{
		// NPCs avoid the area at all costs
		// Used to prevent them from falling off cliffs
		Danger = 0,

		// Anything inside this area is definitely invisible
		// to NPCs with eyes, but not vice-versa
		// Use this when lightgrid data isn't good enough or you wish to be explicit
		Dark,

		// In the NPC's "eyes", this represents a solid, opaque wall
		// Can't see anything through it
		BlockVisibility
	};

	enum SituationType : uint8_t
	{
		ST_Casual = 0, // Doing work etc.
		ST_Combat,
		ST_Victory,
		ST_Defeat,
	};
}
