#include "PhysXTriangleMesh.h"
#include "ObjMeshExt.h"

PhysXTriangleMesh::PhysXTriangleMesh(NxScene *scene, Ogre::SceneManager* sceneMgr,
                                     NxTriangleMeshShapeDesc &desc, char* objFilePath)
	: mInitDone(false), mScene(scene), mTriangleMesh(NULL), mActor(NULL),
	  mSceneMgr(sceneMgr), mManualObj(NULL)
{
	NxTriangleMeshDesc meshDesc;
	saveMeshDesc(meshDesc, objFilePath);
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
}

bool PhysXTriangleMesh::saveMeshDesc(NxTriangleMeshDesc &desc, char* filepath)
{
	ObjMeshExt obj;
	obj.loadFromObjFile(filepath);

	int vertexCount    = obj.getNumVertices();
	int triangleCount  = obj.getNumTriangles();

	if (vertexCount == 0)
		return false;

	NxVec3* verts = new NxVec3[vertexCount];
	NxU32*  faces = new NxU32[triangleCount * 3];

	if (mSceneMgr != NULL)
	{
		mManualObj = mSceneMgr->createManualObject(obj.getName());
	}
	else
	{
		delete[] verts;
		delete[] faces;
		return false;
	}

	mManualObj->estimateVertexCount(vertexCount);
	mManualObj->estimateIndexCount(triangleCount * 3);

	std::vector<int> vertexOffsets;
	std::vector<int> triOffsets;
	vertexOffsets.resize(obj.getNumMaterials()); vertexOffsets.clear();
	triOffsets.resize(obj.getNumMaterials());    triOffsets.clear();

	int prevMat = obj.getTriangle(0).materialNr;
	int max = -1;
	for (int j = 0; j < triangleCount; j++)
	{
		ObjMeshTriangle tri = obj.getTriangle(j);
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
		ObjMeshMaterial material = obj.getMaterial(i);
		mManualObj->begin(material.name, Ogre::RenderOperation::OT_TRIANGLE_LIST);

		for (; j <= vertexOffsets[i]; j++)
		{
			NxVec3 vertex = obj.getVertex(j);
			mManualObj->position(vertex.x, vertex.y, vertex.z);
			NxVec3 normal = obj.getNormal(j);
			mManualObj->normal(normal.x, normal.y, normal.z);
			TexCoord texCoord = obj.getTexCoord(j);
			mManualObj->textureCoord(texCoord.u, texCoord.v);

			verts[j].x = vertex.x;
			verts[j].y = vertex.y;
			verts[j].z = vertex.z;
		}
		for (; k <= triOffsets[i]; k++)
		{
			ObjMeshTriangle tri = obj.getTriangle(k);
			mManualObj->triangle(tri.vertexNr[0] - offset, tri.vertexNr[1] - offset, tri.vertexNr[2] - offset);
		}
		offset = vertexOffsets[i] + 1;

		mManualObj->end();
	}
	mTriangleNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	mTriangleNode->attachObject(mManualObj);

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
