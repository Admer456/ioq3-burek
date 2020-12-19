#include "cg_local.hpp"
#include "RenderEntity.hpp"
#include <string>

Animation::Animation()
{
	memset( name, 0, sizeof( name ) );
}

RenderEntity::RenderEntity( const char* modelName )
{
	memset( &ref, 0, sizeof( ref ) );

	std::string modelPath = modelName;
	modelPath.substr( 0, modelPath.size() - 5 ); // strip the extension
	modelPath += ".mcfg"; // MCFG = Model ConFiGuration file

	LoadModelConfig( modelPath.c_str() );
}

void RenderEntity::Update( float deltaTime )
{
	// manual = frames are set manually elsewhere
	if ( !isManual )
	{
		float framerate = 1.0f / anims[currentAnim].frameLerp;
		currentFrame += deltaTime * framerate;
	}

	ref.oldframe = currentFrame + anims[currentAnim].firstFrame;
	ref.frame = ref.oldframe + 1;
	ref.backlerp = 1.0f - (currentFrame - (int)currentFrame);

	origin.CopyToArray( ref.origin );
}

void RenderEntity::StartAnimation( animHandle anim, bool fromStart )
{
	currentAnim = anim;

	if ( fromStart )
		Reset();
}

void RenderEntity::Reset()
{
	currentFrame = 0.0f;
	Update( 0.0f );
}

void RenderEntity::SetAnimationMode( bool manual )
{
	isManual = manual;
}

void RenderEntity::SetCurrentFrame( float frame )
{
	currentFrame = frame;
	Update( 0.0f );
}

animHandle RenderEntity::GetCurrentAnimation()
{
	return currentAnim;
}

animHandle RenderEntity::GetAnimByName( const char* animName )
{
	for ( Animation& anim : anims )
	{
		if ( !anim.active || !anim.name[0] )
			continue;

		if ( !strcmp( animName, anim.name ) )
			return &anim - anims;
	}

	return AnimHandleNotFound;
}

refEntity_t& RenderEntity::GetRefEntity()
{
	return ref;
}

void RenderEntity::LoadModelConfig( const char* modelConfigPath )
{
	fileHandle_t file;
	char contents[2048];
	char* contents_p;
	int numAnims = 0;

	int len = trap_FS_FOpenFile( modelConfigPath, &file, fsMode_t::FS_READ );

	if ( !len )
	{
		CG_Error( "Cannot find model config file\n'%s'", modelConfigPath );
		return;
	}

	trap_FS_Read( contents, len, file );
	contents[len] = '\0';
	trap_FS_FCloseFile( file );

	contents_p = contents;

	char* token{ nullptr };
	while ( true )
	{
		token = COM_Parse( &contents_p );

		if ( !token[0] )
			break;

		// Begin parsing an animation def
		if ( !stricmp( token, "AnimationDef" ) )
		{
			// Get the name
			token = COM_Parse( &contents_p );
			strcpy( anims[numAnims].name, token );

			// Eat the {
			token = COM_Parse( &contents_p );
			if ( strcmp( token, "{" ) )
			{
				CG_Error( "%s: expected '{' at line %i, got %s", modelConfigPath, COM_GetCurrentParseLine(), token );
			}

			while ( true )
			{
				token = COM_Parse( &contents_p );

				if ( !stricmp( token, "firstFrame" ) )
				{
					token = COM_Parse( &contents_p );
					anims[numAnims].firstFrame = atoi( token );
				}
				else if ( !stricmp( token, "numFrames" ) )
				{
					token = COM_Parse( &contents_p );
					anims[numAnims].numFrames = atoi( token );
				}
				else if ( !stricmp( token, "loopFrames" ) )
				{
					token = COM_Parse( &contents_p );
					anims[numAnims].loopFrames = atoi( token );
				}
				else if ( !stricmp( token, "frameLerp" ) )
				{
					token = COM_Parse( &contents_p );
					anims[numAnims].frameLerp = atoi( token );
				}
				else if ( !stricmp( token, "reversed" ) )
				{
					token = COM_Parse( &contents_p );
					anims[numAnims].reversed = atoi( token ) != 0;
				}
				else if ( !strcmp( token, "}" ) )
				{
					break;
				}
				else if ( !token[0] )
				{
					CG_Error( "%s: reached EOF wtf man", modelConfigPath );
				}
				else
				{
					CG_Error( "%s: unknown property %s", modelConfigPath, token );
				}
			}

			anims[numAnims].active = true;
			numAnims++;

			if ( numAnims >= Animation::MaxAnimations )
				break;
		}


	}
}
