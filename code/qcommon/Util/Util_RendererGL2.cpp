#include "../../renderercommon/tr_common.h"

void Util::Print( const char* str )
{
	ri.Printf( PRINT_ALL, str );
}

void Util::PrintError( const char* str )
{
	ri.Error( ERR_DROP, str );
}

void Util::PrintWarning( const char* str )
{
	ri.Printf( PRINT_WARNING, str );
}

void Util::PrintDev( const char* str, int level )
{
	ri.Printf( PRINT_DEVELOPER, str );
}

int	Util::FileOpen( const char* path, fileHandle_t* f, fsMode_t mode )
{
	// The renderer does not quite do file IO the same as the others :(
	// Gotta fix this later on
	return -1;
}

void Util::FileRead( void* buf, int len, fileHandle_t f )
{
	return;
}

void Util::FileClose( fileHandle_t f )
{
	return;
}