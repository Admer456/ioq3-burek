#pragma once

namespace Entities
{
	class IEntity;
	class BaseQuakeEntity;
	class BasePlayer : public BaseQuakeEntity
	{
	public:
		void			Spawn() override;

	public: // "Properties"
		gclient_t*		GetClient();
		void			SetClient( const gclient_t* playerClient );

		const Vector&	GetClientViewAngle() const;
		void			SetClientViewAngle( const Vector& newAngle );

	protected:
		gclient_s*		client{ nullptr };
	};
}
