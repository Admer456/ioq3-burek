#pragma once

// Legacy Q3 push stuff
struct pushed_t
{
	Entities::BaseEntity* ent;
	Vector	origin;
	Vector	angles;
	float	deltayaw;
};

extern pushed_t pushed[MAX_GENTITIES];
extern pushed_t* pushed_p;

namespace Entities
{
	class IEntity;
	class BaseEntity;

	class BaseMover : public BaseEntity
	{
	public:
		DeclareEntityClass();

		virtual void Spawn() override;

		// Overridden to call both MoverThink and thinkFunction
		void Think() override;

		void MoverThink();
		void CustomMoverThink();
		bool MoverPush( Vector move, Vector amove, BaseEntity** obstacle );
	};
}
