#pragma once

namespace ClientEntities
{
	class BaseClientWeapon;
}

using ClientWeaponAllocator = ::ClientEntities::BaseClientWeapon* (*)( void );

// This is the weapon factory system for automatic weapon name lookup
// It is advised that you don't touch this unless you know what you're doing
class WeaponFactory final
{
public:
	WeaponFactory( const char* weaponName, unsigned int weaponID, ClientWeaponAllocator allocator )
		: name( weaponName ), id( weaponID )
	{
		// Check for same IDs
		WeaponFactory* same = nullptr;
		if ( IsIdTaken( this, same ) )
		{
			Com_Error( ERR_FATAL, "Weapons %s and %s have the same ID (%i)!!!", name, same->name, id );
		}

		this->allocator = allocator;

		prev = head;
		
		if ( prev )
			prev->next = this;
		
		head = this;
	}

	// Universal CreateWeapon method, called by other things
	ClientEntities::BaseClientWeapon* CreateWeapon()
	{
		return allocator();
	}

	// Linked list stuff
	static WeaponFactory* head;
	WeaponFactory* next{ nullptr };
	WeaponFactory* prev{ nullptr };

private:
	const char* name;
	unsigned int id;

	static bool IsIdTaken( WeaponFactory* wf, WeaponFactory* same = nullptr )
	{
		same = nullptr;

		WeaponFactory* next = wf->next;
		while ( next )
		{
			if ( next->id == wf->id )
			{
				same = next;
				return true;
			}

			next = next->next;
		}

		WeaponFactory* prev = wf->prev;
		while ( prev )
		{
			if ( prev->id == wf->id )
			{
				same = prev;
				return true;
			}

			prev = prev->prev;
		}

		return false;
	}

	// Each WeaponFactory instance will have a pointer to a function (99% of the time lambdas)
	// that will allocate the client weapon
	ClientWeaponAllocator allocator = nullptr;
};

// The end user will use this macro 99% of the time

// className: the C++ class e.g. Weapon_M57
// id: the weapon ID, must match the server's ID
#define DefineWeaponClass( className, id ) \
WeaponFactory Factory_##className = WeaponFactory( #className, id, []{ \
	return static_cast<BaseClientWeapon*>( new className() ); \
} );
