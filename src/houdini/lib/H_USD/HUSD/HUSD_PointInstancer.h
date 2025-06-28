/*
* Copyright 2024 Side Effects Software Inc.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
* Produced by:
*	Side Effects Software Inc.
*	123 Front Street West, Suite 1401
*	Toronto, Ontario
*       Canada   M5J 2M2
*	416-504-9876
*
*/

#ifndef __HUSD_PointInstancer_h__
#define __HUSD_PointInstancer_h__

#include <GU/GU_Detail.h>

#include <GA/GA_Types.h>

#include <HUSD/HUSD_API.h>
#include <HUSD/HUSD_DataHandle.h>
#include <HUSD/HUSD_TimeCode.h>

#include <UT/UT_StringHolder.h>


class HUSD_API HUSD_PointInstancer
{
public:
    enum IdSource
    {
      IDSOURCENONE,
      IDSOURCEATTRIBUTE,
      IDSOURCEPOINTNUMBER,
      IDSOURCEATTRIBUTEORPOINTNUMBER
    };

    enum ProtoSource
    {
      PROTOSOURCENONE,
      PROTOSOURCEATTRIBUTE,
      PROTOSOURCEPRIMPATH
    };

    enum CopyStyle
    {
      COPYSTYLEINVALID,
      COPYSTYLEOVERWRITE, // Overwrite existing array value with new length based on only existing points in sops
      COPYSTYLESPARSE     // look up existing usd values and sparsely update with data from sops (ie missing points in sops keep same value)
    };

    enum HUSD_XformType
    {
      XFORMTYPENONE,
      XFORMTYPEWORLD
    };

    // USD -> SOP
    static bool copyUsdPrimvarsToGeoAttrs(HUSD_AutoAnyLock      &lock,
                                          GU_Detail             *gdp,
                                          const GA_Offset       &offsetStart,
                                          const int             &numPoints,
                                          const UT_StringHolder &primvarFilter,
                                          const HUSD_TimeCode   &timeCode,
                                          const UT_StringRef    &PROTOSOURCEPRIMPATH);

    static bool copyUsdXformAttrsToGeoAttrs(HUSD_AutoAnyLock    &lock,
                                            GU_Detail           *gdp,
                                            const GA_Offset     &offsetStart,
                                            const int           &numPoints,
                                            const HUSD_TimeCode &timeCode,
                                            const UT_StringRef  &PROTOSOURCEPRIMPATH,
                                            bool                applyPrimXform=false,
                                            bool                copyPositions=true,
                                            bool                copyOrientations=true,
                                            bool                copyScales=true,
                                            bool                copyAccelerations=true,
                                            bool                copyVelocities=true,
                                            bool                copyAngularVelocities=true,
                                            bool                createUsdXformAttrib=true);

    static bool copyUsdIdAttrsToGeoAttrs(HUSD_AutoAnyLock       &lock,
                                         GU_Detail              *gdp,
                                         const GA_Offset        &offsetStart,
                                         const int              &numPoints,
                                         const HUSD_TimeCode    &timeCode,
                                         const UT_StringRef     &PROTOSOURCEPRIMPATH,
                                         bool                   useInvisIds=true,
                                         IdSource               idSource=IDSOURCENONE,
                                         ProtoSource            protoSource=PROTOSOURCENONE);

    static bool createBoundingBoxGeoAttr(HUSD_AutoAnyLock       &lock,
                                         GU_Detail              *gdp,
                                         const GA_Offset        &offsetStart,
                                         const int              &numPoints,
                                         const HUSD_TimeCode    &timeCode,
                                         const UT_StringRef     &PROTOSOURCEPRIMPATH,
                                         const UT_StringArray   &purposes,
                                         bool                   applyPrimXform=false);

};

#endif // __HUSD_PointInstancer_h__
