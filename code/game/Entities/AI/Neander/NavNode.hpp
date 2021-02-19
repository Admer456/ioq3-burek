#pragma once

namespace Entities
{
	class NavNode : public Node
	{
	public:
		DeclareEntityClass();

		AI::NodeType GetNodeType() override
		{
			return AI::NodeType::Navigation;
		}
	};
}
