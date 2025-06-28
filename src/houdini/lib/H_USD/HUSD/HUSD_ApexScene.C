/*
 * Copyright 2025 Side Effects Software Inc.
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

#include "HUSD_ApexScene.h"

#include "HUSD_DataHandle.h"
#include "HUSD_ErrorScope.h"
#include "HUSD_FindPrims.h"
#include "HUSD_GeoUtils.h"
#include "HUSD_PathSet.h"
#include "HUSD_SetAttributes.h"
#include "HUSD_TimeCode.h"
#include "XUSD_Data.h"
#include "XUSD_Format.h" // IWYU pragma: keep (for UTdebugPrint)
#include "XUSD_Utils.h"
#include "XUSD_LockedGeoRegistry.h"

#include "UsdHoudini/houdiniApexCharacterAPI.h"
#include "UsdHoudini/houdiniApexCharacterBindingAPI.h"
#include "UsdHoudini/houdiniApexScene.h"
#include "UsdHoudini/houdiniApexShapeBindingAPI.h"

#include <APEXA/APEXA_SceneInvoke.h>
#include <APEXA/APEXA_SceneUtils.h>
#include <GA/GA_AttributeFilter.h>
#include <GA/GA_ElementWrangler.h>
#include <GU/GU_Detail.h>
#include <GU/GU_PackedFolders.h>
#include <OP/OP_Node.h>
#include <UT/UT_Debug.h>
#include <UT/UT_Map.h>
#include <UT/UT_Tracing.h>
#include <UT/UT_WorkBuffer.h>
#include <gusd/GU_USD.h>
#include <gusd/agentUtils.h>

#include <pxr/base/tf/pathUtils.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usdGeom/pointBased.h>
#include <pxr/usd/usdSkel/bindingAPI.h>
#include <pxr/usd/usdSkel/skinningQuery.h>

static constexpr UT_StringLit theOutputName("output");

PXR_NAMESPACE_USING_DIRECTIVE

namespace
{
/// Stores information about a shape binding (from HoudiniApexShapeBindingAPI).
struct husdApexShapeBindingData
{
    /// Path to a primitive which is bound to a rig input or output.
    HUSD_Path myPrimPath;
    /// Name of the rig input to bind the primitive to.
    UT_StringHolder myInputName;
    /// Name of the rig output which affects the primitive.
    UT_StringHolder myOutputName;
};

/// Represents a character (from HoudiniApexCharacterAPI) and its shapes.
struct husdApexCharacterData
{
    explicit husdApexCharacterData(const UT_StringRef &char_name)
    {
        myScenePath.format("/{0}.char", char_name);
    }

    /// List of shape bindings applied via HoudiniApexShapeBindingAPI.
    UT_Array<husdApexShapeBindingData> myShapeBindings;
    /// Resolved paths of .bgeo file(s) to load the character's rig, skeleton,
    /// etc from.
    VtArray<SdfAssetPath> myFiles;
    /// Optionally select which of the character's rigs to use.
    UT_StringHolder myRigName;
    /// APEX scene path of the character, e.g. "/electra.char"
    UT_StringHolder myScenePath;
};

/// Data associated with a HoudiniApexScene prim and its characters.
struct husdApexSceneData
{
    /// Assembles the APEX scene input geometry (packed folders).
    GU_DetailHandle loadSceneGeometry(const HUSD_AutoAnyLock &read_lock) const;

    /// Assemble the APEX scene geometry (packed folders) from the files
    /// referenced by the characters and the scene prim.
    void loadFiles(GU_DetailHandle &scene_gdh) const;

    /// Convert USD prim inputs to Houdini geometry and insert into the APEX
    /// scene geometry.
    void loadGeoInputsFromPrims(
            const HUSD_AutoAnyLock &read_lock,
            GU_DetailHandle &scene_gdh) const;

    // Add a dictionary attribute describing which USD prims are bound to rig
    // outputs, for use by the Animate state.
    void recordOutputBindings(GU_DetailHandle &scene_gdh) const;

    /// List of characters which are referenced by the scene.
    UT_Array<husdApexCharacterData> myCharacters;
    /// Resolved paths of .bgeo file(s) to load animation, etc from.
    VtArray<SdfAssetPath> myFiles;
    /// The "Inherit Animation Layers from Input" setting for each scene delta.
    VtArray<bool> myInheritLayers;
};

/// Collect a list of the APEX scenes (and their characters / shapes), and
/// perform some validation (e.g. a prim can't be affected by multiple APEX
/// scenes).
static UT_Array<husdApexSceneData>
husdFindScenes(
        const HUSD_AutoAnyLock &read_lock,
        const HUSD_PathSet &scene_prim_paths)
{
    utZoneScopedN("Finding APEX scenes on stage");

    UT_ASSERT(read_lock.isStageValid());
    UsdStageRefPtr stage = read_lock.constData()->stage();

    // Map from output prim paths to their scene prim path, to verify that a
    // prim is not driven by multiple outputs.
    UT_Map<HUSD_Path, HUSD_Path> known_prim_outputs;

    UT_Array<husdApexSceneData> scenes;

    for (const HUSD_Path &scene_prim_path : scene_prim_paths)
    {
        husdApexSceneData scene;

        auto scene_prim = UsdHoudiniHoudiniApexScene::Get(
                stage, scene_prim_path.sdfPath());
        if (!scene_prim)
        {
            UT_WorkBuffer msg;
            msg.format(
                    "Primitive '{0}' is not a {1}", scene_prim_path.sdfPath(),
                    UsdHoudiniTokens->HoudiniApexScene);
            HUSD_ErrorScope::addWarning(HUSD_ERR_STRING, msg.buffer());
            continue;
        }

        scene_prim.GetSceneFilesAttr().Get(&scene.myFiles);
        scene_prim.GetInheritAnimationLayersAttr().Get(&scene.myInheritLayers);

        if (scene.myInheritLayers.size() != scene.myFiles.size())
        {
            UT_WorkBuffer msg;
            msg.format(
                    "Invalid number of values for '{0}' attribute on primitive "
                    "'{1}', expected {2} but got {3}",
                    UsdHoudiniTokens->inheritAnimationLayers,
                    scene_prim_path.sdfPath(), scene.myFiles.size(),
                    scene.myInheritLayers.size());
            HUSD_ErrorScope::addWarning(HUSD_ERR_STRING, msg.buffer());
            continue;
        }

        // Record all of the characters bound to this scene.
        // The referenced prim must have the HoudiniApexCharacterAPI applied.
        for (const UsdHoudiniHoudiniApexCharacterBindingAPI &char_binding :
             UsdHoudiniHoudiniApexCharacterBindingAPI::GetAll(
                     scene_prim.GetPrim()))
        {
            husdApexCharacterData character(char_binding.GetName().GetString());

            SdfPathVector char_targets;
            char_binding.GetBindingRel().GetForwardedTargets(&char_targets);
            if (char_targets.size() != 1 || !char_targets.front().IsPrimPath())
                continue;

            SdfPath char_prim_path = char_targets.front().GetPrimPath();
            auto char_api = UsdHoudiniHoudiniApexCharacterAPI::Get(
                    stage, char_prim_path);
            if (!char_api)
            {
                UT_WorkBuffer msg;
                msg.format(
                        "Primitive '{0}' is missing required {1} schema",
                        char_prim_path,
                        UsdHoudiniTokens->HoudiniApexCharacterAPI);
                HUSD_ErrorScope::addWarning(HUSD_ERR_STRING, msg.buffer());
                continue;
            }

            char_api.GetFilesAttr().Get(&character.myFiles);

            // The CharacterBindingAPI takes precedence for the rig selection.
            std::string rig_name;
            char_binding.GetRigAttr().Get(&rig_name);
            if (rig_name.empty())
                char_api.GetRigAttr().Get(&rig_name);

            character.myRigName = rig_name;

            for (const UsdHoudiniHoudiniApexShapeBindingAPI &shape_binding_api :
                 UsdHoudiniHoudiniApexShapeBindingAPI::GetAll(
                         char_api.GetPrim()))
            {
                SdfPathVector shape_targets;
                shape_binding_api.GetBindingRel().GetTargets(&shape_targets);

                if (shape_targets.size() != 1
                    || !shape_targets.front().IsPrimPath())
                {
                    continue;
                }

                SdfPath shape_prim_path = shape_targets.front().GetPrimPath();
                UsdPrim shape_prim = stage->GetPrimAtPath(shape_prim_path);
                if (!shape_prim)
                    continue;

                // Currently we only support adjusting point-based prims by
                // writing back deformed positions and normals.
                if (!UsdGeomPointBased(shape_prim))
                {
                    UT_WorkBuffer msg;
                    msg.format(
                            "Primitive '{0}' is not a UsdGeomPointBased, "
                            "skipping.",
                            shape_prim_path);
                    HUSD_ErrorScope::addWarning(HUSD_ERR_STRING, msg.buffer());
                    continue;
                }

                // We can't edit instance proxies.
                if (shape_prim.IsInstanceProxy())
                {
                    HUSD_ErrorScope::addWarning(
                            HUSD_ERR_IGNORING_INSTANCE_PROXY,
                            shape_prim_path.GetText());
                    continue;
                }

                husdApexShapeBindingData shape_binding;
                shape_binding.myPrimPath = shape_prim_path;

                std::string rig_input;
                shape_binding_api.GetInputAttr().Get(&rig_input);
                shape_binding.myInputName = rig_input;

                std::string rig_output;
                shape_binding_api.GetOutputAttr().Get(&rig_output);
                shape_binding.myOutputName = rig_output;

                // Verify that the shape is only driven by one output.
                if (shape_binding.myOutputName)
                {
                    if (auto it = known_prim_outputs.find(shape_prim_path);
                        it != known_prim_outputs.end())
                    {
                        UT_WorkBuffer msg;
                        msg.format(
                                "Primitive '{0}' is affected by multiple "
                                "outputs (scenes '{1}' and '{2}'). Scene '{1}' "
                                "will be used.",
                                shape_prim_path, it->second.sdfPath(),
                                scene_prim_path.sdfPath());
                        HUSD_ErrorScope::addWarning(
                                HUSD_ERR_STRING, msg.buffer());
                        continue;
                    }

                    known_prim_outputs.emplace(
                            shape_prim_path, scene_prim_path);
                }

                character.myShapeBindings.append(std::move(shape_binding));
            }

            scene.myCharacters.append(std::move(character));
        }

        scenes.append(std::move(scene));
    }

    return scenes;
}

/// Loads geometry from an asset path, including locked geometry from a SOP node
/// (e.g. 'op:/path/to/node.sop') similar to SOP Import.
GU_ConstDetailHandle
husdLoadGeometryAsset(const SdfAssetPath &asset_path)
{
    std::string resolved_path = asset_path.GetResolvedPath();
    if (resolved_path.empty())
        resolved_path = asset_path.GetAssetPath();

    std::string node_path;
    SdfLayer::FileFormatArguments args;
    if (SdfLayer::SplitIdentifier(resolved_path, &node_path, &args)
        && TfGetExtension(node_path) == "sop")
    {
        GU_ConstDetailHandle gdh = XUSD_LockedGeoRegistry::getGeometry(
                node_path, args);
        if (!gdh.isValid())
        {
            UT_WorkBuffer msg;
            msg.format(
                    "Failed to get locked geometry from '{0}'", resolved_path);
            HUSD_ErrorScope::addWarning(HUSD_ERR_STRING, msg.buffer());
        }

        return gdh;
    }

    return HUSDloadGeometryFromAsset(resolved_path);
}

/// Selects the default rig for a character.
/// For now, this just matches APEXA_Scene::loadCharacterFromGeometry() and
/// picks the last file matching *.rig
static UT_StringHolder
husdGetDefaultRig(
        const GU_ConstDetailHandle &scene_gdh,
        const UT_StringRef &character_path)
{
    UT_WorkBuffer pattern;
    pattern.format("{0}/*.rig", character_path);

    GU_PackedFoldersRO scene_folders(scene_gdh);
    UT_Array<const GU_PackedFoldersRO::FileInfo *> rig_files;
    UT_StringHolder error;
    if (!scene_folders.findFilesByPattern(
                rig_files, pattern, /*sorted=*/true, error)
        || rig_files.isEmpty())
    {
        return UT_StringHolder::theEmptyString;
    }

    return UTstringFileName(rig_files.last()->path());
}

