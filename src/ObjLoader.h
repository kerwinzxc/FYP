#ifndef OBJLOADER_H
#define OBJLOADER_H

#include <Ogre.h>

class ObjLoader
{
public:
	static Ogre::MeshPtr loadObj(const Ogre::String& path);

private:
	ObjLoader();
	~ObjLoader();

	static Ogre::MeshPtr HandleStream(Ogre::FileHandleDataStream* stream, Ogre::String& name);
};

#endif
