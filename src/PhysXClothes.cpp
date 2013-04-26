#include "PhysXClothes.h"
#include "ObjMeshExt.h"

PhysXClothes::PhysXClothes(NxScene* scene, Ogre::SceneManager* sceneMgr, NxClothDesc &desc,
                           char* objFilePath, NxVec3 initPosition, NxU32 numOfClothes)
	: mInitDone(false), mInitPosition(initPosition), mNumOfClothes(numOfClothes),
	  mScene(scene), mDesc(desc), mSceneMgr(sceneMgr)
{
	saveMeshDesc(objFilePath);
	init();
	initOgreScene();
}

PhysXClothes::~PhysXClothes()
{
	if (mInitDone)
	{
		releaseMeshDescBuffers();
		for (size_t i = 0; i < mClothes.size(); ++i)
			mScene->releaseCloth(*(mClothes[i]));
		for (size_t i = 0; i < mClothMeshes.size(); ++i)
			mScene->getPhysicsSDK().releaseClothMesh(*(mClothMeshes[i]));
		releaseReceiveBuffers();
	}
}

bool PhysXClothes::saveMeshDesc(char* objFilePath)
{
	ObjMeshExt obj;
	obj.loadFromObjFile(objFilePath);

	int vertexCount   = obj.getNumVertices();
	if (vertexCount == 0)
		return false;

	mName = obj.getName();
	mNumTriangles = obj.getNumTriangles();

	NxVec3* verts = new NxVec3[vertexCount];
	NxU32*  faces = new NxU32[mNumTriangles * 3];

	for (int i = 0; i < vertexCount; i++)
	{
		NxVec3 vertex = obj.getVertex(i);
		verts[i].x = vertex.x;
		verts[i].y = vertex.y;
		verts[i].z = vertex.z;
	}
	for (int i = 0; i < mNumTriangles; i++)
	{
		ObjMeshTriangle tri = obj.getTriangle(i);
		faces[i * 3]     = tri.vertexNr[0];
		faces[i * 3 + 1] = tri.vertexNr[1];
		faces[i * 3 + 2] = tri.vertexNr[2];
	}

	mMeshDesc.numVertices           = vertexCount;
	mMeshDesc.numTriangles          = mNumTriangles;
	mMeshDesc.pointStrideBytes      = sizeof(NxVec3);
	mMeshDesc.triangleStrideBytes   = 3 * sizeof(NxU32);
	mMeshDesc.vertexMassStrideBytes = sizeof(NxReal);
	mMeshDesc.vertexFlagStrideBytes = sizeof(NxU32);
	mMeshDesc.points                = verts;
	mMeshDesc.triangles             = faces;
	mMeshDesc.vertexMasses          = 0;
	mMeshDesc.vertexFlags           = 0;
	mMeshDesc.flags                 = 0;
	mMeshDesc.flags                |= NX_CLOTH_MESH_WELD_VERTICES;
	mMeshDesc.weldingDistance       = 0.5f;

	return true;
}

void PhysXClothes::init()
{
	mReceiveBuffers = (NxMeshData*) malloc(sizeof(NxMeshData) * mNumOfClothes);
	mNumVertices    = (NxU32*)      malloc(sizeof(NxU32) * mNumOfClothes);
	mNumIndices     = (NxU32*)      malloc(sizeof(NxU32) * mNumOfClothes);
	mMeshDirtyFlags = (NxU32*)      malloc(sizeof(NxU32) * mNumOfClothes);

	for (NxU32 i = 0; i < mNumOfClothes; i++)
	{
		mPositions.push_back(new NxVec3[mMeshDesc.numVertices]);
		mNormals.push_back(new NxVec3[mMeshDesc.numVertices]);
		mIndices.push_back(new NxU32[mMeshDesc.numTriangles * 3]);

		mNumVertices[i]    = 0;
		mNumIndices[i]     = 0;
		mMeshDirtyFlags[i] = 0;

		createCloth(i);
	}
	mPositions.resize(mNumOfClothes);
	mNormals.resize(mNumOfClothes);
	mIndices.resize(mNumOfClothes);
}

