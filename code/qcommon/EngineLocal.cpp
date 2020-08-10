#include "../server/server.hpp"

#include "IEngineExports.h"
#include "EngineLocal.h"

extern int Sys_Milliseconds( void );
extern int Cmd_Argc( void );
extern void Cmd_ArgvBuffer( int arg, char* buffer, int bufferLength );

void EngineLocal::Print( const char* string )
{
	Com_Printf( string );
}

void EngineLocal::Error( const char* string )
{
	Com_Error( ERR_DROP, string );
}

int EngineLocal::Milliseconds()
{
	return Sys_Milliseconds();
}

int EngineLocal::ArgC( void )
{
	return Cmd_Argc();
}

void EngineLocal::ArgV( int n, char* buffer, int bufferLength )
{
	Cmd_ArgvBuffer( n, buffer, bufferLength );
}

bool EngineLocal::GetEntityToken( char* buffer, int bufferSize )
{
	const char* s = COM_Parse( &sv.entityParsePoint );

	Q_strncpyz( buffer, s, bufferSize );

	if ( !sv.entityParsePoint && !s[0] )
		return false;
	
	return true;
}

int EngineLocal::FileSystem_OpenFile( const char* qpath, fileHandle_t* file, fsMode_t fileMode )
{
	return FS_FOpenFileByMode( qpath, file, fileMode );
}

void EngineLocal::FileSystem_Read( const void* buffer, int length, fileHandle_t file )
{
	FS_Read( const_cast<void*>(buffer), length, file );
}

void EngineLocal::FileSystem_Write( const void* buffer, int length, fileHandle_t file )
{
	FS_Write( const_cast<void*>(buffer), length, file );
}

void EngineLocal::FileSystem_CloseFile( fileHandle_t file )
{
	FS_FCloseFile( file );
}

int EngineLocal::FileSystem_GetFileList( const char* path, const char* extension, char* listBuffer, int bufferSize )
{
	return FS_GetFileList( path, extension, listBuffer, bufferSize );
}

int EngineLocal::FileSystem_Seek( fileHandle_t file, long offset, int origin )
{
	return FS_Seek( file, offset, origin );
}

void EngineLocal::SendConsoleCommand( int whenToExecute, const char* commandText )
{
	Cbuf_ExecuteText( whenToExecute, commandText );
}

int EngineLocal::RealTime( qtime_t* qtime )
{
	return Com_RealTime( qtime );
}

void EngineLocal::SnapVector( float* v )
{
	Q_SnapVector( v );
}
