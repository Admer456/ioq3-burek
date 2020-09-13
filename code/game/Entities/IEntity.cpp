#include "Game/g_local.hpp"
#include "Components/IComponent.hpp"
#include <type_traits>

using namespace Entities;
using namespace Components;

template<typename componentType>
componentType* IEntity::GetComponent()
{
	for ( auto component : components )
	{
		if ( c = dynamic_cast<componentType*>(component) )
		{
			return c;
		}
	}

	return nullptr;
}

template<typename componentType>
componentType* IEntity::CreateComponent()
{
	IComponent* comp = new componentType();
	
	components.push_back( comp );

	return comp;
}

template<typename componentType>
componentType* IEntity::GetOrCreateComponent()
{
	componentType* comp;
	if ( !(comp = GetComponent<componentType>()) )
	{
		comp = CreateComponent<componentType>();
	}

	return comp;
}
