//
// Copyright 2016 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
// GENERATED FILE.  DO NOT EDIT.
#include "./api.h"
#include "pxr/external/boost/python/class.hpp"
#include "./tokens.h"

PXR_NAMESPACE_USING_DIRECTIVE

namespace {

// Helper to return a static token as a string.  We wrap tokens as Python
// strings and for some reason simply wrapping the token using def_readonly
// bypasses to-Python conversion, leading to the error that there's no
// Python type for the C++ TfToken type.  So we wrap this functor instead.
class _WrapStaticToken {
public:
    static std::string Wrap(const TfToken* token) {
        return token->GetString();
    }
};

template <typename T>
void
_AddToken(T& cls, const char* name, const TfToken& token)
{
    cls.add_static_property(name,
                            pxr_boost::python::make_function(
                                &_WrapStaticToken::Wrap,
                                pxr_boost::python::return_value_policy<
                                    pxr_boost::python::return_by_value>()));
}

} // anonymous

void wrapUsdHoudiniTokens()
{
    pxr_boost::python::class_<UsdHoudiniTokensType, pxr_boost::python::noncopyable>
        cls("Tokens", pxr_boost::python::no_init);
    _AddToken(cls, "character", UsdHoudiniTokens->character);
    _AddToken(cls, "character_MultipleApplyTemplate_Binding", UsdHoudiniTokens->character_MultipleApplyTemplate_Binding);
    _AddToken(cls, "character_MultipleApplyTemplate_Rig", UsdHoudiniTokens->character_MultipleApplyTemplate_Rig);
    _AddToken(cls, "houdiniApexCharacterFiles", UsdHoudiniTokens->houdiniApexCharacterFiles);
    _AddToken(cls, "houdiniApexCharacterRig", UsdHoudiniTokens->houdiniApexCharacterRig);
    _AddToken(cls, "houdiniApexDeformJoints", UsdHoudiniTokens->houdiniApexDeformJoints);
    _AddToken(cls, "houdiniApexShape", UsdHoudiniTokens->houdiniApexShape);
    _AddToken(cls, "houdiniApexShape_MultipleApplyTemplate_Binding", UsdHoudiniTokens->houdiniApexShape_MultipleApplyTemplate_Binding);
    _AddToken(cls, "houdiniApexShape_MultipleApplyTemplate_Input", UsdHoudiniTokens->houdiniApexShape_MultipleApplyTemplate_Input);
    _AddToken(cls, "houdiniApexShape_MultipleApplyTemplate_Output", UsdHoudiniTokens->houdiniApexShape_MultipleApplyTemplate_Output);
    _AddToken(cls, "houdiniBackgroundimage", UsdHoudiniTokens->houdiniBackgroundimage);
    _AddToken(cls, "houdiniClippingRange", UsdHoudiniTokens->houdiniClippingRange);
    _AddToken(cls, "houdiniEditable", UsdHoudiniTokens->houdiniEditable);
    _AddToken(cls, "houdiniForegroundimage", UsdHoudiniTokens->houdiniForegroundimage);
    _AddToken(cls, "houdiniGuidescale", UsdHoudiniTokens->houdiniGuidescale);
    _AddToken(cls, "houdiniInviewermenu", UsdHoudiniTokens->houdiniInviewermenu);
    _AddToken(cls, "houdiniProcedural", UsdHoudiniTokens->houdiniProcedural);
    _AddToken(cls, "houdiniProcedural_MultipleApplyTemplate_HoudiniActive", UsdHoudiniTokens->houdiniProcedural_MultipleApplyTemplate_HoudiniActive);
    _AddToken(cls, "houdiniProcedural_MultipleApplyTemplate_HoudiniAnimated", UsdHoudiniTokens->houdiniProcedural_MultipleApplyTemplate_HoudiniAnimated);
    _AddToken(cls, "houdiniProcedural_MultipleApplyTemplate_HoudiniPriority", UsdHoudiniTokens->houdiniProcedural_MultipleApplyTemplate_HoudiniPriority);
    _AddToken(cls, "houdiniProcedural_MultipleApplyTemplate_HoudiniProceduralArgs", UsdHoudiniTokens->houdiniProcedural_MultipleApplyTemplate_HoudiniProceduralArgs);
    _AddToken(cls, "houdiniProcedural_MultipleApplyTemplate_HoudiniProceduralPath", UsdHoudiniTokens->houdiniProcedural_MultipleApplyTemplate_HoudiniProceduralPath);
    _AddToken(cls, "houdiniProcedural_MultipleApplyTemplate_HoudiniProceduralType", UsdHoudiniTokens->houdiniProcedural_MultipleApplyTemplate_HoudiniProceduralType);
    _AddToken(cls, "houdiniSelectable", UsdHoudiniTokens->houdiniSelectable);
    _AddToken(cls, "inheritAnimationLayers", UsdHoudiniTokens->inheritAnimationLayers);
    _AddToken(cls, "primvarsHoudiniApexDeformJointIndices", UsdHoudiniTokens->primvarsHoudiniApexDeformJointIndices);
    _AddToken(cls, "primvarsHoudiniApexDeformJointWeights", UsdHoudiniTokens->primvarsHoudiniApexDeformJointWeights);
    _AddToken(cls, "sceneFiles", UsdHoudiniTokens->sceneFiles);
    _AddToken(cls, "HoudiniApexCharacterAPI", UsdHoudiniTokens->HoudiniApexCharacterAPI);
    _AddToken(cls, "HoudiniApexCharacterBindingAPI", UsdHoudiniTokens->HoudiniApexCharacterBindingAPI);
    _AddToken(cls, "HoudiniApexScene", UsdHoudiniTokens->HoudiniApexScene);
    _AddToken(cls, "HoudiniApexShapeBindingAPI", UsdHoudiniTokens->HoudiniApexShapeBindingAPI);
    _AddToken(cls, "HoudiniApexShapeDeformAPI", UsdHoudiniTokens->HoudiniApexShapeDeformAPI);
    _AddToken(cls, "HoudiniCameraPlateAPI", UsdHoudiniTokens->HoudiniCameraPlateAPI);
    _AddToken(cls, "HoudiniEditableAPI", UsdHoudiniTokens->HoudiniEditableAPI);
    _AddToken(cls, "HoudiniFieldAsset", UsdHoudiniTokens->HoudiniFieldAsset);
    _AddToken(cls, "HoudiniLayerInfo", UsdHoudiniTokens->HoudiniLayerInfo);
    _AddToken(cls, "HoudiniMetaCurves", UsdHoudiniTokens->HoudiniMetaCurves);
    _AddToken(cls, "HoudiniProceduralAPI", UsdHoudiniTokens->HoudiniProceduralAPI);
    _AddToken(cls, "HoudiniSelectableAPI", UsdHoudiniTokens->HoudiniSelectableAPI);
    _AddToken(cls, "HoudiniViewportGuideAPI", UsdHoudiniTokens->HoudiniViewportGuideAPI);
    _AddToken(cls, "HoudiniViewportLightAPI", UsdHoudiniTokens->HoudiniViewportLightAPI);
}
