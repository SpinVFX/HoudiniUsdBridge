//
// Copyright 2016 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
#include "./tokens.h"

PXR_NAMESPACE_OPEN_SCOPE

UsdHoudiniTokensType::UsdHoudiniTokensType() :
    barndoorbottom("barndoorbottom", TfToken::Immortal),
    barndoorbottomedge("barndoorbottomedge", TfToken::Immortal),
    barndoorleft("barndoorleft", TfToken::Immortal),
    barndoorleftedge("barndoorleftedge", TfToken::Immortal),
    barndoorright("barndoorright", TfToken::Immortal),
    barndoorrightedge("barndoorrightedge", TfToken::Immortal),
    barndoortop("barndoortop", TfToken::Immortal),
    barndoortopedge("barndoortopedge", TfToken::Immortal),
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
    HoudiniCameraPlateAPI("HoudiniCameraPlateAPI", TfToken::Immortal),
    HoudiniEditableAPI("HoudiniEditableAPI", TfToken::Immortal),
    HoudiniFieldAsset("HoudiniFieldAsset", TfToken::Immortal),
    HoudiniLayerInfo("HoudiniLayerInfo", TfToken::Immortal),
    HoudiniLightBarnDoorAPI("HoudiniLightBarnDoorAPI", TfToken::Immortal),
    HoudiniMetaCurves("HoudiniMetaCurves", TfToken::Immortal),
    HoudiniProceduralAPI("HoudiniProceduralAPI", TfToken::Immortal),
    HoudiniSelectableAPI("HoudiniSelectableAPI", TfToken::Immortal),
    HoudiniViewportGuideAPI("HoudiniViewportGuideAPI", TfToken::Immortal),
    HoudiniViewportLightAPI("HoudiniViewportLightAPI", TfToken::Immortal),
    allTokens({
        barndoorbottom,
        barndoorbottomedge,
        barndoorleft,
        barndoorleftedge,
        barndoorright,
        barndoorrightedge,
        barndoortop,
        barndoortopedge,
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
        HoudiniCameraPlateAPI,
        HoudiniEditableAPI,
        HoudiniFieldAsset,
        HoudiniLayerInfo,
        HoudiniLightBarnDoorAPI,
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
