#ifndef OGRE_APP_H
#define OGRE_APP_H

#include "PhysXSystem.h"
#include "PhysXTriangleMesh.h"
#include "PhysXSoftBody.h"

using namespace Ogre;

class OgreApp : public BaseApplication
{
public:
	OgreApp();
	virtual ~OgreApp();

protected:
	virtual bool configure();
	virtual void createScene();
	virtual bool frameStarted(const FrameEvent& evt);

private:
	String getFilePath(const String& filename);

	void createTerrian();
	void createTree();

	PhysXSystem*       mPhysXSys;
	PhysXTriangleMesh* mTerrian;
	PhysXSoftBody*     mTree;
};

#endif
