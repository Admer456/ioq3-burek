#pragma once

namespace Entities
{
	class TestModel : public BaseQuakeEntity
	{
	public:
		DeclareEntityClass();
		void Spawn() override;
	};
}
