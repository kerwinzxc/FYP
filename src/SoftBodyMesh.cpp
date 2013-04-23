#include "SoftBodyMesh.h"

#include "MeshHash.h"

SoftBodyMesh::SoftBodyMesh(Ogre::MeshPtr mesh, std::vector<std::vector<unsigned int>> *indexLinks)
{
	mMesh = mesh;
	mIndexLinks = indexLinks;

	for (int i = 0; i < (int) mIndexLinks->size(); i++)
	{
		Ogre::SubMesh* submesh = mMesh->getSubMesh((*mIndexLinks)[i][0]);
		Ogre::VertexData* vertexData = submesh->vertexData;
		const Ogre::VertexElement* posElem = vertexData->vertexDeclaration->findElementBySemantic(Ogre::VES_POSITION);
		Ogre::HardwareVertexBufferSharedPtr vbuf = vertexData->vertexBufferBinding->getBuffer(posElem->getSource());
		unsigned char* vertex = static_cast<unsigned char*>(vbuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));

		Ogre::Real* pReal;
		size_t offset = vbuf->getVertexSize();
		for (size_t j = 0; j < vertexData->vertexCount; ++j, vertex += offset)
		{
			posElem->baseVertexPointerToElement(vertex, &pReal);
			mBounds.include(NxVec3(pReal[0], pReal[1], pReal[2]));
		}
		vbuf->unlock();
	}
}

SoftBodyMesh::~SoftBodyMesh() {}

void SoftBodyMesh::loadTetFile(const Ogre::String &filename, NxArray<NxVec3>* vertices, NxArray<NxU32>* indices)
{
	Ogre::DataStreamPtr stream = Ogre::ResourceGroupManager::getSingleton().openResource(filename);

	while (!stream->eof())
	{
		Ogre::String str = stream->getLine();

		if (Ogre::StringUtil::startsWith(str, "v "))
		{
			Ogre::Vector3 vec = Ogre::StringConverter::parseVector3(str.substr(2));
			vertices->push_back(Vector3toVec3(vec));
		}
		else if (Ogre::StringUtil::startsWith(str, "t "))
		{
			Ogre::StringVector vec = Ogre::StringUtil::tokenise(str.substr(2), " ");
			for (Ogre::uint32 i = 0; i < 4; i++)
				indices->push_back(Ogre::StringConverter::parseUnsignedInt(vec[i]));
			vec.swap(Ogre::StringVector());
		}
	}
	stream->close();
}

// ----------------------------------------------------------------------
// computes barycentric coordinates
NxVec3 SoftBodyMesh::computeBaryCoords(NxVec3 vertex, NxVec3 p0, NxVec3 p1, NxVec3 p2, NxVec3 p3)
{
	NxVec3 baryCoords;

	NxVec3 q  = vertex - p3;
	NxVec3 q0 = p0 - p3;
	NxVec3 q1 = p1 - p3;
	NxVec3 q2 = p2 - p3;

	NxMat33 m;
	m.setColumn(0, q0);
	m.setColumn(1, q1);
	m.setColumn(2, q2);

	NxReal det = m.determinant();

	m.setColumn(0, q);
	baryCoords.x = m.determinant();

	m.setColumn(0, q0); m.setColumn(1, q);
	baryCoords.y = m.determinant();

	m.setColumn(1, q1); m.setColumn(2, q);
	baryCoords.z = m.determinant();

	if (det != 0.0f)
		baryCoords /= det;

	return baryCoords;
}

