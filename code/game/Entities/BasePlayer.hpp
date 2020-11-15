#pragma once

namespace Entities
{
	class IEntity;
	class BaseQuakeEntity;
	class BasePlayer : public BaseQuakeEntity
	{
	public:
		void			Spawn() override;

	public: // "Properties"
		gclient_t*		GetClient();
		void			SetClient( const gclient_t* playerClient );

		const Vector&	GetClientViewAngle() const;
		void			SetClientViewAngle( const Vector& newAngle );

	public: // Misc
		void			ClientCommand();

		// Aka spawn a dead body
		void			CopyToBodyQue();
		// Add an event to be played back by the client
		void			AddEvent( int event, int eventParameter );
		// Forcibly fire a weapon
		void			FireWeapon();
		void			SetTeam( const char* teamName );
		void			StopFollowing();
		void			FollowCycle( int dir );

	protected: // Client command
		void Command_Say( int mode, bool arg0 );
		void Command_Tell();
		void Command_Score();
		void Command_Give();
		void Command_God();
		void Command_Notarget();
		void Command_Noclip();
		void Command_Kill();
		void Command_Team();
		void Command_Where();
		void Command_TeamTask();
		void Command_LevelShot();
		void Command_Follow();
		void Command_CallVote();
		void Command_Vote();
		void Command_CallTeamVote();
		void Command_TeamVote();
		void Command_GameCommand();
		void Command_SetViewpos();
		void Command_Stats();

	protected:
		gclient_s*		client{ nullptr };
	};
}
