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

#include "HUSD_PointInstancer.h"

#include <HUSD/HUSD_Constants.h>
#include <HUSD/HUSD_GetAttributes.h>
#include <HUSD/HUSD_Info.h>
#include <HUSD/HUSD_SetAttributes.h>

#include <HUSD/XUSD_Utils.h>
#include <HUSD/XUSD_Data.h>

#include <HUSD/XUSD_AttributeUtils.h>

#include <UT/UT_StringHolder.h>
#include <UT/UT_VarEncode.h>
#include <UT/UT_Set.h>

#include <GA/GA_Names.h>
#include <GA/GA_Handle.h>
#include <GA/GA_ATINumericArray.h>
#include <GA/GA_ATIStringArray.h>
#include <GA/GA_Types.h>
#include <GA/GA_AttributeInstanceMatrix.h>

#include <GEO/GEO_Primitive.h>

#include <pxr/base/vt/value.h>
#include <pxr/usd/sdf/types.h>
#include <pxr/usd/usdGeom/primvarsAPI.h>
#include <pxr/usd/usdGeom/xformCache.h>
#include <pxr/usd/usdGeom/tokens.h>

#include <pxr/base/gf/matrix2f.h>
#include <pxr/base/gf/matrix3f.h>
#include <pxr/base/gf/matrix4f.h>

#include <gusd/UT_TypeTraits.h>
#include <gusd/UT_Gf.h>

PXR_NAMESPACE_USING_DIRECTIVE

static constexpr UT_StringLit theVisibilityAttr("usdvisibility");
static constexpr UT_StringLit theProtoIdsAttr("protoIndices");
static constexpr UT_StringLit theProtoPathsAttr("protoPaths");
static constexpr UT_StringLit theUsdXformAttr("usdxform");

namespace
{
void
husdGetSopAttrName(const UT_StringRef &usdAttrName,
               UT_StringHolder &attrName)
{
    if (usdAttrName.equal(UsdGeomTokens->primvarsDisplayColor.GetString()))
        attrName = GA_Names::Cd;
    else if (usdAttrName.equal(UsdGeomTokens->primvarsDisplayOpacity.GetString()))
        attrName = GA_Names::Alpha;
    else if (usdAttrName.equal(UsdGeomTokens->accelerations.GetString()))
        attrName = GA_Names::accel;
    else if (usdAttrName.equal(UsdGeomTokens->velocities.GetString()))
        attrName = GA_Names::v;
    else if (usdAttrName.equal(UsdGeomTokens->angularVelocities.GetString()))
        attrName = GA_Names::w;
    else
        attrName = UT_VarEncode::encodeVar(usdAttrName);
}

// TODO: Need to handle ids attr for these
template <class ELEMTYPE, class ATTRTYPE, GA_Storage STORAGE=GA_STORE_INVALID>
bool
husdCopyToAttr(GU_Detail *detail,
           const GA_Offset &offsetStart,
           const int &numPoints,
           const UT_StringHolder &usdAttrName,
           const VtValue &value,
           GA_TypeInfo info=GA_TYPE_VOID)
{
    if (!value.IsHolding<ELEMTYPE>())
        return false;

    int tupleSize = GusdGetTupleSize<ELEMTYPE>();

    GA_RWHandleT<ATTRTYPE> attr(detail, GA_ATTRIB_POINT, usdAttrName);
    if (!attr.isValid())
    {
        attr = detail->getAttributes().createTupleAttribute(
                GA_ATTRIB_POINT, GA_SCOPE_PUBLIC, usdAttrName,
                STORAGE,tupleSize, GA_Defaults(0.0));

        if (info != GA_TYPE_VOID)
            attr->setTypeInfo(info);
    }

    ATTRTYPE utValue;
    HUSDgetValue(value, utValue);

    GA_Offset end = offsetStart + numPoints;
    for (GA_Offset ptoff = offsetStart; ptoff < end; ++ptoff)
    {
        attr.set(ptoff, utValue);
    }
    return true;
}

template <class ELEMTYPE, class ATTRTYPE, GA_Storage STORAGE=GA_STORE_INVALID>
bool
husdCopyToArrayAttr(GU_Detail *detail,
                    const GA_Offset &offsetStart,
                    const int &numPoints,
                    const UT_StringHolder &usdAttrName,
                    const VtValue &value,
                    const GA_TypeInfo info=GA_TYPE_VOID)
{
    if (!value.IsHolding<VtArray<ELEMTYPE>>() ||
            value.GetArraySize() != numPoints)
        return false;

    int tupleSize = GusdGetTupleSize<ELEMTYPE>();

    GA_RWHandleT<ATTRTYPE> attr(detail, GA_ATTRIB_POINT, usdAttrName);
    if (!attr.isValid())
    {
        attr = detail->getAttributes().createTupleAttribute(
                GA_ATTRIB_POINT, GA_SCOPE_PUBLIC, usdAttrName, STORAGE,
                tupleSize, GA_Defaults(0.0));

        if (info != GA_TYPE_VOID)
            attr->setTypeInfo(info);
    }

    UT_Array<ATTRTYPE> utValues;
    HUSDgetValue(value, utValues);

    for (int i = 0; i < numPoints; ++i)
    {
        attr.set(offsetStart + i, utValues[i]);
    }
    return true;
}

template <class ELEMTYPE, class ATTRTYPE, GA_Storage STORAGE=GA_STORE_INVALID>
bool
_husdCopyToAttr(GU_Detail *detail,
            const GA_Offset &offsetStart,
            const int &numPoints,
            const UT_StringHolder &usdAttrName,
            const VtValue &value,
            const GA_TypeInfo info=GA_TYPE_VOID)
{
    if (husdCopyToArrayAttr<ELEMTYPE, ATTRTYPE, STORAGE>(detail, offsetStart, numPoints, usdAttrName, value, info))
        return true;
    return husdCopyToAttr<ELEMTYPE, ATTRTYPE, STORAGE>(detail, offsetStart, numPoints, usdAttrName, value, info);
}

} // namespace


