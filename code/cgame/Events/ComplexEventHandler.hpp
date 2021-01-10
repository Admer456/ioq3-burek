#pragma once

#include <vector>

class EventParser;

class ComplexEventHandler final
{
public:

	// Runs through 'parsers' and if the event ID matches, it executes and returns true
	// If it cannot find any matching parser, it returns false
	bool			ParseComplexEvent( int id, centity_t* cent, Vector position );

	// A vector of various parsers, where
	// each event parser will react to its specified event ID
	static std::vector<EventParser*> parsers;
};

// Individual event parsers will inherit from this
class EventParser
{
public:
	EventParser( int ev )
	{
		id = ev;
		ComplexEventHandler::parsers.push_back( this );
	}

	// Execute the event
	virtual void	Parse( centity_t* cent, Vector position ) = 0;
	// Which event to correspond to
	int				id;
};

// Sometimes this may throw "constant CE_YourEventHere is not a type name",
// feel free to ignore it, it's just IntelliSense acting up
#define RegisterEventParser( id, className ) \
className::className( int ev ) : EventParser( ev ){} \
static className StaticParserInstance( id );
