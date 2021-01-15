#pragma once

// PER-LIBRARY utilities
// Here are some utilities that will have a separate implementation in each library
// Shared utilities that are implemented the same in every library are in SharedUtilities.hpp

namespace Util
{
	// Printing to the console
	void	Print( const char* str );
	void	PrintError( const char* str );
	void	PrintWarning( const char* str );
	void	PrintDev( const char* str, int level );

	// File IO
	int		FileOpen( const char* path, fileHandle_t* f, fsMode_t mode );
	void	FileRead( void* buf, int len, fileHandle_t f );
	void	FileClose( fileHandle_t f );
}
