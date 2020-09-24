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
			TR_STATIONARY,
			TR_INTERPOLATE,				// non-parametric, but interpolate between snapshots
			TR_LINEAR,
			TR_LINEAR_STOP,
			TR_SINE,					// value = base + sin( time / duration ) * delta
			TR_GRAVITY
		};

		struct trajectoryData
		{
			trajectoryType trType;
			int			trTime;
			int			trDuration; // if non 0, trTime + trDuration = stop time
			Vector		trBase;
			Vector		trDelta; // velocity, etc.
		};

	public:
		void			Spawn() override {}
		void			Think() override {}

		int				entityIndex;
		int				entityType;
		int				entityFlags;

		int				time;
		int				time2;

		int				otherEntityIndex[2];
		int				groundEntityIndex;

		bool			linked;
		int				linkCount;
		int				serverFlags; // SVF_NOCLIENT, SVF_BROADCAST, etc.

		int				constantLight;	// r + (g<<8) + (b<<16) + (intensity<<24)
		int				loopSound;		// constantly loop this sound

		int				modelIndex;
		int				modelIndex2;
		int				clientNum;		// 0 to (MAX_CLIENTS - 1), for players and corpses
		int				frame;

		int				solid;			// for client side prediction, trap_linkentity sets this properly

		int				event;			// impulse events -- muzzle flashes, footsteps, etc
		int				eventParm;

		// some of these are for players
		int				powerups;		// bit flags
		int				weapon;			// determines weapon and flash model, etc.
		int				animations[8];	// Generic animations, up to 8 animation channels
		int				legsAnim;		// mask off ANIM_TOGGLEBIT
		int				torsoAnim;		// mask off ANIM_TOGGLEBIT

		int				generic1;

		// only send to this client when SVF_SINGLECLIENT is set	
		// if SVF_CLIENTMASK is set, use bitmask for clients to send to (maxclients must be <= 32, up to the mod to enforce this)
		int				singleClient;
	
		bool			bmodel;		// if false, assume an explicit mins / maxs bounding box
									// only set by trap_SetBrushModel
	
		Vector			mins, maxs;
		int				contents;	// CONTENTS_TRIGGER, CONTENTS_SOLID, CONTENTS_BODY, etc
									// a non-solid entity should set to 0
	
		Vector			absmin, absmax; // derived from mins/maxs and origin + rotation
		
		Vector			origin;
		Vector			origin2;

		Vector			angles;
		Vector			angles2;

		trajectoryData	trPos;
		trajectoryData	trApos;

		// when a trace call is made and passEntityNum != ENTITYNUM_NONE,
		// an ent will be excluded from testing if:
		// ent->s.number == passEntityNum	(don't interact with self)
		// ent->r.ownerNum == passEntityNum	(don't interact with your own missiles)
		// entity[ent->r.ownerNum].r.ownerNum == passEntityNum	(don't interact with other missiles from owner)
		int				ownerNumber;
	};
}
