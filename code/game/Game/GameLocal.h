#pragma once

class GameLocal : public IGame
{
public:
	GameLocal() = default;
	virtual ~GameLocal() = default;

	// Game interface methods
	void				Init( int levelTime, int randomSeed, int restart ) override;
	void				Shutdown( bool restart ) override;

	const char*			ClientConnect( int clientNum, bool firstTime, bool isBot ) override;
	void				ClientBegin( int clientNum ) override;
	void				ClientUserInfoChanged( int clientNum ) override;
	void				ClientDisconnect( int clientNum ) override;
	void				ClientCommand( int clientNum ) override;
	void				ClientThink( int clientNum ) override;

	void				RunFrame( int levelTime ) override;
	bool				ConsoleCommand( void ) override;

	int					BotAI_StartFrame( int time ) override;

private: // GameLocal-specific methods
	void				FindTeams();
	void				ClientCleanName( const char* in, char* out, int outSize );
	
	void				InitSessionData( gclient_t* client, char* userInfo );
	void				ReadSessionData( gclient_t* client );
	void				WriteClientSessionData( gclient_t* client );
	void				WriteSessionData();

	void				RegisterCVars();
	void				UpdateCVars();
	void				CheckCVars();

	void				BroadcastTeamChange( gclient_t* client, int oldTeam );
	void				CalculateRanks();
	void				RemoveTournamentLoser();

	void				CheckExitRules();
	void				BeginIntermission();
	void				CheckIntermissionExit();
	void				AdjustTournamentScores();
	void				UpdateTournamentInfo();
	void				SendScoreboardMessageToAll();
	void				DeathmatchScoreboardMessage( Entities::IEntity* ent );

	bool				ScoreIsTied();
	void				LogExit( const char* string );
	void				ExitLevel();

};
