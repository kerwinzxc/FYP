#include "PhysXCloth.h"
#include "ObjMeshExt.h"

PhysXCloth::PhysXCloth(NxScene* scene, Ogre::SceneManager* sceneMgr, NxClothDesc &desc,
                       ObjMeshExt* objMesh, int index)
	: mInitDone(false), mScene(scene), mCloth(NULL), mClothMesh(NULL),
	  mSceneMgr(sceneMgr), mClothNode(NULL), mManualObj(NULL),
	  mNumVertices(0), mNumIndices(0), mMeshDirtyFlags(0)
{
	mName = objMesh->getName();
	mMaterial = Ogre::String(objMesh->getMaterial(0).name);
	char* temp;
	itoa(index, temp, 10);
	mName.append(temp);
	mNumTriangles = objMesh->getNumTriangles();

	NxClothMeshDesc meshDesc;
	saveMeshDesc(meshDesc, objMesh);
	init(desc, meshDesc);
	initOgreScene();
}

PhysXCloth::~PhysXCloth()
{
	if (mInitDone)
	{
		if (mCloth)
			mScene->releaseCloth(*mCloth);
		if (mClothMesh)
			mScene->getPhysicsSDK().releaseClothMesh(*mClothMesh);
		releaseReceiveBuffers();
	}
	mPositions.swap(std::vector<NxVec3>());
	mNormals.swap(std::vector<NxVec3>());
	mIndices.swap(std::vector<NxU32>());

	if (mClothNode)
	{
		mClothNode->removeAndDestroyAllChildren();
		mSceneMgr->destroySceneNode(mClothNode);
	}
	if (mManualObj)
		mSceneMgr->destroyManualObject(mManualObj);
}

bool PhysXCloth::saveMeshDesc(NxClothMeshDesc &desc, ObjMeshExt* objMesh)
{
	int vertexCount = objMesh->getNumVertices();
	if (vertexCount == 0)
		return false;

	NxVec3* verts = new NxVec3[vertexCount];
	NxU32*  faces = new NxU32[mNumTriangles * 3];

	for (int i = 0; i < vertexCount; i++)
	{
		NxVec3 vertex = objMesh->getVertex(i);
		verts[i].x = vertex.x;
		verts[i].y = vertex.y;
		verts[i].z = vertex.z;
	}
	for (int i = 0; i < mNumTriangles; i++)
	{
		ObjMeshTriangle tri = objMesh->getTriangle(i);
		faces[i * 3]     = tri.vertexNr[0];
		faces[i * 3 + 1] = tri.vertexNr[1];
		faces[i * 3 + 2] = tri.vertexNr[2];
	}

	desc.setToDefault();
	desc.numVertices           = vertexCount;
	desc.numTriangles          = mNumTriangles;
	desc.pointStrideBytes      = sizeof(NxVec3);
	desc.triangleStrideBytes   = 3 * sizeof(NxU32);
	desc.points                = verts;
	desc.triangles             = faces;
	desc.flags                 = 0;
	desc.flags                |= NX_CLOTH_MESH_WELD_VERTICES;
	desc.weldingDistance       = 0.5f;

	return true;
}

void PhysXCloth::init(NxClothDesc &desc, NxClothMeshDesc &meshDesc)
{
	allocateReceiveBuffers(meshDesc.numVertices, meshDesc.numTriangles);
	cookMesh(meshDesc);
	releaseMeshDescBuffers(meshDesc);

	desc.clothMesh = mClothMesh;
	desc.meshData  = mReceiveBuffers;

	assert(desc.isValid());
	mCloth = mScene->createCloth(desc);
	mCloth->wakeUp();

	mInitDone = true;
}

void PhysXCloth::allocateReceiveBuffers(NxU32 numVertices, NxU32 numTriangles)
{
	mPositions.resize(numVertices);
	mNormals.resize(numVertices);
	mIndices.resize(numTriangles * 3);

	mReceiveBuffers.setToDefault();
	mReceiveBuffers.verticesPosBegin         = &(mPositions[0].x);
	mReceiveBuffers.verticesPosByteStride    = sizeof(NxVec3);
	mReceiveBuffers.verticesNormalBegin      = &(mNormals[0].x);
	mReceiveBuffers.verticesNormalByteStride = sizeof(NxVec3);
	mReceiveBuffers.indicesBegin             = &(mIndices[0]);
	mReceiveBuffers.indicesByteStride        = sizeof(NxU32);
	mReceiveBuffers.maxVertices              = numVertices;
	mReceiveBuffers.numVerticesPtr           = &mNumVertices;
	mReceiveBuffers.maxIndices               = numTriangles * 3;
	mReceiveBuffers.numIndicesPtr            = &mNumIndices;
	mReceiveBuffers.dirtyBufferFlagsPtr      = &mMeshDirtyFlags;
}

bool PhysXCloth::cookMesh(NxClothMeshDesc &desc)
{
	assert(desc.isValid());

	MemoryWriteBuffer wb;
	if (!NxCookClothMesh(desc, wb))
		return false;
	MemoryReadBuffer rb(wb.data);
	mClothMesh = mScene->getPhysicsSDK().createClothMesh(rb);

	return true;
}

void PhysXCloth::releaseMeshDescBuffers(const NxClothMeshDesc& desc)
{
	NxVec3* y = (NxVec3*)desc.points;
	NxU32*  t = (NxU32*) desc.triangles;
	free(y);
	free(t);
}

void PhysXCloth::releaseReceiveBuffers()
{
	free(mReceiveBuffers.parentIndicesBegin);
	mReceiveBuffers.setToDefault();
}

void PhysXCloth::initOgreScene()
{
	mManualObj = mSceneMgr->createManualObject(mName);
	mClothNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	mClothNode->attachObject(mManualObj);
}

void PhysXCloth::render()
{
	mManualObj->clear();
	mManualObj->begin(mMaterial, Ogre::RenderOperation::OT_TRIANGLE_LIST);

	for (NxU32 i = 0; i < mNumVertices; i++)
	{
		Ogre::Vector3 pos = Ogre::Vector3(mPositions[i].x, mPositions[i].y, mPositions[i].z);
		mManualObj->position(pos);
		Ogre::Vector3 nor = Ogre::Vector3(mNormals[i].x, mNormals[i].y, mNormals[i].z);
		mManualObj->normal(nor);
		mManualObj->colour(Ogre::ColourValue::Green);
	}
	for (NxU32 i = 0; i < mNumTriangles; i++)
		mManualObj->triangle(mIndices[i * 3], mIndices[i * 3 + 1], mIndices[i * 3 + 2]);

	mManualObj->end();
}
