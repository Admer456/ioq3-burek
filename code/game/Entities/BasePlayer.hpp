#pragma once

namespace Entities
{
	class IEntity;
	class BaseEntity;
	class BaseWeapon;

	class BasePlayer : public BaseEntity
	{
	public:
		DeclareEntityClass();

		void			Spawn() override;

		void			TakeDamage( IEntity* inflictor, IEntity* attacker, int damageFlags, float damageDealt ) override;

	public: // "Properties"
		gclient_t*		GetClient();
		void			SetClient( const gclient_t* playerClient );

		Vector			GetClientViewAngle() const;
		void			SetClientViewAngle( const Vector& newAngle );

	public: // Weapons
		void			AddWeapon( BaseWeapon* weapon );
		BaseWeapon*		GetCurrentWeapon();
		bool			HasAnyWeapon();
		bool			HasWeapon( int weaponID );
		void			SendWeaponEvent( uint32_t weaponEvent );
		void			UpdateWeapon();
		void			ClearWeapons( bool deleteAllWeapons = false );

	protected:
		BaseWeapon*		currentWeapon{ nullptr }; // the currently selected weapon
		BaseWeapon*		weapons[MAX_WEAPONS]; // weapon inventory

	public: // Misc
		void			ClientCommand();
		// Uses the object the player is looking at
		void			PlayerUse();
		// Get the position of the player's "eyes"
		Vector			GetViewOrigin();
		// Aka spawn a dead body
		void			CopyToBodyQue();
		// Add an event to be played back by the client
		void			AddEvent( int event, int eventParameter ) override;
		// Forcibly fire a weapon
		void			FireWeapon();
		void			SetTeam( const char* teamName );
		void			StopFollowing();
		void			FollowCycle( int dir );
		// Burn from lava, drowning etc.
		void			WorldEffects();
		// Applies all the damage taken this frame
		void			ApplyDamage();
		// Teleports the player to a given place
		void			Teleport( const Vector& toOrigin, const Vector& toAngles );

		BaseEntity* TestEntityPosition() override;

	protected: // Client command, implemented in BasePlayerCommands.cpp
		void			Command_Say( int mode, bool arg0 );
		void			Command_Tell();
		void			Command_Score();
		void			Command_Give();
		void			Command_God();
		void			Command_Notarget();
		void			Command_Noclip();
		void			Command_Kill();
		void			Command_Team();
		void			Command_Where();
		void			Command_TeamTask();
		void			Command_LevelShot();
		void			Command_Follow();
		void			Command_CallVote();
		void			Command_Vote();
		void			Command_CallTeamVote();
		void			Command_TeamVote();
		void			Command_GameCommand();
		void			Command_SetViewpos();
		void			Command_Stats();

	protected:
		gclient_s*		client{ nullptr };
		float			damage{ 0 };
		int32_t			painDebounceTime{ 0 };

		bool			isUsing; // Is holding use the current frame
		bool			wasUsing; // Held use last frame?
	};
}