// -----------------------------------------------------------------------
// compute the links between the surface mesh and tetrahedras
void SoftBodyMesh::buildTetraLinks(const NxVec3 *vertices, const NxU32 *indices,
                                   const NxU32 numTets)
{
	if(!mTetraLinks.empty())
		return;

	mTetraLinks.clear();

	MeshHash* hash = new MeshHash();

	// hash tetrahedra for faster search
	hash->setGridSpacing(mBounds.min.distance(mBounds.max) * 0.1f);

	for (NxU32 i = 0; i < numTets; i++) {
		const NxU32 *ix = &indices[4*i];
		NxBounds3 tetraBounds;
		tetraBounds.setEmpty();
		tetraBounds.include(vertices[*ix++]);
		tetraBounds.include(vertices[*ix++]);
		tetraBounds.include(vertices[*ix++]);
		tetraBounds.include(vertices[*ix++]);
		hash->add(tetraBounds, i);
	}

	for (NxU32 i = 0; i < (int) mIndexLinks->size(); i++) {
		// prepare datastructure for drained tetras
		mDrainedTriVertices.push_back(false);

		MeshTetraLink tmpLink;

		Ogre::SubMesh* submesh = mMesh->getSubMesh((*mIndexLinks)[i][0]);
		Ogre::VertexData* vertexData = submesh->vertexData;
		const Ogre::VertexElement* posElem = vertexData->vertexDeclaration->findElementBySemantic(Ogre::VES_POSITION);
		Ogre::HardwareVertexBufferSharedPtr vbuf = vertexData->vertexBufferBinding->getBuffer(posElem->getSource());
		unsigned char* vertex = static_cast<unsigned char*>(vbuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));

		Ogre::Real* pReal;
		size_t offset = vbuf->getVertexSize();
		posElem->baseVertexPointerToElement(vertex + (*mIndexLinks)[i][1] * offset, &pReal);
		NxVec3 triVert = NxVec3(pReal[0], pReal[1], pReal[2]);
		vbuf->unlock();

		std::vector<int> itemIndices;
		hash->queryUnique(triVert, itemIndices);

		NxReal minDist = 0.0f;
		NxVec3 b;
		int num, isize;
		num = isize = (int) itemIndices.size();
		if (num == 0) num = numTets;

		for (int i = 0; i < num; i++) {
			int j = i;
			if (isize > 0) j = itemIndices[i];

			const NxU32 *ix = &indices[j*4];
			const NxVec3 &p0 = vertices[*ix++];
			const NxVec3 &p1 = vertices[*ix++];
			const NxVec3 &p2 = vertices[*ix++];
			const NxVec3 &p3 = vertices[*ix++];

			NxVec3 b = computeBaryCoords(triVert, p0, p1, p2, p3);

			// is the vertex inside the tetrahedron? If yes we take it
			if (b.x >= 0.0f && b.y >= 0.0f && b.z >= 0.0f && (b.x + b.y + b.z) <= 1.0f) {
				tmpLink.barycentricCoords = b;
				tmpLink.tetraNr = j;
				break;
			}

			// otherwise, if we are not in any tetrahedron we take the closest one
			NxReal dist = 0.0f;
			if (b.x + b.y + b.z > 1.0f) dist = b.x + b.y + b.z - 1.0f;
			if (b.x < 0.0f) dist = (-b.x < dist) ? dist : -b.x;
			if (b.y < 0.0f) dist = (-b.y < dist) ? dist : -b.y;
			if (b.z < 0.0f) dist = (-b.z < dist) ? dist : -b.z;

			if (i == 0 || dist < minDist) {
				minDist = dist;
				tmpLink.barycentricCoords = b;
				tmpLink.tetraNr = j;
			}
		}

		mTetraLinks.push_back(tmpLink);
	}
	delete hash;
}

void SoftBodyMesh::allocateReceiveBuffers(NxU32 numVertices, NxU32 numTetrahedra)
{
	// here we setup the buffers through which the SDK returns the dynamic softbody data
	// we reserve more memory for vertices than the initial mesh takes
	// because tearing creates new vertices
	// the SDK only tears softbodies as long as there is room in these buffers

	NxU32 maxVertices = 2 * numVertices;
	mReceiveBuffers.verticesPosBegin = (NxVec3*) malloc(sizeof(NxVec3) * maxVertices);
	mReceiveBuffers.verticesPosByteStride = sizeof(NxVec3);
	mReceiveBuffers.maxVertices = maxVertices;
	mReceiveBuffers.numVerticesPtr = (NxU32*) malloc(sizeof(NxU32));

	mReceiveBuffers.verticesNormalBegin = new NxVec3[maxVertices];
	mReceiveBuffers.verticesNormalByteStride = sizeof(NxVec3);

	// the number of tetrahedra is constant, even if the softbody is torn
	NxU32 maxIndices = 4 * numTetrahedra;
	mReceiveBuffers.indicesBegin = (NxU32*) malloc(sizeof(NxU32) * maxIndices);
	mReceiveBuffers.indicesByteStride = sizeof(NxU32);
	mReceiveBuffers.maxIndices = maxIndices;
	mReceiveBuffers.numIndicesPtr = (NxU32*) malloc(sizeof(NxU32));

	// init the buffers in case we want to draw the mesh
	// before the SDK as filled in the correct values
	*mReceiveBuffers.numVerticesPtr = 0;
	*mReceiveBuffers.numIndicesPtr = 0;
}

