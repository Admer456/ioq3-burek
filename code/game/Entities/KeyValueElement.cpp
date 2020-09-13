#include "Game/g_local.hpp"
#include "KeyValueElement.hpp"

// I'll temporarily put my type punning utilities here

// Used when you want to interpret bytes of one data type as another
// E.g. pun<int>( 2.0f )
template< typename returnType, typename inputType >
returnType Pun( inputType variable )
{
	returnType* rt = reinterpret_cast<returnType*>(&variable);
	return *rt;
}

// Used when you want to write to a memory location
// E.g. punWrite( someRawDataPointer, 20 );
template< typename valueType, typename destinationType >
void PunWrite( destinationType variable, valueType value )
{
	valueType* varPointer = reinterpret_cast<valueType*>(variable);
	*varPointer = value;
}

// Used when you want to read from a memory location
// E.g. PunDeref<int>( someMemoryLocation ); // will read 4 bytes starting from that location
template< typename derefType, typename pointerType >
derefType PunDeref( pointerType variable )
{
	derefType* dt = reinterpret_cast<derefType*>(variable);
	return *dt;
}

namespace KVHandlers
{
	int Int( byte* variableLocation, const char* value )
	{
		int intValue = atoi( value );

		PunWrite( variableLocation, intValue );

		return Success;
	}

	int CString( byte* variableLocation, const char* value )
	{
		char** string = Pun<char**>( variableLocation );

		if ( string[0] )
		{
			//printf( "DEBUG: string %s, length %i\n", *string, strlen( *string ) );
			delete[] *string;
		}

		char* newString = new char[strlen( value ) + 1];
		strcpy( newString, value );

		*string = newString;

		return Success;
	}

	int String( byte* variableLocation, const char* value )
	{
		std::string stringValue = value;

		// Something tells me this won't just work this way, but I haven't tested it yet, soooo
		PunWrite( variableLocation, stringValue );

		return Success;
	}

	// TODO: Implement a Vector class first, dummy
	int Vector( byte* variableLocation, const char* value )
	{
		return FunctionNotFound;
	}

	int Vector2D( byte* variableLocation, const char* value )
	{
		return FunctionNotFound;
	}

	int Bool( byte* variableLocation, const char* value )
	{
		bool boolValue = atoi( value ) != 0;

		PunWrite( variableLocation, boolValue );

		return FunctionNotFound;
	}

	int Float( byte* variableLocation, const char* value )
	{
		float floatValue = atof( value );

		PunWrite( variableLocation, floatValue );

		return FunctionNotFound;
	}
}

KeyValueElement::KeyValueElement( const char* memberVariableName, const size_t& memberOffset, KeyValueHandlerFunction function )
{
	name = memberVariableName;
	offset = memberOffset;
	handler = function;
}

KeyValueElement::KeyValueElement( KeyValueElement* elements )
{
	baseElements = elements;

	name = nullptr;
	offset = -1;
	handler = nullptr;
}

KeyValueElement::KeyValueElement()
{
	name = "End";
	offset = -1;

	handler = nullptr;
}

int KeyValueElement::ExecuteKeyValueHandler( byte* memberLocation, const char* value )
{
	if ( handler )
	{
		handled = true;
		return handler( memberLocation, value );
	}

	return KVHandlers::FunctionNotFound;
}

int KeyValueElement::KeyValue( const char* key, const char* value, Entities::IEntity* originalEntity )
{
	// First check if this is an array of elements
	if ( IsArray() )
	{
		int success = KVHandlers::NotFound;

		for ( int i = 0; true; i++ )
		{
			// See if we reached the end
			if ( baseElements[i].offset == -1 )
				break;

			// Oh boi, recursive functions
			success = baseElements[i].KeyValue( key, value, originalEntity );

			// If keyvalue has been handled, then there's no need to check the others
			if ( success != KVHandlers::NotFound )
				break;
		}

		return success;
	}

	if ( offset == -1 )
		return KVHandlers::NotFound;

	// Check if names match
	if ( strcmp( name, key ) )
		return KVHandlers::NotFound;

	// Calculate where we'll write to
	byte* classLocation = (byte*)originalEntity;
	byte* memberLocation = classLocation + offset;

	return ExecuteKeyValueHandler( memberLocation, value );
}




