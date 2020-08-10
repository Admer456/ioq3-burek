#pragma once

class IGame
{
public:
	IGame() = default;
	virtual ~IGame() = default;

	virtual void Init( int levelTime, int randomSeed, int restart ) = 0;
	virtual void Shutdown( bool restart ) = 0;
	
	virtual const char* ClientConnect( int clientNum, bool firstTime, bool isBot ) = 0;
	virtual void ClientBegin( int clientNum ) = 0;
	virtual void ClientUserInfoChanged( int clientNum ) = 0;
	virtual void ClientDisconnect( int clientNum ) = 0;
	virtual void ClientCommand( int clientNum ) = 0;
	virtual void ClientThink( int clientNum ) = 0;

	virtual void RunFrame( int levelTime ) = 0;
	virtual bool ConsoleCommand( void ) = 0;

	virtual int BotAI_StartFrame( int time ) = 0;
};

extern IGame* game;
