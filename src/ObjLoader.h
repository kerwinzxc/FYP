#ifndef OBJLOADER_H
#define OBJLOADER_H

#include <Ogre.h>

class ObjLoader
{
public:
	static Ogre::MeshPtr loadObj(const Ogre::String& filename);

private:
	ObjLoader();
	~ObjLoader();

	static Ogre::MeshPtr HandleStream(Ogre::DataStreamPtr stream, Ogre::String& name);
};

#endif