void
husdApexSceneData::recordOutputBindings(GU_DetailHandle &scene_gdh) const
{
    static constexpr UT_StringLit theOutputPathName("output_path");
    static constexpr UT_StringLit theOutputKeyName("output_key");
    static constexpr UT_StringLit theRigPatternName("rig_pattern");

    auto output_bindings = UTmakeUnique<UT_Options>();

    for (const husdApexCharacterData &character : myCharacters)
    {
        UT_StringHolder rig_name = character.myRigName;
        if (!rig_name)
            rig_name = husdGetDefaultRig(scene_gdh, character.myScenePath);

        if (!rig_name)
            continue;

        UT_StringHolder rig_output_path;
        rig_output_path.format(
                "{0}/{1}/{2}", character.myScenePath, rig_name,
                theOutputName.asRef());

        for (const husdApexShapeBindingData &binding :
             character.myShapeBindings)
        {
            if (!binding.myOutputName)
                continue;

            UT_StringHolder rig_pattern;
            rig_pattern.format(
                    "{0}:{1}", theOutputName.asRef(), binding.myOutputName);

            auto output_info = UTmakeUnique<UT_Options>();
            output_info->setOptionS(
                    theOutputPathName.asHolder(), rig_output_path);
            output_info->setOptionS(
                    theOutputKeyName.asHolder(), binding.myOutputName);
            output_info->setOptionS(theRigPatternName.asHolder(), rig_pattern);

            output_bindings->setOptionDict(
                    binding.myPrimPath.pathStr(),
                    UT_OptionsHolder(std::move(output_info)));
        }
    }


    // Add a dictionary attribute describing the USD prims bound to rig outputs,
    // for use by the Animate state.
    GA_RWHandleDict output_bindings_attrib = scene_gdh.gdpNC()->addDictTuple(
            GA_ATTRIB_DETAIL, "usdoutputbindings"_UTsh, 1);
    output_bindings_attrib.set(
            GA_DETAIL_OFFSET, UT_OptionsHolder(std::move(output_bindings)));
}

