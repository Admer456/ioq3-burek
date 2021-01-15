#include "Game/g_local.hpp"
#include "../qcommon/IEngineExports.h"

void Util::Print( const char* str )
{
	return engine->Print( str );
}

void Util::PrintError( const char* str )
{
	return engine->Error( str );
}

void Util::PrintWarning( const char* str )
{
	engine->Print( S_COLOR_YELLOW );
	engine->Print( str );
	return engine->Print( S_COLOR_WHITE );
}

void Util::PrintDev( const char* str, int level )
{
	engine->Print( S_COLOR_YELLOW );
	engine->Print( str );
	return engine->Print( S_COLOR_WHITE );
}

int	Util::FileOpen( const char* path, fileHandle_t* f, fsMode_t mode )
{
	return engine->FileSystem_OpenFile( path, f, mode );
}

void Util::FileRead( void* buf, int len, fileHandle_t f )
{
	return engine->FileSystem_Read( buf, len, f );
}

void Util::FileClose( fileHandle_t f )
{
	return engine->FileSystem_CloseFile( f );
}
