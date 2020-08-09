#pragma once

// Client game interface
// The engine will call these functions
class IClientGame
{
public:
	IClientGame() = default;
	virtual ~IClientGame() = default;

	virtual void	Init( int serverMessageNum, int serverCommandSequence, int clientNum ) = 0;
	virtual void	Shutdown() = 0;

	virtual bool	ConsoleCommand() = 0;
	virtual void	DrawActiveFrame( int serverTime, stereoFrame_t stereoView, bool demoPlayback ) = 0;

	virtual int		CrosshairPlayer() = 0;
	virtual int		LastAttacker() = 0;

	virtual void	KeyEvent( int key, bool down ) = 0;
	virtual void	MouseEvent( int x, int y ) = 0;
	virtual void	EventHandling( int type ) = 0;
};