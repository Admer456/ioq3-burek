#pragma once

// Legacy Q3 push stuff
struct pushed_t
{
	Entities::BaseQuakeEntity* ent;
	Vector	origin;
	Vector	angles;
	float	deltayaw;
};

extern pushed_t pushed[MAX_GENTITIES];
extern pushed_t* pushed_p;

namespace Entities
{
	class IEntity;
	class BaseQuakeEntity;

	class BaseMover : public BaseQuakeEntity
	{
	public:
		DeclareEntityClass();

		// Overridden to call both MoverThink and thinkFunction
		void Think() override;

		void MoverThink();
		bool MoverPush( IEntity* pusher, Vector move, Vector amove, BaseQuakeEntity** obstacle );
	};
}
