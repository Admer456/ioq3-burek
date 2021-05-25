#pragma once

namespace Entities
{
	class IEntity;
	class BaseEntity;
	class BaseAttachment;

	enum MemoryFrameFlags
	{
		MFF_None = 0,
		MFF_Seen = 1 << 0,
		MFF_Heard = 1 << 1,
		MFF_Shot = 1 << 2,
		MFF_Killed = 1 << 3
	};

	class MemoryFrame final
	{
	public:
		void			Reset();

		Vector			lastSeen{ Vector::Zero }; // where did I last see the entity?
		Vector			lastHeard{ Vector::Zero }; // where did I last hear the entity?

		uint32_t		flags{ 0U }; // various memory flags
		float			time{ -1.0f }; // when did I get this information?
	};

	class EntityMemory final
	{
	public:
		constexpr static size_t MaxMemoryFrames = 16U;
		constexpr static float AwarenessExpiration[AI::Awareness_MAX] =
		{
			100.0f, // None
			3.0f, // Imagining
			15.0f, // Suspicious
			60.0f, // Certain
			240.0f, // MaximumCertainty
		};

		const static EntityMemory NoMemory;

		EntityMemory();
		EntityMemory( BaseEntity* ent, AI::Relationship relationship = AI::Relationship::None );

		void			AddFrame( const MemoryFrame& frame );
		void			Update( const float& time );

		Vector			LastSeen() const;
		Vector			LastHeard() const;
		float			LastMemoryTime() const;

		void			IncreaseAwareness( const float& time );
		void			DecreaseAwareness();

		bool			alive{ true };
		uint8_t			awareness{ AI::Awareness::None };
		float			lastAware{ 0.0f };
		AI::Relationship relationship;

		uint16_t		entityIndex{ ENTITYNUM_NONE };
		MemoryFrame		frames[MaxMemoryFrames];
	};

	class Node;

	class Mercenary : public BaseEntity
	{
	public:
		// Constants
		enum HeadVariant : uint8_t
		{
			Head_Normal,
			Head_Dead
		};

		enum HeadShape : uint8_t
		{
			HeadShape_Plump,
			HeadShape_Normal,
			HeadShape_Chad
		};

	public:
		DeclareEntityClass();

		void			Spawn() override;
		void			Think() override;

		void			Precache() override;

		void			TakeDamage( IEntity* attacker, IEntity* inflictor, int damageFlags, float damage ) override;
		void			Die( IEntity* killer ) override;

		BaseEntity*		TestEntityPosition() override;

		static void		ShotAlert( BaseEntity* ent, Vector origin, float radius );

	public:
		Vector			GetHeadOffset() const;

		uint8_t			GetSpecies() const { return species; }
		uint8_t			GetFaction() const { return faction; }

		void			SetWeapon( int weaponId );
		int				GetWeapon();

		int				GetWeaponAmmo( int weaponId );

		// Limits:
		// headNumber: 0 to 4
		// headState: 0 to 4
		// weaponInventory: 0 to 4
		// currentWeapon: 0 to 1
		void			EncodeRenderData();

		AI::Relationship Relationship( BaseEntity* entity ) const;
		AI::Relationship Relationship( uint8_t spec, uint8_t fac ) const;

	protected:
		// Animation stuff
		void			SetupAnimations();
		void			EvaluateAnimation();
		void			PlayAnimation( animHandle animation, int flags );

		float			animTimer{ 0.0f };
		enum
		{
			Anim_WalkForward,
			Anim_WalkForwardGun,
			Anim_WalkBackward,
			Anim_WalkBackwardGun,

			Anim_RunForward,
			Anim_RunForwardGun,
			Anim_RunBackward,
			Anim_RunBackwardGun,

			Anim_Idle,
			Anim_IdleGun,

			Anim_DeathFront,
			Anim_DeathBack,

			Anim_Max
		};

		animHandle		humanAnims[Anim_Max];
		float			deathTime{ 0.0f };

