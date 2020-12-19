#pragma once

class Client final
{
public:
	ClientEntities::BaseClientWeapon* GetCurrentWeapon();
};

extern Client* GetClient();
