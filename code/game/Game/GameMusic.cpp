#include "g_local.hpp"
#include "GameWorld.hpp"
#include "../qcommon/IEngineExports.h"
#include "Game/IGameImports.h"

#include "GameMusic.hpp"

/*
Game music will work this way:
When an NPC sees the player, we call
GameMusic::ToAction();
which will then do the following:
if ( ambient ) next = actionPrepare
if ( actionPrepare ) next = actionStart
if ( actionStart ) next = action[rand() % 3]
return

When the battle is over, we call
GameMusic::ToAmbient()
which will then do:
if ( action or actionPrepare ) next = actionEnd

Custom labels (e.g. set by the mapper) aren't
implemented yet
*/

// UGLY HAX0rr00r0r but I need -w-
#define streq( str1, str2 ) (!Q_stricmp( (str1), (str2) ))

GameMusic::Song::Song( const char* songLabel, const float& songLength )
{
	strncpy( label, songLabel, sizeof( label )-1 );
	label[31] = '\0';

	length = songLength;
}

void GameMusic::Song::Play() const
{
	gameImports->SetConfigString( CS_MUSIC, "" ); // force reset
	gameImports->SetConfigString( CS_MUSIC, label );
}

void GameMusic::Initialise( const char* musicFile )
{ 
	char* token = nullptr;
	char* cp = nullptr;
	char contents[2048];

	fileHandle_t f;
	int l = Util::FileOpen( musicFile, &f, FS_READ );

	if ( l <= 0 )
	{	// No .mus file! It's okay tho'
		return;
	}

	songs.clear();
	currentSong = NoSong;

	Util::FileRead( contents, l, f );
	contents[l] = '\0';
	Util::FileClose( f );

	cp = contents;

	bool ambientExists = false;
	while ( true )
	{
		char songLabel[32];
		float songLength;

		// Label name
		token = COM_Parse( &cp );
		if ( !token[0] )
			break; // end of file

		strcpy( songLabel, token );

		if ( streq( songLabel, "ambient" ) )
			ambientExists = true;

		// Song path, ignore
		token = COM_ParseExt( &cp, qfalse );
		if ( !token[0] )
		{
			Util::PrintError( va( "%s: missing path for label '%s'\n", musicFile, songLabel ) );
			return;
		}

		// Song length
		token = COM_ParseExt( &cp, qfalse );
		songLength = atof( token ) - 0.02f; // there's an ever so slight delay when switching music, so we gotta battle that

		songs.push_back( Song( songLabel, songLength ) );
	}

	if ( !ambientExists )
	{
		Util::PrintWarning( va( "WARNING: no ambient song in %s\n", musicFile ) );
	}

	// We do have songs now
	noSongs = false;
	SetNextLabel( "ambient", true );
}

void GameMusic::SetNextLabel( const char* label, bool forceStart )
{
	if ( noSongs )
	{
		return;
	}

	uint16_t count = 0;
	for ( const Song& s : songs )
	{
		if ( streq( label, s.label ) )
		{
			nextSong = count;
			
			if ( forceStart || currentSong == NoSong )
			{
				if ( currentSong == NoSong )
				{
					currentLabelStart = level.time * 0.001f + 5.0f;
					nextLabel = currentLabelStart;
				}
				else
				{
					s.Play();
					currentLabelStart = level.time * 0.001f;
					nextLabel = currentLabelStart + s.length;
				}
				
				currentSong = count;
			}

			return;
		}

		count++;
	}

	engine->Print( va( "Couldn't play label '%s', doesn't exist\n", label ) );
}

void GameMusic::Update( const float& time )
{
	if ( noSongs )
	{
		return;
	}

	const Song& curSong = songs.at( currentSong );
	const Song& nexSong = songs.at( nextSong );

	// If the ambient music is playing but we're in action, switch to it immediately
	if ( isAction && IsAmbientPlaying() )
	{
		return SetNextLabel( "action_prepare", true );
	}

	// If the current song has finished, figure out what to play next
	if ( nextLabel < time )
	{
		if ( isAction )
		{
			if ( streq( curSong.label, "ambient" ) ) 
				SetNextLabel( "action_prepare", true );

			else if ( streq( curSong.label, "action_prepare" ) ) 
				SetNextLabel( "action_start", true );

			else if ( streq( curSong.label, "action_start" ) ) 
				SetNextLabel( "action1", true );

			else if ( streq( curSong.label, "action1" ) ) 
				SetNextLabel( "action2", true );

			else if ( streq( curSong.label, "action2" ) ) 
				SetNextLabel( "action3", true );

			else if ( streq( curSong.label, "action3" ) ) 
				SetNextLabel( "action1", true );
		}
		else
		{
			if ( streq( curSong.label, "action_end" ) )
				SetNextLabel( "ambient", true );

			else if ( !Q_stricmpn( curSong.label, "action", 6 ) )
				SetNextLabel( "action_end", true );

			else SetNextLabel( "ambient", true ); // in case we have a custom label
		}
	}
}

bool GameMusic::IsAmbientPlaying()
{
	if ( noSongs )
	{
		return false;
	}

	const char* currentLabel = songs.at( currentSong ).label;

	// action_end can be interruptible too
	if ( streq( currentLabel, "ambient" ) || streq( currentLabel, "action_end" ) )
	{
		return true;
	}
	
	return false;
}

void GameMusic::ToAction()
{
	isAction = true;
}

void GameMusic::ToAmbient()
{
	isAction = false;
}
