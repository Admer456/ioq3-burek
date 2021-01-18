#include "../q_shared.hpp"
#include "../qcommon.hpp"

void Util::Print( const char* str )
{
	return Com_Printf( str );
}

void Util::PrintError( const char* str )
{
	return Com_Error( ERR_DROP, str );
}

void Util::PrintWarning( const char* str )
{
	Com_Printf( S_COLOR_YELLOW );
	Com_Printf( str );
	return Com_Printf( S_COLOR_WHITE );
}

void Util::PrintDev( const char* str, int level )
{
	if ( Cvar_VariableValue( "developer" ) < level )
		return;

	Com_Printf( S_COLOR_YELLOW );
	Com_Printf( str );
	return Com_Printf( S_COLOR_WHITE );
}

qboolean FS_SV_FileExists( const char* file );

int	Util::FileOpen( const char* path, fileHandle_t* f, fsMode_t mode )
{
	return FS_FOpenFileByMode( path, f, mode );
}

void Util::FileRead( void* buf, int len, fileHandle_t f )
{
	FS_Read( buf, len, f );
}

void Util::FileClose( fileHandle_t f )
{
	return FS_FCloseFile( f );
}
