#pragma once

namespace Entities
{
	class IEntity;
	class BaseEntity;

	class BaseAttachment : public BaseEntity
	{
	public:
		DeclareEntityClass();
		
		static void	CalculateFrame( IEntity* ent, int& startFrame, int& endFrame, float& fraction );

		void		Spawn() override;
		void		PostSpawn() override;
		void		Think() override;

		void		AttachTo( IEntity* parent, const char* targetBone );
		void		SwitchToBone( const char* targetBone );
		void		Detach();

		const char* GetBoneName() const;

	protected:
		void		UpdateAttachmentTransform();
	};
}
