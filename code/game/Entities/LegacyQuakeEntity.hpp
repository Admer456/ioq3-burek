#pragma once

namespace Entities
{
	class LegacyQuakeEntity : public BaseQuakeEntity
	{
	public:
		LegacyQuakeEntity();
		~LegacyQuakeEntity();

		virtual void	Spawn() override;
		virtual void	KeyValue() override;

		static KeyValueElement keyValues[];

	public: // Fricking snitching Quake 3 gentity vars
		struct gclient_s* client;			// NULL if not a client

		qboolean		inuse;

		char*			classname;			// set in QuakeEd
		
		// BaseQuakeEntity already has spawnflags
		//int			spawnflags;			// set in QuakeEd

		qboolean		neverFree;			// if true, FreeEntity will only unlink
											// bodyque uses this

		int				flags;				// FL_* variables

		char*			model;
		char*			model2;
		int				freetime;			// level.time when the object was freed

		int				eventTime;			// events will be cleared EVENT_VALID_MSEC after set
		qboolean		freeAfterEvent;
		qboolean		unlinkAfterEvent;

		qboolean		physicsObject;		// if true, it can be pushed by movers and fall off edges
											// all game items are physicsObjects, 
		float			physicsBounce;		// 1.0 = continuous bounce, 0.0 = no bounce
		int				clipmask;			// brushes with this content value will be collided against
											// when moving.  items and corpses do not collide against
											// players, for instance

		// movers
		moverState_t	moverState;
		int				soundPos1;
		int				sound1to2;
		int				sound2to1;
		int				soundPos2;
		int				soundLoop;
		IEntity*		parent;
		IEntity*		nextTrain;
		IEntity*		prevTrain;
		Vector			pos1, pos2;

		char*			message;

		int				timestamp;			// body queue sinking, etc

		char*			target;
		char*			targetname;
		char*			team;
		char*			targetShaderName;
		char*			targetShaderNewName;
		IEntity*		target_ent;

		float			speed;
		Vector			movedir;

		// These are handled by BaseQuakeEntity and IEntity now
		//int			nextthink;
		//void			(*think)(gentity_t* self);
		//void			(*reached)(gentity_t* self);	// movers call this when hitting endpoint
		//void			(*blocked)(gentity_t* self, gentity_t* other);
		//void			(*touch)(gentity_t* self, gentity_t* other, trace_t* trace);
		//void			(*use)(gentity_t* self, gentity_t* other, gentity_t* activator);
		//void			(*pain)(gentity_t* self, gentity_t* attacker, int damage);
		//void			(*die)(gentity_t* self, gentity_t* inflictor, gentity_t* attacker, int damage, int mod);

		int				pain_debounce_time;
		int				fly_sound_debounce_time;// wind tunnel
		int				last_move_time;

		int				health;

		qboolean		takedamage;

		int				damage;
		int				splashDamage;		// quad will increase this without increasing radius
		int				splashRadius;
		int				methodOfDeath;
		int				splashMethodOfDeath;

		int				count;

		IEntity*		chain;
		IEntity*		enemy;
		IEntity*		activator;
		IEntity*		teamchain;			// next entity in team
		IEntity*		teammaster;			// master of the team

		int				watertype;
		int				waterlevel;

		int				noise_index;

		// timing variables
		float			wait;
		float			random;

		gitem_t*		item;				// for bonus items
	};
}
