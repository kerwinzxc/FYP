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

                                                                                       

#ifndef NXOGRE_HEIGHTFIELDDATA_H
#define NXOGRE_HEIGHTFIELDDATA_H

                                                                                       

#include "NxOgreStable.h"
#include "NxOgreCommon.h"
#include "NxOgreHeightFieldSample.h"

                                                                                       

namespace NxOgre
{

                                                                                       

class NxOgrePublicClass HeightFieldData : public ResourceAllocatable
{
  
 public:
  
  /** \brief MeshData constructor.
  */
                                   HeightFieldData();
  
  /** \brief MeshData destructor.
  */
                                  ~HeightFieldData();
  
  /** \brief Set the name to null-string, type to unknown and clean all Buffers.
  */
  void                             clear();
  
  /** \brief
  */
  void                             cook(Resource*);
  
  /** \brief
  */
  void                             cook(const Path&);
  
  /*! function. cookInternal
      desc.
          Cook internally without using the X file format functions.
      note.
          HeightField will be loaded automatically into the HeightField under mName.
  */
  HeightField*                     cookQuickly();
  
  String                           mName;
  
  unsigned int                     mNbRows;
  
  unsigned int                     mNbColumns;
  
  float                            mThickness;
  
  float                            mVerticalExtent;
  
  float                            mConvexEdgeThreshold;
  
  bool                             mNoEdgeBoundaries;
  
  buffer<HeightFieldSample>        mSamples;
  
};

                                                                                       

} // namespace NxOgre

                                                                                       

#endif
