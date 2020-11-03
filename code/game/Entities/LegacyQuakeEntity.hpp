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

		qboolean		neverFree;			// if true, FreeEntity will only unlink
											// bodyque uses this
		char*			model;
		char*			model2;
		int				freetime;			// level.time when the object was freed

		int				eventTime;			// events will be cleared EVENT_VALID_MSEC after set
		qboolean		freeAfterEvent;
		qboolean		unlinkAfterEvent;

		qboolean		physicsObject;		// if true, it can be pushed by movers and fall off edges
											// all game items are physicsObjects, 
		float			physicsBounce;		// 1.0 = continuous bounce, 0.0 = no bounce

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

		char*			team;
		char*			targetShaderName;
		char*			targetShaderNewName;
		IEntity*		target_ent;

		float			speed;
		Vector			movedir;

		int				pain_debounce_time;
		int				fly_sound_debounce_time;// wind tunnel
		int				last_move_time;

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

		int				noise_index;

		// timing variables
		float			wait;
		float			random;

		gitem_t*		item;				// for bonus items
	};
}
