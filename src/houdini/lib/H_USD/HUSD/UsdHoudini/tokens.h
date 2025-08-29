//
// Copyright 2016 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
#ifndef USDHOUDINI_TOKENS_H
#define USDHOUDINI_TOKENS_H

/// \file usdHoudini/tokens.h

// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
// 
// This is an automatically generated file (by usdGenSchema.py).
// Do not hand-edit!
// 
// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

#include "pxr/pxr.h"
#include "./api.h"
#include "pxr/base/tf/staticData.h"
#include "pxr/base/tf/token.h"
#include <vector>

PXR_NAMESPACE_OPEN_SCOPE


/// \class UsdHoudiniTokensType
///
/// \link UsdHoudiniTokens \endlink provides static, efficient
/// \link TfToken TfTokens\endlink for use in all public USD API.
///
/// These tokens are auto-generated from the module's schema, representing
/// property names, for when you need to fetch an attribute or relationship
/// directly by name, e.g. UsdPrim::GetAttribute(), in the most efficient
/// manner, and allow the compiler to verify that you spelled the name
/// correctly.
///
/// UsdHoudiniTokens also contains all of the \em allowedTokens values
/// declared for schema builtin attributes of 'token' scene description type.
/// Use UsdHoudiniTokens like so:
///
/// \code
///     gprim.GetMyTokenValuedAttr().Set(UsdHoudiniTokens->barndoorbottom);
/// \endcode
struct UsdHoudiniTokensType {
    USDHOUDINI_API UsdHoudiniTokensType();
    /// \brief "barndoorbottom"
    /// 
    /// UsdHoudiniHoudiniLightBarnDoorAPI
    const TfToken barndoorbottom;
    /// \brief "barndoorbottomedge"
    /// 
    /// UsdHoudiniHoudiniLightBarnDoorAPI
    const TfToken barndoorbottomedge;
    /// \brief "barndoorleft"
    /// 
    /// UsdHoudiniHoudiniLightBarnDoorAPI
    const TfToken barndoorleft;
    /// \brief "barndoorleftedge"
    /// 
    /// UsdHoudiniHoudiniLightBarnDoorAPI
    const TfToken barndoorleftedge;
    /// \brief "barndoorright"
    /// 
    /// UsdHoudiniHoudiniLightBarnDoorAPI
    const TfToken barndoorright;
    /// \brief "barndoorrightedge"
    /// 
    /// UsdHoudiniHoudiniLightBarnDoorAPI
    const TfToken barndoorrightedge;
    /// \brief "barndoortop"
    /// 
    /// UsdHoudiniHoudiniLightBarnDoorAPI
    const TfToken barndoortop;
    /// \brief "barndoortopedge"
    /// 
    /// UsdHoudiniHoudiniLightBarnDoorAPI
    const TfToken barndoortopedge;
    /// \brief "houdini:backgroundimage"
    /// 
    /// UsdHoudiniHoudiniCameraPlateAPI
    const TfToken houdiniBackgroundimage;
    /// \brief "houdini:clippingRange"
    /// 
    /// UsdHoudiniHoudiniViewportLightAPI
    const TfToken houdiniClippingRange;
    /// \brief "houdini:editable"
    /// 
    /// UsdHoudiniHoudiniEditableAPI
    const TfToken houdiniEditable;
    /// \brief "houdini:foregroundimage"
    /// 
    /// UsdHoudiniHoudiniCameraPlateAPI
    const TfToken houdiniForegroundimage;
    /// \brief "houdini:guidescale"
    /// 
    /// UsdHoudiniHoudiniViewportGuideAPI
    const TfToken houdiniGuidescale;
    /// \brief "houdini:inviewermenu"
    /// 
    /// UsdHoudiniHoudiniViewportGuideAPI
    const TfToken houdiniInviewermenu;
    /// \brief "houdiniProcedural"
    /// 
    /// Property namespace prefix for the UsdHoudiniHoudiniProceduralAPI schema.
    const TfToken houdiniProcedural;
    /// \brief "houdiniProcedural:__INSTANCE_NAME__:houdini:active"
    /// 
    /// UsdHoudiniHoudiniProceduralAPI
    const TfToken houdiniProcedural_MultipleApplyTemplate_HoudiniActive;
    /// \brief "houdiniProcedural:__INSTANCE_NAME__:houdini:animated"
    /// 
    /// UsdHoudiniHoudiniProceduralAPI
    const TfToken houdiniProcedural_MultipleApplyTemplate_HoudiniAnimated;
    /// \brief "houdiniProcedural:__INSTANCE_NAME__:houdini:priority"
    /// 
    /// UsdHoudiniHoudiniProceduralAPI
    const TfToken houdiniProcedural_MultipleApplyTemplate_HoudiniPriority;
    /// \brief "houdiniProcedural:__INSTANCE_NAME__:houdini:procedural:args"
    /// 
    /// UsdHoudiniHoudiniProceduralAPI
    const TfToken houdiniProcedural_MultipleApplyTemplate_HoudiniProceduralArgs;
    /// \brief "houdiniProcedural:__INSTANCE_NAME__:houdini:procedural:path"
    /// 
    /// UsdHoudiniHoudiniProceduralAPI
    const TfToken houdiniProcedural_MultipleApplyTemplate_HoudiniProceduralPath;
    /// \brief "houdiniProcedural:__INSTANCE_NAME__:houdini:procedural:type"
    /// 
    /// UsdHoudiniHoudiniProceduralAPI
    const TfToken houdiniProcedural_MultipleApplyTemplate_HoudiniProceduralType;
    /// \brief "houdini:selectable"
    /// 
    /// UsdHoudiniHoudiniSelectableAPI
    const TfToken houdiniSelectable;
    /// \brief "HoudiniCameraPlateAPI"
    /// 
    /// Schema identifer and family for UsdHoudiniHoudiniCameraPlateAPI
    const TfToken HoudiniCameraPlateAPI;
    /// \brief "HoudiniEditableAPI"
    /// 
    /// Schema identifer and family for UsdHoudiniHoudiniEditableAPI
    const TfToken HoudiniEditableAPI;
    /// \brief "HoudiniFieldAsset"
    /// 
    /// Schema identifer and family for UsdHoudiniHoudiniFieldAsset
    const TfToken HoudiniFieldAsset;
    /// \brief "HoudiniLayerInfo"
    /// 
    /// Schema identifer and family for UsdHoudiniHoudiniLayerInfo
    const TfToken HoudiniLayerInfo;
    /// \brief "HoudiniLightBarnDoorAPI"
    /// 
    /// Schema identifer and family for UsdHoudiniHoudiniLightBarnDoorAPI
    const TfToken HoudiniLightBarnDoorAPI;
    /// \brief "HoudiniMetaCurves"
    /// 
    /// Schema identifer and family for UsdHoudiniHoudiniMetaCurves
    const TfToken HoudiniMetaCurves;
    /// \brief "HoudiniProceduralAPI"
    /// 
    /// Schema identifer and family for UsdHoudiniHoudiniProceduralAPI
    const TfToken HoudiniProceduralAPI;
    /// \brief "HoudiniSelectableAPI"
    /// 
    /// Schema identifer and family for UsdHoudiniHoudiniSelectableAPI
    const TfToken HoudiniSelectableAPI;
    /// \brief "HoudiniViewportGuideAPI"
    /// 
    /// Schema identifer and family for UsdHoudiniHoudiniViewportGuideAPI
    const TfToken HoudiniViewportGuideAPI;
    /// \brief "HoudiniViewportLightAPI"
    /// 
    /// Schema identifer and family for UsdHoudiniHoudiniViewportLightAPI
    const TfToken HoudiniViewportLightAPI;
    /// A vector of all of the tokens listed above.
    const std::vector<TfToken> allTokens;
};

/// \var UsdHoudiniTokens
///
/// A global variable with static, efficient \link TfToken TfTokens\endlink
/// for use in all public USD API.  \sa UsdHoudiniTokensType
extern USDHOUDINI_API TfStaticData<UsdHoudiniTokensType> UsdHoudiniTokens;

PXR_NAMESPACE_CLOSE_SCOPE

#endif
