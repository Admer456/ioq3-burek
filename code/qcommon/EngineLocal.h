#pragma once

class IEngineExports;

class EngineLocal final : public IEngineExports
{
public:
	EngineLocal() = default;
	virtual ~EngineLocal() = default;

	void	Print( const char* string ) override;
	void	Error( const char* string ) override;
	int		Milliseconds() override;

	int		ArgC( void ) override;
	void	ArgV( int n, char* buffer, int bufferLength ) override;

	bool	GetEntityToken( char* buffer, int bufferSize ) override;

	int		FileSystem_OpenFile( const char* qpath, fileHandle_t* file, fsMode_t fileMode ) override;
	void	FileSystem_Read( const void* buffer, int length, fileHandle_t file ) override;
	void	FileSystem_Write( const void* buffer, int length, fileHandle_t file ) override;
	void	FileSystem_CloseFile( fileHandle_t file ) override;
	int		FileSystem_GetFileList( const char* path, const char* extension, char* listBuffer, int bufferSize ) override;
	int		FileSystem_Seek( fileHandle_t file, long offset, int origin ) override;
	void	SendConsoleCommand( int whenToExecute, const char* commandText ) override;

	int		RealTime( qtime_t* qtime ) override;
	void	SnapVector( float* v ) override;
};