GU_DetailHandle
husdApexSceneData::loadSceneGeometry(const HUSD_AutoAnyLock &read_lock) const
{
    utZoneScopedN("husdApexSceneData load scene geometry");

    GU_DetailHandle scene_gdh;
    scene_gdh.allocateAndSet(new GU_Detail(), /*own=*/true);

    // Assemble the input geometry.
    loadFiles(scene_gdh);
    loadGeoInputsFromPrims(read_lock, scene_gdh);

    recordOutputBindings(scene_gdh);

    return scene_gdh;
}

void
husdApexSceneData::loadFiles(GU_DetailHandle &scene_gdh) const
{
    utZoneScopedN("husdApexSceneData load files");

    GU_PackedFolders scene_folders(scene_gdh);

    // Load the characters' files first, under each character's path.
    // The scene asset are merged later since they are allowed to override the
    // character elements.
    for (const husdApexCharacterData &character : myCharacters)
    {
        GU_DetailHandle char_gdh;
        char_gdh.allocateAndSet(new GU_Detail(), /*own=*/true);

        // Merge the packed folders from the character's files.
        for (const SdfAssetPath &asset_path : character.myFiles)
        {
            GU_ConstDetailHandle gdh = husdLoadGeometryAsset(asset_path);
            if (!gdh.isValid())
                continue;

            APEXAmergeSceneDelta(
                    char_gdh, gdh, /*inherit_animation_layers=*/false);
        }

        // Insert under the character's path, e.g. '/electra.char'.
        scene_folders.insert(
                character.myScenePath, char_gdh, /*pack=*/true,
                /*is_folder=*/true, /*is_visible=*/true);
    }

    // Next, load the scene files at the root of the hierarchy.
    for (exint i = 0, n = myFiles.size(); i < n; ++i)
    {
        GU_ConstDetailHandle gdh = husdLoadGeometryAsset(myFiles[i]);
        if (!gdh.isValid())
            continue;

        APEXAmergeSceneDelta(scene_gdh, gdh, myInheritLayers[i]);
    }

#if 0 // Output the result to a bgeo for debugging
    mySceneGdh.gdp()->save("/tmp/scene.bgeo", nullptr);
#endif
}

