#ifndef OBJLOADER_H
#define OBJLOADER_H

#include <Ogre.h>

class ObjLoader
{
public:
	ObjLoader();
	~ObjLoader();

	Ogre::MeshPtr loadObj(const Ogre::String& filename);

	std::vector<std::vector<unsigned int>> mIndexLinks;

private:
	Ogre::MeshPtr handleStream(Ogre::DataStreamPtr stream, Ogre::String& name);
};

#endif
