#pragma once


class ICollisionModel
{
public:
	ICollisionModel() = default;
	virtual ~ICollisionModel() = default;

	virtual void			LoadMap( const char* mapName ) = 0;
	virtual int				NumInlineModels( void ) = 0;
	virtual clipHandle_t	InlineModel( int index ) = 0;
	virtual clipHandle_t	TempBoxModel( const vec3_t mins, const vec3_t maxs ) = 0;
	virtual clipHandle_t	TempCapsuleModel( const vec3_t mins, const vec3_t maxs ) = 0;
	virtual int				PointContents( const vec3_t p, clipHandle_t model ) = 0;
	virtual int				TransformedPointContents( const vec3_t p, clipHandle_t model, const vec3_t origin, const vec3_t angles ) = 0;
	virtual void			BoxTrace( trace_t* results, const vec3_t start, const vec3_t end, const vec3_t mins, const vec3_t maxs, clipHandle_t model, int brushmask ) = 0;
	virtual void			CapsuleTrace( trace_t* results, const vec3_t start, const vec3_t end, const vec3_t mins, const vec3_t maxs, clipHandle_t model, int brushmask ) = 0;
	virtual void			TransformedBoxTrace( trace_t* results, const vec3_t start, const vec3_t end, const vec3_t mins, const vec3_t maxs, clipHandle_t model, int brushmask, const vec3_t origin, const vec3_t angles ) = 0;
	virtual void			TransformedCapsuleTrace( trace_t* results, const vec3_t start, const vec3_t end, const vec3_t mins, const vec3_t maxs, clipHandle_t model, int brushmask, const vec3_t origin, const vec3_t angles ) = 0;
	virtual int				MarkFragments( int numPoints, const vec3_t* points, const vec3_t projection, int maxPoints, vec3_t pointBuffer, int maxFragments, markFragment_t* fragmentBuffer ) = 0;
};

class IClientSound
{
public:
	IClientSound() = default;
	virtual ~IClientSound() = default;

	virtual void			StartSound( vec3_t origin, int entityNum, int entchannel, sfxHandle_t sfx ) = 0;
	virtual void			StartLocalSound( sfxHandle_t sfx, int channelNum ) = 0;
	virtual void			ClearLoopingSounds( bool killall ) = 0;
	virtual void			AddLoopingSound( int entityNum, const vec3_t origin, const vec3_t velocity, sfxHandle_t sfx ) = 0;
	virtual void			AddRealLoopingSound( int entityNum, const vec3_t origin, const vec3_t velocity, sfxHandle_t sfx ) = 0;
	virtual void			StopLoopingSound( int entityNum ) = 0;
	virtual void			UpdateEntityPosition( int entityNum, const vec3_t origin ) = 0;
	virtual void			Respatialize( int entityNum, const vec3_t origin, vec3_t axis[3], int inwater ) = 0;
	virtual sfxHandle_t		RegisterSound( const char* sample, bool compressed ) = 0;
	virtual void			StartBackgroundTrack( const char* intro, const char* loop ) = 0;
	virtual void			StopBackgroundTrack( void ) = 0;

};
class IClientRenderer
{
public:
	IClientRenderer() = default;
	virtual ~IClientRenderer() = default;

	virtual void			LoadWorldMap( const char* mapname ) = 0;
	virtual qhandle_t		RegisterModel( const char* name ) = 0;
	virtual qhandle_t		RegisterSkin( const char* name ) = 0;
	virtual qhandle_t		RegisterShader( const char* name ) = 0;
	virtual qhandle_t		RegisterShaderNoMip( const char* name ) = 0;
	virtual void			RegisterFont( const char* fontName, int pointSize, fontInfo_t* font ) = 0;
	virtual void			ClearScene( void ) = 0;
	virtual void			AddRefEntityToScene( const refEntity_t* re ) = 0;
	virtual void			AddPolyToScene( qhandle_t hShader, int numVerts, const polyVert_t* verts ) = 0;
	virtual void			AddPolysToScene( qhandle_t hShader, int numVerts, const polyVert_t* verts, int num ) = 0;
	virtual int				LightForPoint( vec3_t point, vec3_t ambientLight, vec3_t directedLight, vec3_t lightDir ) = 0;
	virtual void			AddLightToScene( const vec3_t org, float intensity, float r, float g, float b ) = 0;
	virtual void			AddAdditiveLightToScene( const vec3_t org, float intensity, float r, float g, float b ) = 0;
	virtual void			RenderScene( const refdef_t* fd ) = 0;
	virtual void			SetColor( const float* rgba ) = 0;
	virtual void			DrawStretchPic( float x, float y, float w, float h, float s1, float t1, float s2, float t2, qhandle_t hShader ) = 0;
	virtual void			ModelBounds( clipHandle_t model, vec3_t mins, vec3_t maxs ) = 0;
	virtual int				LerpTag( orientation_t* tag, clipHandle_t mod, int startFrame, int endFrame, float frac, const char* tagName ) = 0;
	virtual void			RemapShader( const char* oldShader, const char* newShader, const char* timeOffset ) = 0;
};

class IClientImports
{
public:
	IClientImports() = default;
	virtual ~IClientImports() = default;

	virtual void			UpdateScreen() = 0;

	virtual void			GetGlconfig( glconfig_t* glconfig ) = 0;
	virtual void			GetGameState( gameState_t* gamestate ) = 0;
	virtual void			GetCurrentSnapshotNumber( int* snapshotNumber, int* serverTime ) = 0;
	virtual bool			GetSnapshot( int snapshotNumber, snapshot_t* snapshot ) = 0;
	virtual bool			GetServerCommand( int serverCommandNumber ) = 0;
	virtual int				GetCurrentCmdNumber( void ) = 0;
	virtual bool			GetUserCmd( int cmdNumber, usercmd_t* ucmd ) = 0;
	virtual void			SetUserCmdValue( int stateValue, float sensitivityScale ) = 0;
	virtual int				MemoryRemaining() = 0;

	virtual bool			Key_IsDown( int keyNum ) = 0;
	virtual int				Key_GetCatcher( void ) = 0;
	virtual void			Key_SetCatcher( int catcher ) = 0;
	virtual int				Key_GetKey( const char* binding ) = 0;

	virtual int				PC_AddGlobalDefine( char* define ) = 0;
	virtual int				PC_LoadSource( const char* fileName ) = 0;
	virtual int				PC_FreeSource( int handle ) = 0;
	virtual int				PC_ReadToken( int handle, pc_token_t* pc_token ) = 0;
	virtual int				PC_SourceFileAndLine( int handle, char* fileName, int* line ) = 0;

	virtual int				CIN_PlayCinematic( const char* arg0, int xpos, int ypos, int width, int height, int bits ) = 0;
	virtual e_status		CIN_StopCinematic( int handle ) = 0;
	virtual e_status		CIN_RunCinematic( int handle ) = 0;
	virtual void			CIN_DrawCinematic( int handle ) = 0;
	virtual void			CIN_SetExtents( int handle, int x, int y, int w, int h ) = 0;
};
