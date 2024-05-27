/*
* Copyright 2019 Side Effects Software Inc.
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
 */
#include "HD_HoudiniFieldAssetAdapter.h"
#include <HUSD/UsdHoudini/houdiniFieldAsset.h>
#include <HUSD/UsdHoudini/tokens.h>
#include <HUSD/XUSD_Tokens.h>

#include "pxr/usd/usdVol/tokens.h"
#include "pxr/usdImaging/usdVolImaging/dataSourceFieldAsset.h"

#include "pxr/base/tf/type.h"

PXR_NAMESPACE_OPEN_SCOPE

TF_REGISTRY_FUNCTION_WITH_TAG(TfType, USD_HD_HoudiniFieldAssetAdapter)
{
    typedef HD_HoudiniFieldAssetAdapter Adapter;
    TfType t = TfType::Define<Adapter, TfType::Bases<Adapter::BaseAdapter> >();
    t.SetFactory< UsdImagingPrimAdapterFactory<Adapter> >();
}

HD_HoudiniFieldAssetAdapter::~HD_HoudiniFieldAssetAdapter() = default;

TfTokenVector
HD_HoudiniFieldAssetAdapter::GetImagingSubprims(UsdPrim const& prim)
{
    return { TfToken() };
}

TfToken
HD_HoudiniFieldAssetAdapter::GetImagingSubprimType(
        UsdPrim const& prim,
        TfToken const& subprim)
{
    if (subprim.IsEmpty()) {
        return HusdHdPrimTypeTokens->houdiniFieldAsset;
    }
    return TfToken();
}

HdContainerDataSourceHandle
HD_HoudiniFieldAssetAdapter::GetImagingSubprimData(
        UsdPrim const& prim,
        TfToken const& subprim,
        const UsdImagingDataSourceStageGlobals &stageGlobals)
{
    if (subprim.IsEmpty()) {
        return UsdImagingDataSourceFieldAssetPrim::New(
            prim.GetPath(),
            prim,
            stageGlobals);
    }
    return nullptr;
}

VtValue
HD_HoudiniFieldAssetAdapter::Get(
    UsdPrim const& prim,
    SdfPath const& cachePath,
    TfToken const& key,
    UsdTimeCode time,
    VtIntArray *outIndices) const
{
    if ( key == UsdVolTokens->filePath ||
         key == UsdVolTokens->fieldName ||
         key == UsdVolTokens->fieldIndex ||
         key == UsdVolTokens->fieldDataType ||
         key == UsdVolTokens->vectorDataRoleHint ||
         key == UsdVolTokens->fieldClass) {
        
        if (UsdAttribute const &attr = prim.GetAttribute(key)) {
            VtValue value;
            if (attr.Get(&value, time)) {
                return value;
            }
        }

        if (key == UsdVolTokens->filePath) {
            return VtValue(SdfAssetPath());
        }
        if (key == UsdVolTokens->fieldIndex) {
            constexpr int def = 0;
            return VtValue(def);
        }

        return VtValue(TfToken());
    }
    
    return
        BaseAdapter::Get(
            prim,
            cachePath,
            key,
            time,
            outIndices);
}

TfToken
HD_HoudiniFieldAssetAdapter::GetPrimTypeToken() const
{
    return HusdHdPrimTypeTokens->houdiniFieldAsset;
}

PXR_NAMESPACE_CLOSE_SCOPE
