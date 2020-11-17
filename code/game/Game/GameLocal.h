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

};
