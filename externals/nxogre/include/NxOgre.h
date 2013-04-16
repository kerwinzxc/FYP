/** 
    
    This file is part of NxOgre.
    
    Copyright (c) 2009 Robin Southern, http://www.nxogre.org
    
    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:
    
    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.
    
    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.
    
*/

                                                                                       

#ifndef NXOGRE_H
#define NXOGRE_H

                                                                                       

#include "NxOgreAutoConfiguration.h"
#include "NxOgreActor.h"
#include "NxOgreActorMachinePart.h"
#include "NxOgreAllocatables.h"
#include "NxOgreAllocator.h"
#include "NxOgreAuxiliaryScene.h"
#include "NxOgreBounds.h"
#include "NxOgreBox.h"
#include "NxOgreBoxDescription.h"
#include "NxOgreBuffer.h"
#include "NxOgreCallback.h"
#include "NxOgreCapsule.h"
#include "NxOgreCapsuleDescription.h"
#include "NxOgreCharacterController.h"
#include "NxOgreCharacterControllerDescription.h"
#include "NxOgreClasses.h"
#include "NxOgreClassPrototypes.h"
#include "NxOgreCloth.h"
#include "NxOgreClothDescription.h"
#include "NxOgreCommon.h"
#include "NxOgreCompartment.h"
#include "NxOgreCompartmentDescription.h"
#include "NxOgreConfiguration.h"
#include "NxOgreContactPair.h"
#include "NxOgreConvex.h"
#include "NxOgreConvexDescription.h"
#include "NxOgreD6Joint.h"
#include "NxOgreD6JointDescription.h"
#include "NxOgreEntityReport.h"
#include "NxOgreEnums.h"
#include "NxOgreErrorStream.h"
#include "NxOgreException.h"
#include "NxOgreFileResource.h"
#include "NxOgreFileResourceProtocol.h"
#include "NxOgreFixedJoint.h"
#include "NxOgreFixedSceneTimer.h"
#include "NxOgreFlatYAML.h"
#include "NxOgreFlower.h"
#include "NxOgreFluid.h"
#include "NxOgreFluidDescription.h"
#include "NxOgreFluidEmitter.h"
#include "NxOgreFluidEmitterDescription.h"
#include "NxOgreForceField.h"
#include "NxOgreForceFieldDescription.h"
#include "NxOgreFunctions.h"
#include "NxOgreGroupsMask.h"
#include "NxOgreHeightField.h"
#include "NxOgreHeightFieldGeometry.h"
#include "NxOgreHeightFieldGeometryDescription.h"
#include "NxOgreHeightFieldManager.h"
#include "NxOgreHeightFieldSample.h"
#include "NxOgreIntVectors.h"
#include "NxOgreJoint.h"
#include "NxOgreJointDescription.h"
#include "NxOgreJointDriveDescription.h"
#include "NxOgreJointLimitDescription.h"
#include "NxOgreJointLimitSoftDescription.h"
#include "NxOgreKinematicActor.h"
#include "NxOgreKinematicSceneLink.h"
#include "NxOgreLinearInterpolationMap.h"
#include "NxOgreMachine.h"
#include "NxOgreMachinePart.h"
#include "NxOgreMallocAllocator.h"
#include "NxOgreManualHeightField.h"
#include "NxOgreManualMesh.h"
#include "NxOgreMap.h"
#include "NxOgreMaterial.h"
#include "NxOgreMaterialDescription.h"
#include "NxOgreMath.h"
#include "NxOgreMatrix.h"
#include "NxOgreMemoryResource.h"
#include "NxOgreMemoryResourceProtocol.h"
#include "NxOgreMesh.h"
#include "NxOgreMeshData.h"
#include "NxOgreMeshGenerator.h"
#include "NxOgreMeshManager.h"
#include "NxOgreMeshRenderable.h"
#include "NxOgreNXS.h"
#include "NxOgreMeshStats.h"
#include "NxOgreMotorDescription.h"
#include "NxOgreNodeRenderable.h"
#include "NxOgreNotice.h"
#include "NxOgreParticleData.h"
#include "NxOgrePath.h"
#include "NxOgrePhysXCallback.h"
#include "NxOgrePhysXConfiguration.h"
#include "NxOgrePhysXOutputStream.h"
#include "NxOgrePhysXParticleData.h"
#include "NxOgrePhysXPointer.h"
#include "NxOgrePhysXPrototypes.h"
#include "NxOgrePhysXRaycastReport.h"
#include "NxOgrePhysXStream.h"
#include "NxOgrePhysXUserAllocator.h"
#include "NxOgrePhysXUserEntityReport.h"
#include "NxOgrePlaneGeometry.h"
#include "NxOgrePlaneGeometryDescription.h"
#include "NxOgrePointerFunctions.h"
#include "NxOgrePointRenderable.h"
#include "NxOgrePrincipalScene.h"
#include "NxOgrePrototypeFunctions.h"
#include "NxOgreQuat.h"
#include "NxOgreRadian.h"
#include "NxOgreRay.h"
#include "NxOgreRaycastHit.h"
#include "NxOgreReason.h"
#include "NxOgreRemoteDebugger.h"
#include "NxOgreRemoteDebuggerListener.h"
#include "NxOgreRenderable.h"
#include "NxOgreResource.h"
#include "NxOgreResourceProtocol.h"
#include "NxOgreResourceSystem.h"
#include "NxOgreResourceStream.h"
#include "NxOgreRevoluteJoint.h"
#include "NxOgreRevoluteJointDescription.h"
#include "NxOgreRigidBody.h"
#include "NxOgreRigidBodyDescription.h"
#include "NxOgreRigidBodyFunctions.h"
#include "NxOgreRigidBodyEventIterator.h"
#include "NxOgreScene.h"
#include "NxOgreSceneDescription.h"
#include "NxOgreSceneGeometry.h"
#include "NxOgreSceneLink.h"
#include "NxOgreSceneTimer.h"
#include "NxOgreShape.h"
#include "NxOgreShapeDescription.h"
#include "NxOgreShapeFunctions.h"
#include "NxOgreSharedPointer.h"
#include "NxOgreSimple.h"
#include "NxOgreSingleton.h"
#include "NxOgreSoftBody.h"
#include "NxOgreSoftBodyDescription.h"
#include "NxOgreSphere.h"
#include "NxOgreSphereDescription.h"
#include "NxOgreSphericalJoint.h"
#include "NxOgreSphericalJointDescription.h"
#include "NxOgreSpringDescription.h"
#include "NxOgreStable.h"
#include "NxOgreString.h"
#include "NxOgreSweepQuery.h"
#include "NxOgreTimeListener.h"
#include "NxOgreTimer.h"
#include "NxOgreTimeStep.h"
#include "NxOgreTireFunction.h"
#include "NxOgreTriangle.h"
#include "NxOgreTriangleGeometry.h"
#include "NxOgreTriangleGeometryDescription.h"
#include "NxOgreTuple.h"
#include "NxOgreUtil.h"
#include "NxOgreVec2.h"
#include "NxOgreVec3.h"
#include "NxOgreVec4.h"
#include "NxOgreVector.h"
#include "NxOgreVersion.h"
#include "NxOgreVisualDebugger.h"
#include "NxOgreVisualDebuggerMeshData.h"
#include "NxOgreVolume.h"
#include "NxOgreWarning.h"
#include "NxOgreWheel.h"
#include "NxOgreWheelDescription.h"
#include "NxOgreWheelMachinePart.h"
#include "NxOgreWorld.h"
#include "NxOgreWorldDescription.h"
#include "NxOgreX.h"

                                                                                       

#endif