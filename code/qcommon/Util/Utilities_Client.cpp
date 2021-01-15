#include "cg_local.hpp"

void Util::Print( const char* str )
{
	return CG_Printf( str );
}

void Util::PrintError( const char* str )
{
	return CG_Error( str );
}

void Util::PrintWarning( const char* str )
{
	CG_Printf( S_COLOR_YELLOW );
	CG_Printf( str );
	return CG_Printf( S_COLOR_WHITE );
}

void Util::PrintDev( const char* str, int level )
{
	CG_Printf( S_COLOR_YELLOW );
	CG_Printf( str );
	return CG_Printf( S_COLOR_WHITE );
}

int	Util::FileOpen( const char* path, fileHandle_t* f, fsMode_t mode )
{
	return trap_FS_FOpenFile( path, f, mode );
}

void Util::FileRead( void* buf, int len, fileHandle_t f )
{
	return trap_FS_Read( buf, len, f );
}

void Util::FileClose( fileHandle_t f )
{
	return trap_FS_FCloseFile( f );
}
