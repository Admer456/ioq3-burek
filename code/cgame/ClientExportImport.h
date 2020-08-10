#pragma once

class IClientGame;
class ICollisionModel;
class IClientSound;
class IClientRenderer;
class IClientImports;

struct ClientExport_t
{
	IClientGame* clientGame;
};

struct ClientImport_t
{
	ICollisionModel* collisionModel;
	IClientSound* sound;
	IClientRenderer* renderer;
	IClientImports* other;
};

extern "C"
{
	typedef ClientExport_t* (*GetClientAPIFn)( ClientImport_t* import );
}
