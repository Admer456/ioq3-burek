#pragma once

#pragma once

class IEngineExports
{
public:
	IEngineExports() = default;
	virtual ~IEngineExports() = default;

	virtual void	Print( const char* string ) = 0;
	virtual void	Error( const char* string ) = 0;
	virtual int		Milliseconds() = 0;

	virtual int		ArgC( void ) = 0;
	virtual int		ArgV( int n, char* buffer, int bufferLength ) = 0;

	virtual bool	GetEntityToken( char* buffer, int bufferSize ) = 0;

	virtual int		FileSystem_OpenFile( const char* qpath, fileHandle_t* file, fsMode_t fileMode ) = 0;
	virtual void	FileSystem_Read( const void* buffer, int length, fileHandle_t file ) = 0;
	virtual void	FileSystem_Write( const void* buffer, int length, fileHandle_t file ) = 0;
	virtual void	FileSystem_CloseFile( fileHandle_t file ) = 0;
	virtual int		FileSystem_GetFileList( const char* path, const char* extension, char* listBuffer, int bufferSize ) = 0;
	virtual int		FileSystem_Seek( fileHandle_t file, long offset, int origin ) = 0;
	virtual void	SendConsoleCommand( int whenToExecute, const char* commandText ) = 0;

	virtual int		RealTime( qtime_t* qtime ) = 0;
	virtual void	SnapVector( float* v ) = 0;
};