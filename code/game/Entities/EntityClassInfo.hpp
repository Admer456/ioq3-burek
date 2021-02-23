#pragma once

#include <string>

namespace Entities
{
	class IEntity;
}

class StaticCounter
{
public:
	StaticCounter()
	{
		localID = GlobalID;
		GlobalID++;
	}

	static size_t GlobalID;

	size_t GetID() const
	{
		return localID;
	}

private:
	size_t localID; 
};

using EntityAllocatorFn = Entities::IEntity*();

class EntityClassInfo
{
public:
	EntityClassInfo( const char* mapClassName, const char* entClassName, const char* superClassName, EntityAllocatorFn entityAllocator )
		: mapClass( mapClassName ), className( entClassName ), superName( superClassName )
	{
		AllocateInstance = entityAllocator;
		prev = head;
		head = this;

		// Doesn't actually quite work here, so I wrote SetupSuperClasses
		super = GetInfoByName( superClassName );
	}

	// This will be used to allocate instances of each entity class
	// In theory, multiple map classnames can allocate one C++ class
	Entities::IEntity* (*AllocateInstance)();

	// Is this entity of this specific class?
	bool IsClass( const EntityClassInfo& eci ) const
	{
		return classInfoID.GetID() == eci.classInfoID.GetID();
	}

	// Is this entity a subclass of this class?
	bool IsSubclassOf( const EntityClassInfo& eci ) const
	{
		if ( nullptr == super )
			return false;

		if ( classInfoID.GetID() == eci.classInfoID.GetID() )
			return true;

		return super->IsSubclassOf( eci );
	}

	// Get classinfo by map classname
	static EntityClassInfo* GetInfoByMapName( const char* name )
	{ 
		EntityClassInfo* current = nullptr;
		current = head;

		while ( current )
		{
			if ( !strcmp( current->mapClass, name ) )
				return current;

			current = current->prev;
		}

		return nullptr;
	}

	// Get classinfo by name
	static EntityClassInfo* GetInfoByName( const char* name )
	{
		EntityClassInfo* current = nullptr;
		current = head;

		while ( current )
		{
			if ( !strcmp( current->className, name ) )
				return current;

			current = current->prev;
		}

		return nullptr;
	}

	// This is called during game initialisation to properly set all superclasses
	static void SetupSuperClasses()
	{
		EntityClassInfo* current = nullptr;
		current = head;

		while ( current )
		{
			current->super = GetInfoByName( current->superName );

			current = current->prev;
		}
	}

	EntityClassInfo* prev;
	static EntityClassInfo* head;

	StaticCounter classInfoID;
	EntityClassInfo* super;

	const char* mapClass;
	const char* className;
	const char* superName;
};

#define DeclareEntityClass()					\
static Entities::IEntity* AllocateInstance();	\
virtual EntityClassInfo* GetClassInfo()			\
{												\
	return &ClassInfo;							\
}												\
static EntityClassInfo ClassInfo;

#define DefineEntityClass_NoMapSpawn( className, superName ) \
Entities::IEntity* className::AllocateInstance()	\
{													\
	return gameWorld->CreateEntity<className>();	\
}													\
EntityClassInfo className::ClassInfo = EntityClassInfo( #className "_dyn", #className, #superName, nullptr );


#define DefineEntityClass( mapClass, className, superName ) \
Entities::IEntity* className::AllocateInstance()			\
{															\
	return gameWorld->CreateEntity<className>();			\
}															\
EntityClassInfo className::ClassInfo = EntityClassInfo( mapClass, #className, #superName, &className::AllocateInstance );

#define DefineAbstractEntityClass( className, superName ) \
Entities::IEntity* className::AllocateInstance()			\
{															\
	return nullptr;											\
}															\
EntityClassInfo className::ClassInfo = EntityClassInfo( #className "_abs", #className, #superName, nullptr );
