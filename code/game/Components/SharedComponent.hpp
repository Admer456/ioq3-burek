#pragma once

namespace Components
{
	class IComponent;

	// Represents entityShared_t and entityState_t
	class SharedComponent : public IComponent
	{
	public:
		enum trajectoryType
		{
			Trajectory_Stationary,
			Trajectory_Interpolate,	// non-parametric, but interpolate between snapshots
			Trajectory_Linear,
			Trajectory_LinearStop,
			Trajectory_Sine,		// value = base + sin( time / duration ) * delta
			Trajectory_Gravity
		};

		struct trajectoryData
		{
			trajectoryType trType{ Trajectory_Stationary };
			int			trTime{ 0 };
			int			trDuration{ 0 }; // if non 0, trTime + trDuration = stop time
			Vector		trBase{ Vector::Zero };
			Vector		trDelta{ Vector::Zero }; // velocity, etc.
		};

	public:
		void			Spawn() override {}
		void			Think() override {}

		int				entityIndex{ 0 };
		int				entityType{ 0 };
		int				entityFlags{ 0 };

		int				time{ 0 };
		int				time2{ 0 };

		int				otherEntityIndex[2];
		int				groundEntityIndex{ 0 };

		bool			linked{ false };
		int				linkCount{ 0 };
		int				serverFlags{ 0 };	// SVF_NOCLIENT, SVF_BROADCAST, etc.

		int				constantLight{ 0 };	// r + (g<<8) + (b<<16) + (intensity<<24)
		int				loopSound{ 0 };		// constantly loop this sound

		int				modelIndex{ 0 };
		int				modelIndex2{ 0 };
		int				clientNum{ 0 };		// 0 to (MAX_CLIENTS - 1), for players and corpses
		int				frame{ 0 };

		int				solid{ 0 };			// for client side prediction, trap_linkentity sets this properly

		int				event{ 0 };			// impulse events -- muzzle flashes, footsteps, etc
		int				eventParm{ 0 };

		// some of these are for players
		int				powerups{ 0 };		// bit flags
		int				weapon{ 0 };		// determines weapon and flash model, etc.
		int				animations[8];		// Generic animations, up to 8 animation channels
		int				legsAnim{ 0 };		// mask off ANIM_TOGGLEBIT
		int				torsoAnim{ 0 };		// mask off ANIM_TOGGLEBIT

		int				generic1{ 0 };

		// only send to this client when SVF_SINGLECLIENT is set	
		// if SVF_CLIENTMASK is set, use bitmask for clients to send to (maxclients must be <= 32, up to the mod to enforce this)
		int				singleClient{ 0 };
	
		bool			bmodel{ false };// if false, assume an explicit mins / maxs bounding box
									// only set by trap_SetBrushModel
	
		Vector			mins, maxs;
		int				contents{ 0 };// CONTENTS_TRIGGER, CONTENTS_SOLID, CONTENTS_BODY, etc
									// a non-solid entity should set to 0
	
		Vector			absmin{ Vector::Zero };
		Vector			absmax{ Vector::Zero }; // derived from mins/maxs and origin + rotation
		
		Vector			origin{ Vector::Zero };
		Vector			origin2{ Vector::Zero };

		Vector			angles{ Vector::Zero };
		Vector			angles2{ Vector::Zero };

		trajectoryData	trPos;
		trajectoryData	trApos;

		// when a trace call is made and passEntityNum != ENTITYNUM_NONE,
		// an ent will be excluded from testing if:
		// ent->s.number == passEntityNum	(don't interact with self)
		// ent->r.ownerNum == passEntityNum	(don't interact with your own missiles)
		// entity[ent->r.ownerNum].r.ownerNum == passEntityNum	(don't interact with other missiles from owner)
		int				ownerNumber{ 0 };
	};
}