void
husdApexSceneData::loadGeoInputsFromPrims(
        const HUSD_AutoAnyLock &read_lock,
        GU_DetailHandle &scene_gdh) const
{
    utZoneScopedN("husdApexSceneData load prim inputs");

    UT_ASSERT(read_lock.isStageValid());
    UsdStageConstRefPtr stage = read_lock.constData()->stage();
    const GusdPurposeSet purposes = GusdPurposeSetFromMask("*");

    auto input_bindings = UTmakeUnique<UT_Options>();

    GU_PackedFolders scene_folders(scene_gdh);

    for (const husdApexCharacterData &character : myCharacters)
    {
        for (const husdApexShapeBindingData &binding :
             character.myShapeBindings)
        {
            if (!binding.myInputName)
                continue;

            UsdPrim prim = stage->GetPrimAtPath(binding.myPrimPath.sdfPath());
            UT_ASSERT(prim.IsValid());
            if (!prim)
                continue;

            GU_DetailHandle shape_gdh;
            shape_gdh.allocateAndSet(new GU_Detail());

            if (!GusdGU_USD::ImportPrimUnpacked(
                        *shape_gdh.gdpNC(), prim, UsdTimeCode::EarliestTime(),
                        /*lod=*/nullptr, purposes))
            {
                continue;
            }

            // Insert the shape into the APEX scene, with the name (e.g.
            // 'Base.shp') as the rig input name. This will be auto-bound to the
            // rig input via APEXA_SceneCharacter::getInputGeos().
            UT_WorkBuffer shape_path;
            shape_path.format(
                    "{0}/{1}", character.myScenePath, binding.myInputName);

            // TODO - remove this attribute once it's no longer used by the
            // animate state.
            GU_Detail *gdp = shape_gdh.gdpNC();
            GA_RWAttributeRef prim_path_attr(gdp->createStringAttribute(
                    GA_ATTRIB_GLOBAL, "primPath"));
            const GA_AIFStringTuple *string_tuple =
                    prim_path_attr.getAIFStringTuple();
            if (string_tuple)
                string_tuple->setString(prim_path_attr.getAttribute(), GA_Offset(0),
                                        binding.myPrimPath.pathStr(), GA_Offset(0));

            input_bindings->setOptionS(
                    shape_path, binding.myPrimPath.pathStr());

            scene_folders.insert(shape_path, shape_gdh);
        }
    }

    // Record a detail attribute describing which shapes in the APEX scene were
    // imported from a USD prim, for use by the Animate state.
    GA_RWHandleDict input_bindings_attrib = scene_gdh.gdpNC()->addDictTuple(
            GA_ATTRIB_DETAIL, "usdinputbindings"_UTsh, 1);
    input_bindings_attrib.set(
            GA_DETAIL_OFFSET, UT_OptionsHolder(std::move(input_bindings)));

#if 0 // Output the result to a bgeo for debugging
    mySceneGdh.gdp()->save("/tmp/scene.bgeo", nullptr);
#endif
}

