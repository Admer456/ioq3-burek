#pragma once

namespace Entities
{
	class IEntity;

	// Test base entity, will move a brush up'n'down
	class BaseEntity_Test : public IEntity
	{
	public: 
		void			Spawn() override;
		void			MyThink();
	};
}
