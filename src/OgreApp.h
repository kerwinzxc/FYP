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
	virtual bool frameRenderingQueued(const FrameEvent& evt);

	virtual bool keyPressed(const KeyEvent &arg);
	virtual bool mouseMoved(const MouseEvent &arg);
	virtual bool mousePressed(const MouseEvent &arg, MouseButtonID id);
	virtual bool mouseReleased(const MouseEvent &arg, MouseButtonID id);

	virtual void resetCamPos();

	SceneNode* mCameraNode;
	SceneNode* mHelperNode;

	ParamsPanel* mStatesPanel;

	AnimationState* mTerrainAnimState;
	AnimationState* mTreeAnimState;
	AnimationState* mFluidAnimState;

private:
	String getFilePath(const String& filename);
	NxVec3 getLeafPosition() {return NxVec3(-90.0 + rand() % 30, 20.0 + rand() % 70, 30.0 + rand() % 50);}

	void createTerrain();
	void createTree();
	void createTreeBody();
	void createLeaves();
	void createFluid();
	void clearPhysX();

	PhysXSystem*       mPhysXSys;
	PhysXTriangleMesh* mTerrain;
	PhysXSoftBody*     mTree;
	PhysXFluid*        mFluid;

	std::vector<PhysXCapsule*> mTreeBody;
	std::vector<PhysXCloth*>   mLeaves;

	NxClothDesc mLeafDesc;

	ObjMeshExt* mTerrainObj;
	ObjMeshExt* mTreeObj;
	ObjMeshExt* mLeafObj;

	static const enum msState {CPU, GPU, OPTIMIZED};

	msState mLastState;

	NxVec3 mMouseDistance;
	NxVec3 mWindVector;

	int mNumLeaves;
};

#endif