bool HUSD_PointInstancer::copyUsdIdAttrsToGeoAttrs(HUSD_AutoAnyLock &lock,
                                          GU_Detail *gdp,
                                          const GA_Offset &offsetStart,
                                          const int &numPoints,
                                          const HUSD_TimeCode &timeCode,
                                          const UT_StringRef &primPath,
                                          bool useInvisIds /*= true*/,
                                          IdSource idSource /*= IDSOURCENONE*/,
                                          ProtoSource protoSource /*=PROTOSOURCENONE*/)
{
    GA_RWHandleI       idsAttr;
    GA_RWHandleS       invisIdsAttr;
    GA_RWHandleI       protoIndicesAttr;
    GA_RWHandleS       protoPathsAttr;
    UT_Array<int>      ids;
    UT_Array<int>      invisIds;
    UT_Array<int>      protoIndices;
    UT_StringArray     protoPaths;
    bool               useIds = false;
    GA_Offset          ptoff;
    HUSD_GetAttributes getAttrs(lock);
    HUSD_Info          info(lock);

    if (idSource == IDSOURCEATTRIBUTE || idSource == IDSOURCEATTRIBUTEORPOINTNUMBER)
    {
        getAttrs.getAttributeArray(primPath, UsdGeomTokens->ids.GetString(),
                             ids, timeCode);

        if (idSource == IDSOURCEATTRIBUTEORPOINTNUMBER && ids.size() == 0)
            idSource = IDSOURCEPOINTNUMBER;
        else
            idSource = IDSOURCEATTRIBUTE;
    }

    if (useInvisIds)
    {
        getAttrs.getAttributeArray(
                primPath, UsdGeomTokens->invisibleIds.GetString(), invisIds,
                timeCode);
    }
    getAttrs.getAttributeArray(primPath, UsdGeomTokens->protoIndices.GetString(),
                               protoIndices, timeCode);
    info.getRelationshipTargets(primPath, UsdGeomTokens->prototypes.GetString(),
                               protoPaths);

    if (idSource != IDSOURCENONE)
    {
        if (idSource == IDSOURCEATTRIBUTE)
            useIds = ids.size() == numPoints;
        else
            useIds = true;
    }
    if (useIds)
    {
        idsAttr = gdp->findPointAttribute("id");
        if (!idsAttr.isValid())
        {
            idsAttr = gdp->addIntTuple(GA_ATTRIB_POINT, "id", 1);
        }
    }
    
    useInvisIds = !invisIds.isEmpty();
    if (useInvisIds)
    {
        invisIdsAttr = gdp->findPointAttribute(theVisibilityAttr.asRef());
        if (!invisIdsAttr.isValid())
        {
            invisIdsAttr = gdp->addStringTuple(GA_ATTRIB_POINT, theVisibilityAttr.asRef(), 1);
        }
    }
    
    if (protoSource == PROTOSOURCEPRIMPATH)
    {
        protoPathsAttr = gdp->findPointAttribute(theProtoPathsAttr.asRef());
        if (!protoPathsAttr.isValid())
            protoPathsAttr = gdp->addStringTuple(
                    GA_ATTRIB_POINT, theProtoPathsAttr.asRef(), 1);
    }
    else if (protoSource == PROTOSOURCEATTRIBUTE)
    {
        protoIndicesAttr = gdp->findPointAttribute(theProtoIdsAttr.asRef());
        if (!protoIndicesAttr.isValid())
            protoIndicesAttr = gdp->addIntTuple(
                    GA_ATTRIB_POINT, theProtoIdsAttr.asRef(), 1);
    }
    
    UT_Set<size_t> invisIdsSet;
    for (size_t invisId : invisIds)
        invisIdsSet.insert(invisId);
    
    int id;
    for (int i = 0; i < numPoints; ++i)
    {
        if (idSource == IDSOURCEATTRIBUTE && useIds) //todo: verify this also means idSource == Attr or both?
            id = ids[i];
        else
            id = i;
        
        ptoff = offsetStart + i;

        if (useInvisIds && invisIdsSet.contains(id))
            invisIdsAttr.set(ptoff, HUSD_Constants::getInvisible());
        
        if (useIds && idsAttr.isValid())
            idsAttr.set(ptoff, id);
        
        if (protoSource == PROTOSOURCEPRIMPATH)
            protoPathsAttr.set(ptoff, protoPaths[protoIndices[i]]);
        else if (protoSource == PROTOSOURCEATTRIBUTE)
            protoIndicesAttr.set(ptoff, protoIndices[i]);
        
    }
    
    return true;
}

