#ifndef OBJLOADER_H
#define OBJLOADER_H

#include <Ogre.h>

class ObjLoader
{
public:
	ObjLoader(size_t estimateCount = 0, Ogre::uint32 multiple = 1);
	~ObjLoader();

	Ogre::MeshPtr loadObj(const Ogre::String& filename);

	std::vector<std::vector<unsigned int>> mIndexLinks;
	size_t                                 mEstimateCount;
	Ogre::uint32                           mMultiple;

private:
	Ogre::MeshPtr handleStream(Ogre::DataStreamPtr stream, Ogre::String& name);	
};

#endif
