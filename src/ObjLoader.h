#ifndef OBJLOADER_H
#define OBJLOADER_H

#include <Ogre.h>

class ObjLoader
{
public:
	ObjLoader(const Ogre::String& path, Ogre::SceneManager* mSceneMgr);
	~ObjLoader();

	Ogre::String        name;
	Ogre::SceneManager* mSceneMgr;
	Ogre::ManualObject* obj;

private:
	void HandleStream(Ogre::FileHandleDataStream* stream);
};

#endif
