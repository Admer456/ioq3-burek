#pragma once

#include <vector>
#include <string>
#include <unordered_map>

class Vector;

class KeyValueLibrary final
{
	using KVMap = std::unordered_map<std::string, std::string>;

public:
	KeyValueLibrary() = default;
	~KeyValueLibrary();

	void			AddKeyValue( const char* key, const char* value );
	std::string& GetValue( const char* keyName );
	KVMap& GetMap() { return library; }

	// Templ8s didn't work here for some odd reason so I'm gonna specialise
	const char* GetCString( const char* keyName, const char* defaultValue );
	float			GetFloat( const char* keyName, float defaultValue );
	int				GetInt( const char* keyName, int defaultValue );
	bool			GetBool( const char* keyName, int defaultValue );
	Vector			GetVector( const char* keyName, Vector defaultValue );

	std::string& operator [] ( const char* val )
	{
		return library[val];
	}

private:
	KVMap library;
};
