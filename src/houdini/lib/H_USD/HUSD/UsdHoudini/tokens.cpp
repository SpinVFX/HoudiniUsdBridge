//
// Copyright 2016 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
#include "./tokens.h"

PXR_NAMESPACE_OPEN_SCOPE

UsdHoudiniTokensType::UsdHoudiniTokensType() :
    character("character", TfToken::Immortal),
    character_MultipleApplyTemplate_Binding("character:__INSTANCE_NAME__:binding", TfToken::Immortal),
    character_MultipleApplyTemplate_Rig("character:__INSTANCE_NAME__:rig", TfToken::Immortal),
    houdiniApexCharacterFiles("houdini:apex:character:files", TfToken::Immortal),
    houdiniApexCharacterRig("houdini:apex:character:rig", TfToken::Immortal),
    houdiniApexDeformJoints("houdini:apex:deform:joints", TfToken::Immortal),
    houdiniApexShape("houdini:apex:shape", TfToken::Immortal),
    houdiniApexShape_MultipleApplyTemplate_Binding("houdini:apex:shape:__INSTANCE_NAME__:binding", TfToken::Immortal),
    houdiniApexShape_MultipleApplyTemplate_Input("houdini:apex:shape:__INSTANCE_NAME__:input", TfToken::Immortal),
    houdiniApexShape_MultipleApplyTemplate_Output("houdini:apex:shape:__INSTANCE_NAME__:output", TfToken::Immortal),
    houdiniBackgroundimage("houdini:backgroundimage", TfToken::Immortal),
    houdiniClippingRange("houdini:clippingRange", TfToken::Immortal),
    houdiniEditable("houdini:editable", TfToken::Immortal),
    houdiniForegroundimage("houdini:foregroundimage", TfToken::Immortal),
    houdiniGuidescale("houdini:guidescale", TfToken::Immortal),
    houdiniInviewermenu("houdini:inviewermenu", TfToken::Immortal),
    houdiniProcedural("houdiniProcedural", TfToken::Immortal),
    houdiniProcedural_MultipleApplyTemplate_HoudiniActive("houdiniProcedural:__INSTANCE_NAME__:houdini:active", TfToken::Immortal),
    houdiniProcedural_MultipleApplyTemplate_HoudiniAnimated("houdiniProcedural:__INSTANCE_NAME__:houdini:animated", TfToken::Immortal),
    houdiniProcedural_MultipleApplyTemplate_HoudiniPriority("houdiniProcedural:__INSTANCE_NAME__:houdini:priority", TfToken::Immortal),
    houdiniProcedural_MultipleApplyTemplate_HoudiniProceduralArgs("houdiniProcedural:__INSTANCE_NAME__:houdini:procedural:args", TfToken::Immortal),
    houdiniProcedural_MultipleApplyTemplate_HoudiniProceduralPath("houdiniProcedural:__INSTANCE_NAME__:houdini:procedural:path", TfToken::Immortal),
    houdiniProcedural_MultipleApplyTemplate_HoudiniProceduralType("houdiniProcedural:__INSTANCE_NAME__:houdini:procedural:type", TfToken::Immortal),
    houdiniSelectable("houdini:selectable", TfToken::Immortal),
    inheritAnimationLayers("inheritAnimationLayers", TfToken::Immortal),
    primvarsHoudiniApexDeformJointIndices("primvars:houdini:apex:deform:jointIndices", TfToken::Immortal),
    primvarsHoudiniApexDeformJointWeights("primvars:houdini:apex:deform:jointWeights", TfToken::Immortal),
    sceneFiles("sceneFiles", TfToken::Immortal),
    HoudiniApexCharacterAPI("HoudiniApexCharacterAPI", TfToken::Immortal),
    HoudiniApexCharacterBindingAPI("HoudiniApexCharacterBindingAPI", TfToken::Immortal),
    HoudiniApexScene("HoudiniApexScene", TfToken::Immortal),
    HoudiniApexShapeBindingAPI("HoudiniApexShapeBindingAPI", TfToken::Immortal),
    HoudiniApexShapeDeformAPI("HoudiniApexShapeDeformAPI", TfToken::Immortal),
    HoudiniCameraPlateAPI("HoudiniCameraPlateAPI", TfToken::Immortal),
    HoudiniEditableAPI("HoudiniEditableAPI", TfToken::Immortal),
    HoudiniFieldAsset("HoudiniFieldAsset", TfToken::Immortal),
    HoudiniLayerInfo("HoudiniLayerInfo", TfToken::Immortal),
    HoudiniMetaCurves("HoudiniMetaCurves", TfToken::Immortal),
    HoudiniProceduralAPI("HoudiniProceduralAPI", TfToken::Immortal),
    HoudiniSelectableAPI("HoudiniSelectableAPI", TfToken::Immortal),
    HoudiniViewportGuideAPI("HoudiniViewportGuideAPI", TfToken::Immortal),
    HoudiniViewportLightAPI("HoudiniViewportLightAPI", TfToken::Immortal),
    allTokens({
        character,
        character_MultipleApplyTemplate_Binding,
        character_MultipleApplyTemplate_Rig,
        houdiniApexCharacterFiles,
        houdiniApexCharacterRig,
        houdiniApexDeformJoints,
        houdiniApexShape,
        houdiniApexShape_MultipleApplyTemplate_Binding,
        houdiniApexShape_MultipleApplyTemplate_Input,
        houdiniApexShape_MultipleApplyTemplate_Output,
        houdiniBackgroundimage,
        houdiniClippingRange,
        houdiniEditable,
        houdiniForegroundimage,
        houdiniGuidescale,
        houdiniInviewermenu,
        houdiniProcedural,
        houdiniProcedural_MultipleApplyTemplate_HoudiniActive,
        houdiniProcedural_MultipleApplyTemplate_HoudiniAnimated,
        houdiniProcedural_MultipleApplyTemplate_HoudiniPriority,
        houdiniProcedural_MultipleApplyTemplate_HoudiniProceduralArgs,
        houdiniProcedural_MultipleApplyTemplate_HoudiniProceduralPath,
        houdiniProcedural_MultipleApplyTemplate_HoudiniProceduralType,
        houdiniSelectable,
        inheritAnimationLayers,
        primvarsHoudiniApexDeformJointIndices,
        primvarsHoudiniApexDeformJointWeights,
        sceneFiles,
        HoudiniApexCharacterAPI,
        HoudiniApexCharacterBindingAPI,
        HoudiniApexScene,
        HoudiniApexShapeBindingAPI,
        HoudiniApexShapeDeformAPI,
        HoudiniCameraPlateAPI,
        HoudiniEditableAPI,
        HoudiniFieldAsset,
        HoudiniLayerInfo,
        HoudiniMetaCurves,
        HoudiniProceduralAPI,
        HoudiniSelectableAPI,
        HoudiniViewportGuideAPI,
        HoudiniViewportLightAPI
    })
{
}

TfStaticData<UsdHoudiniTokensType> UsdHoudiniTokens;

PXR_NAMESPACE_CLOSE_SCOPE
