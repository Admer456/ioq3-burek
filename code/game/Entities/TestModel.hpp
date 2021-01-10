#pragma once

namespace Entities
{
	class TestModel : public BaseEntity
	{
	public:
		DeclareEntityClass();
		void Spawn() override;
	};
}