// ----------------------------------------------------------------------
void SoftBodyMesh::removeTrisRelatedToVertex(const std::vector<unsigned int> &indexLink)
{
	Ogre::SubMesh* submesh = mMesh->getSubMesh(indexLink[0]);
	Ogre::VertexData* vertexData = submesh->vertexData;
	const Ogre::VertexElement* posElem = vertexData->vertexDeclaration->findElementBySemantic(Ogre::VES_POSITION);
	Ogre::HardwareVertexBufferSharedPtr vbuf = vertexData->vertexBufferBinding->getBuffer(posElem->getSource());
	unsigned char* vertex = static_cast<unsigned char*>(vbuf->lock(Ogre::HardwareBuffer::HBL_NORMAL));

	Ogre::Real* pReal;
	size_t offset = vbuf->getVertexSize();
	for (int i = 1; i < (int) indexLink.size(); i++)
	{
		posElem->baseVertexPointerToElement(vertex + indexLink[i] * offset, &pReal);
		pReal[1] = pReal[0];
		pReal[2] = pReal[0];
	}

	vbuf->unlock();
}

// ----------------------------------------------------------------------
bool SoftBodyMesh::updateTetraLinks()
{
	if (mTetraLinks.size() != mIndexLinks->size()) return false;

	NxU32 numVertices = *mReceiveBuffers.numVerticesPtr;
	NxU32 numTetrahedra = *mReceiveBuffers.numIndicesPtr / 4;
	const NxVec3 *vertices = (NxVec3*)mReceiveBuffers.verticesPosBegin;
	NxU32* indices = (NxU32*)mReceiveBuffers.indicesBegin;

	for (int i = 0; i < (int) mIndexLinks->size(); i++) {
		MeshTetraLink &link = mTetraLinks[i];

		if (!mDrainedTriVertices[i]) {
			// only done if tetra was not drained before
			const NxU32 *ix = &indices[4 * link.tetraNr];

			if (*ix == *(ix + 1)) {
				// this tetra was drained
				removeTrisRelatedToVertex((*mIndexLinks)[i]);
				mDrainedTriVertices[i] = true;
				continue;
			}

			const NxVec3 &p0 = vertices[*ix++];
			const NxVec3 &p1 = vertices[*ix++];
			const NxVec3 &p2 = vertices[*ix++];
			const NxVec3 &p3 = vertices[*ix++];

			NxVec3 &b = link.barycentricCoords;
			NxVec3 vec = p0 * b.x + p1 * b.y + p2 * b.z + p3 * (1.0f - b.x - b.y - b.z);

			Ogre::SubMesh* submesh = mMesh->getSubMesh((*mIndexLinks)[i][0]);
			Ogre::VertexData* vertexData = submesh->vertexData;
			const Ogre::VertexElement* posElem = vertexData->vertexDeclaration->findElementBySemantic(Ogre::VES_POSITION);
			Ogre::HardwareVertexBufferSharedPtr vbuf = vertexData->vertexBufferBinding->getBuffer(posElem->getSource());
			unsigned char* vertex = static_cast<unsigned char*>(vbuf->lock(Ogre::HardwareBuffer::HBL_NORMAL));

			Ogre::Real* pReal;
			size_t offset = vbuf->getVertexSize();
			for (int j = 1; j < (int) (*mIndexLinks)[i].size(); j++)
			{
				posElem->baseVertexPointerToElement(vertex + (*mIndexLinks)[i][j] * offset, &pReal);
				pReal[0] = vec.x;
				pReal[1] = vec.y;
				pReal[2] = vec.z;
			}

			vbuf->unlock();
		}
	}
	// updateNormals();

	return true;
}