bool HUSD_PointInstancer::copyUsdXformAttrsToGeoAttrs(HUSD_AutoAnyLock &lock,
                                            GU_Detail *gdp,
                                            const GA_Offset &offsetStart,
                                            const int &numPoints,
                                            const HUSD_TimeCode &timeCode,
                                            const UT_StringRef &primPath,
                                            bool applyPrimXform /*= false*/,
                                            bool copyPositions /*=true*/,
                                            bool copyOrientations /*=true*/,
                                            bool copyScales /*=true*/,
                                            bool copyAccelerations /*=true*/,
                                            bool copyVelocities /*=true*/,
                                            bool copyAngularVelocities /*=true*/,
                                            bool createUsdXformAttrib /*=true*/)
{
    GA_RWHandleT<UT_QuaternionH> orientAttr;
    GA_RWHandleT<UT_Vector3F>    scaleAttr;
    GA_RWHandleT<UT_Vector3F>    accelAttr;
    GA_RWHandleT<UT_Vector3F>    velAttr;
    GA_RWHandleT<UT_Vector3F>    angvelAttr;

    HUSD_GetAttributes           getAttrs(lock);

    UT_Matrix4D           usdXform;
    HUSD_Info             info(lock);
    usdXform = info.getWorldXform(primPath, timeCode);

    if (applyPrimXform)
    {
        HUSD_Info             info(lock);
        UT_Array<UT_Matrix4D> xforms;
        UT_Matrix4D           xform;
        UT_Matrix3D           xform3;
        UT_Vector3D           position;
        UT_QuaternionD        orientation;

        info.getPointInstancerXforms(primPath, xforms, timeCode);

        GA_RWHandleM4D usdXformAttr;
        if (createUsdXformAttrib)
        {
            usdXformAttr = gdp->findAttribute(GA_ATTRIB_POINT, theUsdXformAttr.asRef());
            if (!usdXformAttr.isValid())
            {
                usdXformAttr = gdp->getAttributes().createTupleAttribute(
                        GA_ATTRIB_POINT, GA_SCOPE_PUBLIC, theUsdXformAttr.asRef(),
                        GA_STORE_REAL64, 16, GA_Defaults(0.0));
            }
        }

        if (copyOrientations)
        {
            orientAttr = gdp->addFloatTuple(GA_ATTRIB_POINT, GA_Names::orient, 4);
            orientAttr->setTypeInfo(GA_TYPE_QUATERNION);
        }

        GA_Offset ptoff;
        for (int i = 0; i < numPoints; ++i)
        {
            ptoff = offsetStart + i;

            xform = xforms[i] * usdXform;
            xform3 = xform;

            if (copyPositions)
            {
                xform.getTranslates(position);
                gdp->setPos3(ptoff, position);
            }

            if (copyOrientations)
            {
                orientation.updateFromArbitraryMatrix(xform3);
                orientAttr.set(ptoff, orientation);
            }

            if (createUsdXformAttrib)
                usdXformAttr.set(ptoff, usdXform);
        }
    }
    else
    {
        UT_Array<UT_Vector3F>    positions;
        UT_Array<UT_QuaternionH> orientations;
        bool                     hasOrientations;

        if (copyPositions)
            getAttrs.getAttribute(primPath, UsdGeomTokens->positions.GetString(), positions, timeCode);
        if (copyOrientations)
            getAttrs.getAttribute(primPath, UsdGeomTokens->orientations.GetString(), orientations, timeCode);

        hasOrientations = !orientations.isEmpty();

        if (copyOrientations && hasOrientations)
        {
            orientAttr = gdp->addFloatTuple(GA_ATTRIB_POINT, GA_Names::orient, 4);
            orientAttr->setTypeInfo(GA_TYPE_QUATERNION);
        }

        GA_Offset ptoff;
        for (int i = 0; i < numPoints; ++i)
        {
            ptoff = offsetStart + i;
            if (copyPositions)
                gdp->setPos3(ptoff, positions[i]);
            if (copyOrientations && hasOrientations)
                orientAttr.set(ptoff, orientations[i]);
        }
    }

    bool                     hasScales;
    bool                     hasAccelerations;
    bool                     hasVelocities;
    bool                     hasAngularVelocities;
    UT_Array<UT_Vector3F>    scales;
    UT_Array<UT_Vector3F>    accelerations;
    UT_Array<UT_Vector3F>    velocities;
    UT_Array<UT_Vector3F>    angularVelocities;

    if (copyScales)
        getAttrs.getAttribute(primPath, UsdGeomTokens->scales.GetString(), scales, timeCode);
    if (copyAccelerations)
        getAttrs.getAttribute(primPath, UsdGeomTokens->accelerations.GetString(), accelerations, timeCode);
    if (copyVelocities)
        getAttrs.getAttribute(primPath, UsdGeomTokens->velocities.GetString(), velocities, timeCode);
    if (copyAngularVelocities)
        getAttrs.getAttribute(primPath, UsdGeomTokens->angularVelocities.GetString(), angularVelocities, timeCode);

    hasScales = !scales.isEmpty();
    hasAccelerations = !accelerations.isEmpty();
    hasVelocities = !velocities.isEmpty();
    hasAngularVelocities = !angularVelocities.isEmpty();

    if (copyScales && hasScales)
        scaleAttr = gdp->addFloatTuple(GA_ATTRIB_POINT, GA_Names::scale, 3);

    if (copyAccelerations && hasAccelerations)
        accelAttr = gdp->addFloatTuple(GA_ATTRIB_POINT, GA_Names::accel, 3);

    if (copyVelocities && hasVelocities)
        velAttr = gdp->addFloatTuple(GA_ATTRIB_POINT, GA_Names::v, 3);

    if (copyAngularVelocities && hasAngularVelocities)
        angvelAttr = gdp->addFloatTuple(GA_ATTRIB_POINT, GA_Names::w, 3);

    GA_Offset ptoff;
    for (int i = 0; i < numPoints; ++i)
    {
        ptoff = offsetStart + i;
        if (copyScales && hasScales)
            scaleAttr.set(ptoff, scales[i]);

        if (copyAccelerations && hasAccelerations)
        {
            if (applyPrimXform)
                accelAttr.set(ptoff, accelerations[i]*usdXform);
            else
                accelAttr.set(ptoff, accelerations[i]);
        }

        if (copyVelocities && hasVelocities)
            velAttr.set(ptoff, velocities[i]);

        if (copyAngularVelocities && hasAngularVelocities)
        {
            // angularVelocities is in degrees/s, but w is radians/s
            angularVelocities[i].degToRad();
            angvelAttr.set(ptoff, angularVelocities[i]);
        }
    }
    return true;
}

