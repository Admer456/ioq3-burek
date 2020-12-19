#pragma once

using XCommand = void (*)( void );

class BaseButtonFunction
{
public:
	BaseButtonFunction( const char* commandName )
		: name( commandName )
	{

	}

	XCommand keyUp;
	XCommand keyDown;

	const char* GetName()
	{
		return name;
	}

	void Register();
	void Unregister();

protected:
	const char* name;
};

void IN_KeyDown( kbutton_t* button );
void IN_KeyUp( kbutton_t* button );

int BitsToIndex( int bits );

template<int buttonType, kbutton_t* targetButtonArray>
class ButtonFunction : public BaseButtonFunction
{
public:
	ButtonFunction( const char* commandName )
		: BaseButtonFunction( commandName )
	{
		keyUp = KeyUp;
		keyDown = KeyDown;
	}

	static void KeyUp()
	{
		int buttonIndex = BitsToIndex( buttonType );
		IN_KeyUp( &targetButtonArray[buttonIndex] );
	}

	static void KeyDown()
	{
		int buttonIndex = BitsToIndex( buttonType );
		IN_KeyDown( &targetButtonArray[buttonIndex] );
	}
};
