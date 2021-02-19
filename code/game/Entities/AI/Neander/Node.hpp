#pragma once

namespace Entities
{
	class Node : public BaseEntity
	{
	public:
		DeclareEntityClass();

		constexpr static size_t MaxLinks = 4U;

		void		Spawn() override;

		virtual AI::NodeType GetNodeType()
		{
			return AI::NodeType::Generic;
		}

		// Establishes connections with other nodes that:
		// - are in its target field
		// - are targeting this
		void		Relink();

		// Adds 'node' to the list of links
		// Returns true if it was added, false if it
		// couldn't get added (due to hitting MaxLinks)
		bool		AddLink( Node* node );

		// Determines whether an NPC of this species and this faction may pass the node
		bool		IsAllowed( AI::Species species, AI::Faction faction ) const;

	protected:
		Node*		links[MaxLinks];

		// Species and faction "block" masks prevent NPCs
		// of certain species and/or factions from traversing this node
		// By default, they're both 0 to let all NPCs pass
		// Some nodes can only allow, for example, animals to pass
		uint32_t	speciesBlockMask{ 0 };
		uint32_t	factionBlockMask{ 0 };
	};
}