bool
HUSD_PointInstancer::copyUsdPrimvarsToGeoAttrs(HUSD_AutoAnyLock &lock,
                                 GU_Detail *gdp,
                                 const GA_Offset &offsetStart,
                                 const int &numPoints,
                                 const UT_StringHolder &primvarFilters,
                                 const HUSD_TimeCode &timeCode,
                                 const UT_StringRef &primPath)
{
    UT_StringHolder    attrName;
    VtValue            value;
    SdfValueTypeName   valueType;
    GA_TypeInfo        info;
    TfToken            role;

    auto               stage = lock.constData()->stage();
    auto               prim  = stage->GetPrimAtPath(HUSDgetSdfPath(primPath));
    UsdGeomPrimvarsAPI api(prim);

    for (UsdGeomPrimvar primvar : api.GetAuthoredPrimvars())
    {
        attrName = primvar.GetPrimvarName().GetString();
        if (!attrName.multiMatch(primvarFilters))
            continue;

        primvar.ComputeFlattened(&value, HUSDgetUsdTimeCode(timeCode));
        valueType = primvar.GetTypeName();

        info = GA_TYPE_VOID;
        role = valueType.GetRole();
        if (role == SdfValueRoleNames->Color)
            info = GA_TYPE_COLOR;
        else if (role == SdfValueRoleNames->Normal)
            info = GA_TYPE_NORMAL;
        else if (role == SdfValueRoleNames->Point)
            info = GA_TYPE_POINT;
        else if (role == SdfValueRoleNames->Vector)
            info = GA_TYPE_VECTOR;
        else if (role == SdfValueRoleNames->TextureCoordinate)
            info = GA_TYPE_TEXTURE_COORD;

        husdGetSopAttrName(attrName, attrName);

        // Floats
        if (_husdCopyToAttr<GfVec3f, UT_Vector3F, GA_STORE_REAL32>(
                    gdp, offsetStart, numPoints, attrName, value, info))
            continue;
        if (_husdCopyToAttr<GfVec2f, UT_Vector2F, GA_STORE_REAL32>(
                    gdp, offsetStart, numPoints, attrName, value, info))
            continue;
        if (_husdCopyToAttr<float, float, GA_STORE_REAL32>(
                    gdp, offsetStart, numPoints, attrName, value, info))
            continue;
        if (_husdCopyToAttr<GfVec4f, UT_Vector4F, GA_STORE_REAL32>(
                    gdp, offsetStart, numPoints, attrName, value, info))
            continue;
        if (_husdCopyToAttr<GfQuatf, UT_QuaternionF, GA_STORE_REAL32>(
                    gdp, offsetStart, numPoints, attrName, value, GA_TYPE_QUATERNION))
            continue;
        if (_husdCopyToAttr<GfMatrix2f, UT_Matrix2F, GA_STORE_REAL32>(
                    gdp, offsetStart, numPoints, attrName, value, info))
            continue;
        if (_husdCopyToAttr<GfMatrix3f, UT_Matrix3F, GA_STORE_REAL32>(
                    gdp, offsetStart, numPoints, attrName, value, info))
            continue;
        if (_husdCopyToAttr<GfMatrix4f, UT_Matrix4F, GA_STORE_REAL32>(
                    gdp, offsetStart, numPoints, attrName, value, info))
            continue;

        // Doubles
        if (_husdCopyToAttr<GfVec3d, UT_Vector3D, GA_STORE_REAL64>(
                    gdp, offsetStart, numPoints, attrName, value, info))
            continue;
        if (_husdCopyToAttr<GfVec2d, UT_Vector2D, GA_STORE_REAL64>(
                    gdp, offsetStart, numPoints, attrName, value, info))
            continue;
        if (_husdCopyToAttr<double, double, GA_STORE_REAL64>(
                    gdp, offsetStart, numPoints, attrName, value, info))
            continue;
        if (_husdCopyToAttr<GfVec4d, UT_Vector4D, GA_STORE_REAL64>(
                    gdp, offsetStart, numPoints, attrName, value, info))
            continue;
        if (_husdCopyToAttr<GfQuatd, UT_QuaternionD, GA_STORE_REAL64>(
                    gdp, offsetStart, numPoints, attrName, value, GA_TYPE_QUATERNION))
            continue;
        if (_husdCopyToAttr<GfMatrix2d, UT_Matrix2D, GA_STORE_REAL64>(
                    gdp, offsetStart, numPoints, attrName, value, info))
            continue;
        if (_husdCopyToAttr<GfMatrix3d, UT_Matrix3D, GA_STORE_REAL64>(
                    gdp, offsetStart, numPoints, attrName, value, info))
            continue;
        if (_husdCopyToAttr<GfMatrix4d, UT_Matrix4D, GA_STORE_REAL64>(
                    gdp, offsetStart, numPoints, attrName, value, info))
            continue;

        // Halfs
        if (_husdCopyToAttr<GfVec3h, UT_Vector3F, GA_STORE_REAL16>(
                    gdp, offsetStart, numPoints, attrName, value, info))
            continue;
        if (_husdCopyToAttr<GfVec2h, UT_Vector2F, GA_STORE_REAL16>(
                    gdp, offsetStart, numPoints, attrName, value, info))
            continue;
        if (_husdCopyToAttr<GfHalf, float, GA_STORE_REAL16>(
                    gdp, offsetStart, numPoints, attrName, value, info))
            continue;
        if (_husdCopyToAttr<GfVec4h, UT_Vector4F, GA_STORE_REAL16>(
                    gdp, offsetStart, numPoints, attrName, value, info))
            continue;
        if (_husdCopyToAttr<GfQuath, UT_QuaternionF, GA_STORE_REAL16>(
                    gdp, offsetStart, numPoints, attrName, value, GA_TYPE_QUATERNION))
            continue;

        // Ints
        if (_husdCopyToAttr<GfVec3i, UT_Vector3i, GA_STORE_INT32>(
                    gdp, offsetStart, numPoints, attrName, value, info))
            continue;
        if (_husdCopyToAttr<GfVec2i, UT_Vector2i, GA_STORE_INT32>(
                    gdp, offsetStart, numPoints, attrName, value, info))
            continue;
        if (_husdCopyToAttr<int, int, GA_STORE_INT32>(
                    gdp, offsetStart, numPoints, attrName, value, info))
            continue;
        if (_husdCopyToAttr<GfVec4i, UT_Vector4i, GA_STORE_INT32>(
                    gdp, offsetStart, numPoints, attrName, value, info))
            continue;
        if (_husdCopyToAttr<int64, int64, GA_STORE_INT64>(
                    gdp, offsetStart, numPoints, attrName, value, info))
            continue;

        // TODO: GA_RWHandleT<uint> not defined..
        // TODO: GA_RWHandleT<uint64> not defined..

        // Bool
        if (_husdCopyToAttr<bool, int, GA_STORE_INT8>(
                    gdp, offsetStart, numPoints, attrName, value, info))
            continue;

        // String // TODO: requires different type of GA_RWHandle, GA_RWHandleTHolder<UT_StringHolder>, so not compatible with current husdCopyToAttr
        // SdfValueTypeNames->Asset;
        // SdfValueTypeNames->String;                                   // VOID
        // SdfValueTypeNames->Token;                                    // VOID
    }
    return true;
}