/// Compute an updated 'extent' attribute from the deformed positions.
UT_SmallArray<UT_Vector3F>
husdComputeExtent(const UT_Array<UT_Vector3F> &positions)
{
    UT_BoundingBoxF bbox;
    bbox.initBounds();

    for (const UT_Vector3F &pos : positions)
        bbox.enlargeBounds(pos);

    UT_SmallArray<UT_Vector3F> extent;
    extent.append(bbox.minvec());
    extent.append(bbox.maxvec());
    return extent;
}

/// Basic implementation of writing back deformed point positions and
/// normals from the APEX output geometry.
/// TODO - this could be replaced with a more general implementation handling
/// other attributes and import options, likely sharing code with SOP Import.
void
husdUpdatePrimFromGeo(
        HUSD_AutoWriteLock &write_lock,
        const HUSD_Path &prim_path,
        const HUSD_TimeCode &time_code,
        const GU_Detail &output_geo)
{
    utZoneScopedN("HUSD_ApexScene writeback to prim");

    UT_Array<UT_Vector3F> positions;
    output_geo.getPos3AsArray(output_geo.getPointRange(), positions);

    UT_SmallArray<UT_Vector3F> extent = husdComputeExtent(positions);

    GA_ROHandleV3 n_vtx_attrib(
            output_geo.findFloatTuple(GA_ATTRIB_VERTEX, GA_Names::N, 3));
    GA_ROHandleV3 n_pt_attrib(
            output_geo.findFloatTuple(GA_ATTRIB_POINT, GA_Names::N, 3));
    UT_Array<UT_Vector3F> normals;
    if (n_vtx_attrib.isValid())
    {
        // TODO - this assumes the SOP winding order matches the USD prim.
        output_geo.getAttributeAsArray(
                n_vtx_attrib.getAttribute(), output_geo.getVertexRange(),
                normals);
    }
    else if (n_pt_attrib.isValid())
    {
        output_geo.getAttributeAsArray(
                n_pt_attrib.getAttribute(), output_geo.getPointRange(),
                normals);
    }

    HUSD_SetAttributes set_attribs(write_lock);
    set_attribs.setAttributeArray(
            prim_path.pathStr(), UsdGeomTokens->points.GetString(),
            positions, time_code, /*valueType=*/UT_StringHolder::theEmptyString,
            /*custom=*/false, /*clear_existing=*/false);

    set_attribs.setAttributeArray(
            prim_path.pathStr(), UsdGeomTokens->extent.GetString(), extent,
            time_code, /*valueType=*/UT_StringHolder::theEmptyString,
            /*custom=*/false, /*clear_existing=*/false);

    if (!normals.isEmpty())
    {
        set_attribs.setAttributeArray(
                prim_path.pathStr(), UsdGeomTokens->normals.GetString(),
                normals, time_code,
                /*valueType=*/UT_StringHolder::theEmptyString,
                /*custom=*/false, /*clear_existing=*/false);
    }
}

} // namespace

