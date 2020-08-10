#pragma once

class IGame;
class IGameImports;
class IEngineExports;

// To be exported to the engine
struct GameExport_t
{
	unsigned int gameVersion;
	IGame* game;
};

// To be imported from the engine
struct GameImport_t
{
	unsigned int engineVersion;
	IGameImports* gameImports;
	IEngineExports* engineExports;
};

// Export-import function pointer type
// The engine calls GetGameAPI in sv_game.cpp, 
// passing GameImport data, and retrieving GameExport data
extern "C" typedef GameExport_t* (*GetGameAPIFn)( GameImport_t* import );
