#pragma once

#include <string>

namespace KVHandlers
{
	constexpr int Success = 1;
	constexpr int NotFound = 0;
	constexpr int FunctionNotFound = -1;

	// int
	int Int( byte* variableLocation, const char* value );
	
	// char*
	int CString( byte* variableLocation, const char* value );
	
	// std::string
	int String( byte* variableLocation, const char* value );
	
	// Vec3
	int Vector( byte* variableLocation, const char* value );
	
	// Vec2
	int Vector2D( byte* variableLocation, const char* value );
	
	// bool
	int Bool( byte* variableLocation, const char* value );
	
	// float
	int Float( byte* variableLocation, const char* value );
}

typedef int (*KeyValueHandlerFunction) (byte* variableLocation, const char* value);

class KeyValueElement
{
public:
	// Normal keyvalue element
	KeyValueElement( const char* memberVariableName, const size_t& memberOffset, KeyValueHandlerFunction function = nullptr );
	// Special case when 'linking' to an array of keyvalue elements from a base
	KeyValueElement( KeyValueElement* elements );
	// Empty element; "End"
	KeyValueElement();

	// Executes this's keyvalue handler function
	int						ExecuteKeyValueHandler( byte* memberLocation, const char* value );
	
	// Finds a matching keyvalue among the elements, and applies it
	int						KeyValue( const char* key, const char* value, Entities::IEntity* originalEntity );

	inline const char*		GetName()	{ return name; }
	inline size_t			GetOffset() { return offset; }
	inline bool				IsArray()	{ return baseElements != nullptr; }
	inline bool				IsHandled() { return handled; }

	KeyValueElement*		baseElements{ nullptr }; // this is used to "link" to KeyValue element arrays of other classes

protected:
	const char*				name;		// name of the keyvalue
	size_t					offset;		// offset (in bytes) of a member var from its class
	KeyValueHandlerFunction handler;	// callback which will handle the keyvalue and modify a member variable of some class
	bool					handled{ false }; // is this keyvalue handled? If so, we don't need to do it again
};
