#include "client.hpp"
#include "ButtonFunction.hpp"

int BitsToIndex( int bits )
{
	for ( int i = 0; i < 32; i++ )
	{
		if ( bits & 1 << i )
			return i;
	}

	return -1;
}

void BaseButtonFunction::Register()
{
	char down[32];
	char up[32];

	strcpy( down, "+" );
	strcpy( up, "-" );

	strcat( down, name );
	strcat( up, name );

	Cmd_AddCommand( down, keyDown );
	Cmd_AddCommand( up, keyUp );
}

void BaseButtonFunction::Unregister()
{
	char down[32];
	char up[32];

	strcpy( down, "+" );
	strcpy( up, "-" );

	strcat( down, name );
	strcat( up, name );

	Cmd_RemoveCommand( down );
	Cmd_RemoveCommand( up );
}