HUSD_ApexScene::HUSD_ApexScene()
{
    myScene = UTmakeUnique<APEXA_SceneInvoke>();
}

HUSD_ApexScene::~HUSD_ApexScene() = default;

bool
HUSD_ApexScene::loadFromGeometry(const GU_ConstDetailHandle &scene_gdh)
{
    UT_StringHolder errors;
    if (!myScene->updateSourceGeometry(*scene_gdh.gdp(), errors))
    {
        UT_WorkBuffer msg;
        msg.format("Failed to load APEX scene:\n{0}", errors);
        HUSD_ErrorScope::addError(HUSD_ERR_STRING, msg.buffer());
        return false;
    }

    return true;
}

void
HUSD_ApexScene::addOutputPrim(
        const UT_StringHolder &scene_output_path,
        const UT_StringHolder &scene_output_key,
        const HUSD_Path &prim_path)
{
    myScene->addOutput(scene_output_path, scene_output_key);
    myPrimOutputs.append(prim_path);
}

bool
HUSD_ApexScene::loadScenes(
        const HUSD_AutoAnyLock &read_lock,
        const HUSD_FindPrims &find_prims,
        UT_Array<HUSD_ApexScene> &scenes)
{
    UT_Array<husdApexSceneData> scene_datas = husdFindScenes(
            read_lock, find_prims.getExpandedPathSet());

    for (const husdApexSceneData &scene_data : scene_datas)
    {
        HUSD_ApexScene &scene = scenes[scenes.append()];

        GU_DetailHandle scene_gdh = scene_data.loadSceneGeometry(read_lock);
        if (!scene.loadFromGeometry(scene_gdh))
            return false;

        // Add the outputs we're interested in evaluating.
        for (const husdApexCharacterData &character : scene_data.myCharacters)
        {
            // Determine which rig the character should use.
            UT_StringHolder rig_name = character.myRigName;
            if (!rig_name)
                rig_name = husdGetDefaultRig(scene_gdh, character.myScenePath);

            if (!rig_name)
            {
                UT_WorkBuffer msg;
                msg.format(
                        "Character '{0}' does not have any rigs.",
                        character.myScenePath);
                HUSD_ErrorScope::addWarning(HUSD_ERR_STRING, msg.buffer());
                continue;
            }

            UT_StringHolder rig_output_path;
            rig_output_path.format(
                    "{0}/{1}/{2}", character.myScenePath, rig_name,
                    theOutputName.asRef());

            // Set up the output shape bindings for the character.
            for (const husdApexShapeBindingData &binding :
                 character.myShapeBindings)
            {
                if (!binding.myOutputName)
                    continue;

                scene.addOutputPrim(
                        rig_output_path, binding.myOutputName,
                        binding.myPrimPath);
            }
        }
    }

    return true;
}