bool
HUSD_PointInstancer::createBoundingBoxGeoAttr(HUSD_AutoAnyLock &lock,
                                           GU_Detail             *gdp,
                                           const GA_Offset       &offsetStart,
                                           const int             &numPoints,
                                           const HUSD_TimeCode   &timeCode,
                                           const UT_StringRef    &primPath,
                                           const UT_StringArray  &purposes,
                                           bool                  applyPrimXform /*= false*/)
{
    HUSD_Info       info(lock);
    GA_Offset       ptoffset;
    UT_BoundingBoxD boundingBox;

    GA_RWHandleF boundsAttr(gdp, GA_ATTRIB_POINT, "bounds");
    if (!boundsAttr.isValid())
    {
        boundsAttr = gdp->getAttributes().createTupleAttribute(
                     GA_ATTRIB_POINT, GA_SCOPE_PUBLIC, "bounds", GA_STORE_REAL32,
                     6, GA_Defaults(0.0));
    }

    // TODO: consider multithreading this
    for (int i = 0; i < numPoints; ++i)
    {
        ptoffset = offsetStart + i;
        boundingBox = info.getPointInstancerBounds(primPath, i, purposes, timeCode);
        if (applyPrimXform)
        {
            UT_Matrix4D xform = info.getWorldXform(primPath, timeCode);
            boundingBox.transform(xform);
        }

        int j = 0;
        for (fpreal64 d : boundingBox)
        {
            boundsAttr.set(ptoffset, j++, d);
        }
    }
    return true;
}
