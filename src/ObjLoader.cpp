#include "ObjLoader.h"

ObjLoader::ObjLoader()
{
}

ObjLoader::~ObjLoader()
{
}

Ogre::MeshPtr ObjLoader::loadObj(const Ogre::String& filename)
{
	Ogre::String name, ext;
	Ogre::StringUtil::splitBaseFilename(filename, name, ext);

	Ogre::DataStreamPtr stream = Ogre::ResourceGroupManager::getSingleton().openResource(filename);
	
	return HandleStream(stream, name);
}

Ogre::MeshPtr ObjLoader::HandleStream(Ogre::DataStreamPtr stream, Ogre::String& name)
{
	Ogre::ManualObject *obj = OGRE_NEW Ogre::ManualObject(name);

	std::vector<Ogre::Vector3> vertices, normals;
	std::vector<Ogre::Vector2> textureCoords;

	Ogre::uint32 count = 0;
	bool hasTextures = true, hasNormals = true;

	while (!stream->eof())
	{
		Ogre::String str = stream->getLine();

		if (Ogre::StringUtil::startsWith(str, "usemtl "))
		{
			if (obj->getNumSections() != 0)
				obj->end();
			obj->begin(str.substr(7));
		}
		else if (Ogre::StringUtil::startsWith(str, "v "))
		{
			vertices.push_back(Ogre::StringConverter::parseVector3(str.substr(2)));
		}
		else if (Ogre::StringUtil::startsWith(str, "vn "))
		{
			normals.push_back(Ogre::StringConverter::parseVector3(str.substr(3)));
		}
		else if (Ogre::StringUtil::startsWith(str, "vt "))
		{
			textureCoords.push_back(Ogre::StringConverter::parseVector2(str.substr(3)));
		}
		else if (Ogre::StringUtil::startsWith(str, "f "))
		{
			Ogre::StringVector vec = Ogre::StringUtil::tokenise(str.substr(2), " ", "/");

			switch (vec.size())
			{
			case 9:
				break;
			case 6:
				hasNormals = false;
				vec.resize(9);
				vec[8] = "0";
				vec[7] = vec[5];
				vec[6] = vec[4];
				vec[5] = "0";
				vec[4] = vec[3];
				vec[3] = vec[2];
				vec[2] = "0";
				break;
			case 4:
				hasTextures = false;
				vec.resize(9);
				vec[8] = vec[3];
				vec[7] = "0";
				vec[6] = Ogre::StringUtil::tokenise(vec[2], " ")[1];
				vec[5] = Ogre::StringUtil::tokenise(vec[2], " ")[0];
				vec[4] = "0";
				vec[3] = Ogre::StringUtil::tokenise(vec[1], " ")[1];
				vec[2] = Ogre::StringUtil::tokenise(vec[1], " ")[0];
				vec[1] = "0";
				break;
			case 3:
				hasNormals = false;
				hasTextures = false;
				vec.resize(9);
				vec[8] = "0";
				vec[7] = "0";
				vec[6] = vec[2];
				vec[5] = "0";
				vec[4] = "0";
				vec[3] = vec[1];
				vec[2] = "0";
				vec[1] = "0";
				break;
			default:
				exit(1);
			}

			int Indices[3][3];

			for (Ogre::uint32 i = 0; i < 3; i++)
				for (Ogre::uint32 j = 0; j < 3; j++)
					Indices[i][j] = Ogre::StringConverter::parseUnsignedInt(vec[i * 3 + j]) - 1;

			vec.swap(Ogre::StringVector());

			for (Ogre::uint32 i = 0; i < 3; i++)
			{
				obj->position(vertices[Indices[i][0]]);
				if (hasTextures) obj->textureCoord(textureCoords[Indices[i][1]]);
				if (hasNormals)  obj->normal(normals[Indices[i][2]]);
			}

			obj->index(count++);
			obj->index(count++);
			obj->index(count++);
		}
	}
	if (obj->getNumSections() != 0)
		obj->end();

	vertices.swap(std::vector<Ogre::Vector3>());
	normals.swap(std::vector<Ogre::Vector3>());
	textureCoords.swap(std::vector<Ogre::Vector2>());

	stream->close();

	Ogre::MeshPtr mesh = obj->convertToMesh(name);
	OGRE_DELETE obj;

	return mesh;
}
