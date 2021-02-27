#include "../../q_shared.hpp"

using namespace Assets;

std::vector<ModelAnimation> ModelConfigData::GetAnimations( const char* modelFile )
{
	fileHandle_t file;
	char contents[2048];
	char* contents_p;
	int numAnims = 0;
	std::vector<ModelAnimation> anims;

	int len = Util::FileOpen( modelFile, &file, fsMode_t::FS_READ );

	if ( len == -1 )
	{
		Util::PrintWarning( va( "Cannot find model config file\n'%s'", modelFile ) );
		return std::vector<ModelAnimation>();
	}

	Util::FileRead( contents, len, file );
	contents[len] = '\0';
	Util::FileClose( file );

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
			ModelAnimation ma;

			// Get the name
			token = COM_Parse( &contents_p );
			strcpy( ma.name, token );

			// Eat the {
			token = COM_Parse( &contents_p );
			if ( strcmp( token, "{" ) )
			{
				Util::PrintError( va( "%s: expected '{' at line %i, got %s", modelFile, COM_GetCurrentParseLine(), token ) );
			}

			while ( true )
			{
				token = COM_Parse( &contents_p );

				if ( !stricmp( token, "firstFrame" ) )
				{
					token = COM_Parse( &contents_p );
					ma.firstFrame = atoi( token );
				}
				else if ( !stricmp( token, "numFrames" ) )
				{
					token = COM_Parse( &contents_p );
					ma.numFrames = atoi( token );
				}
				else if ( !stricmp( token, "loopFrames" ) )
				{
					token = COM_Parse( &contents_p );
					ma.loopFrames = atoi( token );
				}
				else if ( !stricmp( token, "frameLerp" ) )
				{
					token = COM_Parse( &contents_p );
					ma.frameLerp = atof( token );
				}
				else if ( !stricmp( token, "reversed" ) )
				{
					token = COM_Parse( &contents_p );
					ma.reversed = atoi( token ) != 0;
				}
				else if ( !strcmp( token, "}" ) )
				{
					break;
				}
				else if ( !token[0] )
				{
					Util::PrintError( va( "%s: reached EOF wtf man", modelFile ) );
				}
				else
				{
					Util::PrintError( va( "%s: unknown property %s", modelFile, token ) );
				}
			}

			anims.push_back( ma );
			numAnims++;
		}
	}

	return anims;
}

int ModelConfigData::GetNumAnimations( const char* modelFile )
{
	return GetAnimations( modelFile ).size();
}
