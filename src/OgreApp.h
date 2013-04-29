#ifndef OGRE_APP_H
#define OGRE_APP_H

#include "PhysXSystem.h"

#include "PhysXCapsule.h"
#include "PhysXCloth.h"
#include "PhysXFluid.h"
#include "PhysXSoftBody.h"
#include "PhysXTriangleMesh.h"

using namespace OIS;
using namespace Ogre;
using namespace OgreBites;

class OgreApp : public BaseApplication
{
public:
	OgreApp();
	virtual ~OgreApp();

protected:
	virtual bool configure();
	virtual void createCamera();
	virtual void createFrameListener();
	virtual void createScene();
	virtual void destroyScene();
	virtual bool frameStarted(const FrameEvent& evt);
	virtual bool keyPressed(const KeyEvent &arg);
	virtual bool mouseMoved(const MouseEvent &arg);
	virtual bool mousePressed(const MouseEvent &arg, MouseButtonID id);
	virtual bool mouseReleased(const MouseEvent &arg, MouseButtonID id);

	virtual void resetCamPos();

	ParamsPanel* mStatesPanel;

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

	ObjMeshExt* mTerrianObj;
	ObjMeshExt* mTreeObj;
	ObjMeshExt* mLeafObj;

	bool mLastGPUState;
	bool mWind;

	NxVec3 mMouseDistance;
	NxVec3 mWindVector;

	static const int mNumLeaves = 50;
};

#endif