		static inline sfxHandle_t PainSounds[4]{};
		static inline int PainSoundCounter{ 0 };

		static inline sfxHandle_t NoticeSounds[3]{};
		static inline int NoticeSoundCounter{ 0 };
		
		static inline sfxHandle_t DetectSounds[3]{};
		static inline int DetectSoundCounter{ 0 };
		
		static inline sfxHandle_t IdleSounds[7]{};
		static inline int IdleSoundCounter{ 0 };

		float			idleSoundTimer{ 0.0f };
		float			painSoundTimer{ 0.0f };

		bool			didBleed{ false };

	protected:
		// Movement functions

		// Calculates a multiplier based on the input speed
		// speed can go over 127, in which case this function will return 1.0
		// smooth is not yet implemented
		float			SnapMovementSpeed( float speed, bool smooth = false );
		// Calculates forwardmove, rightmove and upmove
		usercmd_t		CalculateCmdForVelocity( Vector wishVelocity, bool canJump = false );
		// Calculates where and how to move
		void			MovementLogic();
		// Sets up some cmd and playerState vars
		void			CalculateMoveParameters();
		// Performs the movement
		void			Move();
		// Evaluates which next node to take when trying to approach the wished position
		void			UpdatePath();

		// Sight, memory and situation functions

		// Checks for visible entities of interest every 0.25 seconds
		void			SightQuery();
		// Checks whether ent is an enemy or not, or an object etc.
		void			RegisterVisibleEntity( BaseEntity* ent );
		// Checks if an entity is in memory
		bool			IsInMemory( BaseEntity* ent );
		// Gets the collection of memory frames for ent
		EntityMemory*	GetMemory( BaseEntity* ent );
		// Decision making, shall we go into combat mode, who to attack first etc.
		void			EvaluateSituation();
		
	protected:
		// Situations
		void			Situation_Casual();
		void			Situation_Combat();
		
		// Retrieves the most important enemy from memory
		BaseEntity*		GetEnemy();

		// Calculates view angles for the aim target
		// Called in Think
		void			AimAtTarget();

		bool			CanHit( Vector direction );

		void			Attack( Vector direction );
		void			AttackFists( Vector direction );
		void			AttackPistol( Vector direction );
		void			AddBulletHole( Vector planeNormal, Vector bulletDirection, Vector origin, bool glass );

		float			nextAttack{ 0.0f };

	protected:
		// timers
		float			nextSightQuery{ 0.0f };
		float			nextDecision{ 0.0f };
		float			nextIdleMove{ 0.0f };

		// friends'n'foes
		std::vector<EntityMemory> memories;
		
		Vector			viewAngles{ Vector::Zero };
		usercmd_t		cmd{ 0 };
		playerState_t	playerState{ 0 }; // for Pmove

		Vector			lookTarget{ Vector::Zero }; // what am I looking at?
		Vector			lookDirection{ Vector(1,0,0) };

		Vector			moveIdeal{ Vector::Zero }; // wish destination
		Vector			moveTarget{ Vector::Zero }; // the next closest navigation spot to moveIdeal
		Node*			currentNode{ nullptr };

		BaseEntity*		targetEntity{ nullptr }; // my target which I might follow, kill, or assist etc.
	
		// Some render data is encoded into time2
		// Head model: 2 bits
		// Head state: 2 bits
		// Weapon inventory: 2 bits
		// Currently equipped weapon: 1 bit
		uint8_t			headNumber{ HeadShape_Normal };
		uint8_t			headState{ Head_Normal };
		uint8_t			weaponInventory{};
		uint8_t			currentWeapon{ 0U };
		uint8_t			weaponAmmo{ 15U };
		uint16_t		accessory{};

		// The state which determines what the NPC is allowed to do
		AI::DynamismState dynamism{ AI::DynamismState::FreeWill };

		uint8_t			situation{ AI::ST_Casual };
		uint8_t			species{ AI::Species_Human };
		uint8_t			faction{ AI::Faction_Mafia };
	};
}