#! python
# -*- coding: utf-8 -*-

import os
import sys

def readObj(filename):
	tIndex = None
	tVertices = None

	for line in open(filename, 'r'):
		if line.startswith('#'): continue
		values = line.split()
		if not values: continue
		if values[0] == 'usemtl':
			if tVertices == None:
				pass
			elif tIndex == None:
				vertices.append(tVertices)
				normals.append(tNormals)
				textures.append(tTextures)
				faces.append(tFaces)
			else:
				vertices[tIndex].extend(tVertices)
				normals[tIndex].extend(tNormals)
				textures[tIndex].extend(tTextures)
				faces[tIndex].extend(tFaces)
				tIndex = None
			if values[1] in materials:
				tIndex = materials.index(values[1])
			else:
				materials.append(values[1])
			tVertices = []
			tNormals  = []
			tTextures = []
			tFaces    = []
		elif values[0] == 'v':
			tVertices.append(map(float, values[1:4]))
		elif values[0] == 'vn':
			tNormals.append(map(float, values[1:4]))
		elif values[0] == 'vt':
			tTextures.append(map(float, values[1:3]))
		elif values[0] == 'f':
			face = []
			texcoords = []
			norms = []
			for v in values[1:]:
				w = v.split('/')
				face.append(int(w[0]))
			tFaces.append(face)
	if tIndex == None:
		vertices.append(tVertices)
		normals.append(tNormals)
		textures.append(tTextures)
		faces.append(tFaces)
	else:
		vertices[tIndex].extend(tVertices)
		normals[tIndex].extend(tNormals)
		textures[tIndex].extend(tTextures)
		faces[tIndex].extend(tFaces)

def sortIndices():
	for i in range(0, len(faces)):
		Indices = []
		for j in range(0, len(faces[i])):
			Indices.append(faces[i][j][0])
			Indices.append(faces[i][j][1])
			Indices.append(faces[i][j][2])
		Indices = sorted(list(set(Indices)))
		Max = Indices[len(Indices) - 1]
		Min = Indices[0]
		starts = [Min]
		ends   = []
		for j in range(0, len(Indices) - 1):
			if Indices[j + 1] - Indices[j] != 1:
				ends.append(Indices[j])
				starts.append(Indices[j+1])
		ends.append(Max)
		offset = 0
		for j in range(1, len(starts)):
			offset += starts[j] - ends[j - 1] - 1
			for k in range(0, len(faces[i])):
				for l in range(0, 3):
					if faces[i][k][l] in range(starts[j], ends[j] + 1):
						faces[i][k][l] -= offset
		if Min != 1:
			offset = Min - 1
			for j in range(0, len(faces[i])):
				for k in range(0, 3):
					faces[i][j][k] -= offset

def writeOutput():
	name, ext = os.path.splitext(filename)
	f = open(name + '.new' + ext, 'w')
	offset = 0
	for i in range(0, len(materials)):
		f.write('usemtl ' + materials[i] + '\n')
		for vertex in vertices[i]:
			f.write('v '  + str(vertex[0]) + ' ' + str(vertex[1]) + ' ' + str(vertex[2]) + '\n')
		for normal in normals[i]:
			f.write('vn ' + str(normal[0]) + ' ' + str(normal[1]) + ' ' + str(normal[2]) + '\n')
		for texture in textures[i]:
			f.write('vt ' + str(texture[0]) + ' ' + str(texture[1]) + '\n')
		for face in faces[i]:
			for k in range(0, 3):
				face[k] += offset
			f.write('f '  + str(face[0]) + '/' + str(face[0]) + '/' + str(face[0]) + ' ' + str(face[1]) + '/' + str(face[1]) + '/' + str(face[1]) + ' ' + str(face[2]) + '/' + str(face[2]) + '/' + str(face[2]) + '\n')
		offset += len(vertices[i])
		f.write('\n')
	f.close()

if __name__ == '__main__':
	filename = sys.argv[1]
	materials = []
	vertices  = []
	normals   = []
	textures  = []
	faces     = []

	readObj(filename)
	sortIndices()
	writeOutput()
