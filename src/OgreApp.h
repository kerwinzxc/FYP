#ifndef OGRE_APP_H
#define OGRE_APP_H

#include "PhysXSystem.h"

#include "PhysXCapsule.h"
#include "PhysXCloth.h"
#include "PhysXFluid.h"
#include "PhysXSoftBody.h"
#include "PhysXTriangleMesh.h"

using namespace Ogre;

class OgreApp : public BaseApplication
{
public:
	OgreApp();
	virtual ~OgreApp();

protected:
	virtual bool configure();
	virtual void createCamera();
	virtual void createScene();
	virtual void destroyScene();
	virtual bool frameStarted(const FrameEvent& evt);
	virtual bool keyPressed(const OIS::KeyEvent &arg);

private:
	String getFilePath(const String& filename);

	void createTerrian();
	void createTree();
	void createTreeBody();
	void createLeaves();
	void createFluid();
	void clearPhysX();

	PhysXSystem*       mPhysXSys;
	PhysXTriangleMesh* mTerrian;
	PhysXSoftBody*     mTree;
	PhysXFluid*        mFluid;

	std::vector<PhysXCapsule*> mTreeBody;
	std::vector<PhysXCloth*>   mLeaves;

	NxClothDesc mLeafDesc;
	ObjMeshExt  mLeafObj;

	static const int mNumLeaves = 50;
};

#endif