void PhysXClothes::allocateReceiveBuffers(NxU32 numVertices, NxU32 numTriangles, NxU32 offset)
{
	mReceiveBuffers[offset].setToDefault();
	mReceiveBuffers[offset].verticesPosBegin         = &(mPositions[offset][0].x);
	mReceiveBuffers[offset].verticesPosByteStride    = sizeof(NxVec3);
	mReceiveBuffers[offset].verticesNormalBegin      = &(mNormals[offset][0].x);
	mReceiveBuffers[offset].verticesNormalByteStride = sizeof(NxVec3);
	mReceiveBuffers[offset].indicesBegin             = &(mIndices[offset][0]);
	mReceiveBuffers[offset].indicesByteStride        = sizeof(NxU32);

	mReceiveBuffers[offset].maxVertices              = numVertices;
	mReceiveBuffers[offset].numVerticesPtr           = &mNumVertices[offset];
	mReceiveBuffers[offset].maxIndices               = numTriangles * 3;
	mReceiveBuffers[offset].numIndicesPtr            = &mNumIndices[offset];
	mReceiveBuffers[offset].dirtyBufferFlagsPtr      = &mMeshDirtyFlags[offset];
}

bool PhysXClothes::cookMesh(NxU32 offset)
{
	assert(mMeshDesc.isValid());

	MemoryWriteBuffer wb;
	if (!NxCookClothMesh(mMeshDesc, wb))
		return false;
	MemoryReadBuffer rb(wb.data);
	mClothMeshes.push_back(mScene->getPhysicsSDK().createClothMesh(rb));

	return true;
}

void PhysXClothes::createCloth(NxU32 offset)
{
	allocateReceiveBuffers(mMeshDesc.numVertices, mMeshDesc.numTriangles, offset);
	cookMesh(offset);

	NxVec3 vec;
	vec.x = rand() % 30;
	vec.y = rand() % 70;
	vec.z = rand() % 50;
	mDesc.globalPose.t = mInitPosition + vec;
	mDesc.clothMesh = mClothMeshes[offset];
	mDesc.meshData  = mReceiveBuffers[offset];

	assert(mDesc.isValid());
	mClothes.push_back(mScene->createCloth(mDesc));
	mClothes[offset]->wakeUp();
}

void PhysXClothes::releaseMeshDescBuffers()
{
	NxVec3* y = (NxVec3*)mMeshDesc.points;
	NxU32*  t = (NxU32*) mMeshDesc.triangles;
	NxReal* m = (NxReal*)mMeshDesc.vertexMasses;
	NxU32*  z = (NxU32*) mMeshDesc.vertexFlags;
	free(y);
	free(t);
	free(m);
	free(z);
}

void PhysXClothes::releaseReceiveBuffers()
{
	for (NxU32 i = 0; i < mNumOfClothes; i++)
	{
		free(mReceiveBuffers[i].parentIndicesBegin);
		mReceiveBuffers[i].setToDefault();
	}
}

void PhysXClothes::initOgreScene()
{
	mManualObj = mSceneMgr->createManualObject(mName);
	mClothesNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	mClothesNode->attachObject(mManualObj);
}

void PhysXClothes::render()
{
	mManualObj->clear();
	mManualObj->begin("", Ogre::RenderOperation::OT_TRIANGLE_LIST);
	for (NxU32 i = 0; i < mNumOfClothes; i++)
	{
		NxVec3 position = mClothes[i]->getPosition(0);
		if (!(-100.0 < position.x && position.x < 90.0 &&
		         0.0 < position.y && position.y < 120.0 &&
		         0.0 < position.z && position.z < 150.0))
		{
			mScene->releaseCloth(*(mClothes[i]));
			mScene->getPhysicsSDK().releaseClothMesh(*(mClothMeshes[i]));
			createCloth(i);
		}

		for (NxU32 j = 0; j < mNumVertices[i]; j++)
		{
			Ogre::Vector3 pos = Ogre::Vector3(mPositions[i][j].x,
			                                  mPositions[i][j].y,
			                                  mPositions[i][j].z);
			mManualObj->position(pos);
			Ogre::Vector3 nor = Ogre::Vector3(mNormals[i][j].x,
			                                  mNormals[i][j].y,
			                                  mNormals[i][j].z);
			mManualObj->normal(nor);
			mManualObj->colour(Ogre::ColourValue::Green);
		}

		for(NxU32 j = 0; j < mNumTriangles; j++)
		{
			mManualObj->triangle(mIndices[i][j * 3],
			                     mIndices[i][j * 3 + 1],
			                     mIndices[i][j * 3 + 2]);
		}
	}
	mManualObj->end();
}
