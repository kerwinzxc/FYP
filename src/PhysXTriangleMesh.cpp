#include "PhysXTriangleMesh.h"

PhysXTriangleMesh::PhysXTriangleMesh(NxScene *scene, Ogre::SceneManager* sceneMgr,
                                     NxTriangleMeshShapeDesc &desc, ObjMeshExt* objMesh)
	: mInitDone(false), mScene(scene), mTriangleMesh(NULL), mActor(NULL),
	  mSceneMgr(sceneMgr), mManualObj(NULL)
{
	NxTriangleMeshDesc meshDesc;
	saveMeshDesc(meshDesc, objMesh);
	init(desc, meshDesc);
}

PhysXTriangleMesh::~PhysXTriangleMesh()
{
	if (mInitDone)
	{
		if (mActor)
			mScene->releaseActor(*mActor);
		if (mTriangleMesh)
			mScene->getPhysicsSDK().releaseTriangleMesh(*mTriangleMesh);
	}

	if (mTriangleNode)
	{
		mTriangleNode->removeAndDestroyAllChildren();
		mSceneMgr->destroySceneNode(mTriangleNode);
	}
	if (mManualObj)
		mSceneMgr->destroyManualObject(mManualObj);
}

bool PhysXTriangleMesh::saveMeshDesc(NxTriangleMeshDesc &desc, ObjMeshExt* objMesh)
{
	int vertexCount    = objMesh->getNumVertices();
	int triangleCount  = objMesh->getNumTriangles();

	if (vertexCount == 0)
		return false;

	if (mSceneMgr != NULL)
		mManualObj = mSceneMgr->createManualObject(objMesh->getName());
	else
		return false;

	NxVec3* verts = (NxVec3*)malloc(sizeof(NxVec3) * vertexCount);
	NxU32*  faces = (NxU32*) malloc(sizeof(NxU32)  * triangleCount * 3);

	mManualObj->estimateVertexCount(vertexCount);
	mManualObj->estimateIndexCount(triangleCount * 3);

	std::vector<int> vertexOffsets;
	std::vector<int> triOffsets;
	vertexOffsets.resize(objMesh->getNumMaterials()); vertexOffsets.clear();
	triOffsets.resize(objMesh->getNumMaterials());    triOffsets.clear();

	int prevMat = objMesh->getTriangle(0).materialNr;
	int max = -1;
	for (int j = 0; j < triangleCount; j++)
	{
		ObjMeshTriangle tri = objMesh->getTriangle(j);
		faces[j * 3]     = tri.vertexNr[0];
		faces[j * 3 + 1] = tri.vertexNr[1];
		faces[j * 3 + 2] = tri.vertexNr[2];
		if (prevMat == tri.materialNr)
		{
			if (*std::max_element(tri.vertexNr, tri.vertexNr + 3) > max)
				max = *std::max_element(tri.vertexNr, tri.vertexNr + 3);
		}
		else
		{
			prevMat = tri.materialNr;
			vertexOffsets.push_back(max);
			triOffsets.push_back(j - 1);
			max = *std::max_element(tri.vertexNr, tri.vertexNr + 3);
		}
	}
	vertexOffsets.push_back(max);
	triOffsets.push_back(triangleCount - 1);

	int j = 0, k = 0, offset = 0;
	for (size_t i = 0; i < vertexOffsets.size(); ++i)
	{
		ObjMeshMaterial material = objMesh->getMaterial(i);
		mManualObj->begin(material.name, Ogre::RenderOperation::OT_TRIANGLE_LIST);

		for (; j <= vertexOffsets[i]; j++)
		{
			NxVec3 vertex = objMesh->getVertex(j);
			mManualObj->position(vertex.x, vertex.y, vertex.z);
			NxVec3 normal = objMesh->getNormal(j);
			mManualObj->normal(normal.x, normal.y, normal.z);
			TexCoord texCoord = objMesh->getTexCoord(j);
			mManualObj->textureCoord(texCoord.u, texCoord.v);

			verts[j].x = vertex.x;
			verts[j].y = vertex.y;
			verts[j].z = vertex.z;
		}
		for (; k <= triOffsets[i]; k++)
		{
			ObjMeshTriangle tri = objMesh->getTriangle(k);
			mManualObj->triangle(tri.vertexNr[0] - offset, tri.vertexNr[1] - offset, tri.vertexNr[2] - offset);
		}
		offset = vertexOffsets[i] + 1;

		mManualObj->end();
	}
	mTriangleNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	mTriangleNode->attachObject(mManualObj);

	desc.setToDefault();
	desc.numVertices               = vertexCount;
	desc.numTriangles              = triangleCount;
	desc.pointStrideBytes          = sizeof(NxVec3);
	desc.triangleStrideBytes       = 3 * sizeof(NxU32);
	desc.points                    = verts;
	desc.triangles                 = faces;
	desc.flags                     = 0;
	desc.heightFieldVerticalAxis   = NX_Y;
	desc.heightFieldVerticalExtent = 1000.0f;

	return true;
}

void PhysXTriangleMesh::init(NxTriangleMeshShapeDesc &desc, NxTriangleMeshDesc &meshDesc)
{
	cookMesh(meshDesc);
	releaseMeshDescBuffers(meshDesc);

	desc.meshData = mTriangleMesh;

	NxActorDesc ActorDesc;
	ActorDesc.shapes.pushBack(&desc);
	mActor = mScene->createActor(ActorDesc);

	mInitDone = true;
}

bool PhysXTriangleMesh::cookMesh(NxTriangleMeshDesc& desc)
{
	MemoryWriteBuffer wb;
	assert(desc.isValid());

	if (!NxCookTriangleMesh(desc, wb))
		return false;

	MemoryReadBuffer rb(wb.data);
	mTriangleMesh = mScene->getPhysicsSDK().createTriangleMesh(rb);
	return true;
}

void PhysXTriangleMesh::releaseMeshDescBuffers(const NxTriangleMeshDesc& desc)
{
	NxVec3* p = (NxVec3*) desc.points;
	NxU32*  t = (NxU32*)  desc.triangles;
	free(p);
	free(t);
}