GU_DetailHandle
HUSD_ApexScene::loadSceneGeometry(
        const HUSD_AutoAnyLock &read_lock,
        const HUSD_Path &scene_path)
{
    HUSD_PathSet path_set;
    path_set.insert(scene_path);

    UT_Array<husdApexSceneData> scene_datas = husdFindScenes(
            read_lock, path_set);
    if (scene_datas.size() != 1)
        return GU_DetailHandle();

    const husdApexSceneData &scene_data  = scene_datas[0];
    return scene_data.loadSceneGeometry(read_lock);
}

bool
HUSD_ApexScene::evaluateOutputs(HUSD_AutoWriteLock &write_lock, fpreal frame)
{
    utZoneScopedN("HUSD_ApexScene evaluate outputs");

    UT_ASSERT(write_lock.isStageValid());
    UsdStageRefPtr stage = write_lock.data()->stage();

    // Set the evaluation time to the next sample frame, and evaluate all
    // our outputs.
    myScene->updateEvaluationTime(frame);

    for (exint i = 0, n = myScene->getOutputs().size(); i < n; ++i)
    {
        UT_StringHolder error;
        if (!myScene->evaluateOutput(i, error))
        {
            const APEXA_SceneInvoke::Output &output = myScene->getOutputs()[i];
            UT_WorkBuffer msg;
            msg.format(
                    "Failed to evaluate output {0} {1}\n{2}", output.myPath,
                    output.myKey.value_or(UT_StringHolder::theEmptyString),
                    error);
            HUSD_ErrorScope::addError(HUSD_ERR_STRING, msg.buffer());
            return false;
        }
    }

    // Update the primitives bound to a scene output.
    const HUSD_TimeCode time_code(frame);

    for (exint i = 0, n = myScene->getOutputs().size(); i < n; ++i)
    {
        const HUSD_Path &prim_path = myPrimOutputs[i];
        const APEXA_SceneInvoke::Output &output = myScene->getOutputs()[i];

        GU_ConstDetailHandle output_gdh;
        if (output.myGeometry)
            output_gdh = output.myGeometry->asConstHandle();

        if (!output_gdh.isValid())
            continue;

        husdUpdatePrimFromGeo(
                write_lock, prim_path, time_code, *output_gdh.gdp());
    }

    return true;
}

bool
HUSDaddApexSceneFile(
        HUSD_AutoWriteLock &write_lock,
        const HUSD_Path &scene_prim_path,
        const UT_StringHolder &node_path,
        const UT_StringMap<UT_StringHolder> &args,
        const GU_ConstDetailHandle &gdh,
        bool inherit_animation_layers)
{
    utZoneScopedN("HUSDaddApexSceneFile");

    UT_ASSERT(write_lock.isStageValid());
    UsdStageRefPtr stage = write_lock.data()->stage();

    auto scene_prim = UsdHoudiniHoudiniApexScene::Get(
            stage, scene_prim_path.sdfPath());

    if (!scene_prim)
    {
        UT_WorkBuffer msg;
        msg.format(
                "Primitive '{0}' is not a {1}", scene_prim_path.sdfPath(),
                UsdHoudiniTokens->HoudiniApexScene);
        HUSD_ErrorScope::addWarning(HUSD_ERR_STRING, msg.buffer());
        return false;
    }

    UT_StringHolder file_path;
    file_path.format("{0}{1}.sop", OPREF_PREFIX, node_path);

    XUSD_LockedGeoArgs usd_args;
    HUSDconvertToFileFormatArguments(args, usd_args);

    write_lock.data()->addLockedGeo(
            XUSD_LockedGeoRegistry::createLockedGeo(file_path, usd_args, gdh));

    VtArray<SdfAssetPath> files;
    scene_prim.GetSceneFilesAttr().Get(&files);
    VtArray<bool> inherit_layers_settings;
    scene_prim.GetInheritAnimationLayersAttr().Get(&inherit_layers_settings);

    const std::string geo_id = SdfLayer::CreateIdentifier(
            file_path.toStdString(), usd_args);
    files.push_back(SdfAssetPath(geo_id));
    scene_prim.GetSceneFilesAttr().Set(files);

    inherit_layers_settings.push_back(inherit_animation_layers);
    scene_prim.GetInheritAnimationLayersAttr().Set(inherit_layers_settings);

    return true;
}
