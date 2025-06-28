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

#ifndef __HUSD_ApexScene_h__
#define __HUSD_ApexScene_h__

#include "HUSD_API.h"

#include "HUSD_Path.h"

#include <SYS/SYS_Types.h>
#include <UT/UT_Array.h>
#include <UT/UT_NonCopyable.h>
#include <UT/UT_StringMap.h>
#include <UT/UT_UniquePtr.h>

class APEXA_SceneInvoke;
class GU_ConstDetailHandle;
class GU_DetailHandle;
class HUSD_AutoAnyLock;
class HUSD_AutoWriteLock;
class HUSD_FindPrims;
class UT_StringHolder;

/// Stores an APEX scene loaded from USD (a HoudiniApexScene prim), along with
/// information about the USD primitives bound to APEX scene outputs.
/// This can be safely cached between LOP cooks to avoid rebuilding APEX scenes
/// unless the input stage has changed.
class HUSD_API HUSD_ApexScene
{
public:
    HUSD_ApexScene();
    ~HUSD_ApexScene();

    UT_NON_COPYABLE(HUSD_ApexScene)

    /// Assembles APEX scenes from the specified list of HoudiniApexScene
    /// prims.
    /// This performs validation checks to ensure that e.g. a prim isn't driven
    /// by the output of more than one scene.
    static bool loadScenes(
            const HUSD_AutoAnyLock &read_lock,
            const HUSD_FindPrims &find_prims,
            UT_Array<HUSD_ApexScene> &scenes);

    /// Assemble the input geometry for an APEX scene from the specified
    /// HoudiniApexScene prim.
    static GU_DetailHandle loadSceneGeometry(
            const HUSD_AutoAnyLock &read_lock,
            const HUSD_Path &scene_path);

    /// Evaluates the scene at the specified frame, and updates any prims bound
    /// to a scene output.
    bool evaluateOutputs(HUSD_AutoWriteLock &write_lock, fpreal frame);

private:
    /// Loads the APEX scene from packed folders.
    bool loadFromGeometry(const GU_ConstDetailHandle &scene_gdh);

    /// Registers a prim which is bound to an APEX scene output.
    void addOutputPrim(
            const UT_StringHolder &scene_output_path,
            const UT_StringHolder &scene_output_key,
            const HUSD_Path &prim_path);

    UT_UniquePtr<APEXA_SceneInvoke> myScene;
    UT_Array<HUSD_Path> myPrimOutputs;
};

/// Add the geometry (packed folders) to the locked geometry registry, and add a
/// reference to it from the 'sceneFiles' attribute on the HoudiniApexScene
/// prim.
HUSD_API bool HUSDaddApexSceneFile(
        HUSD_AutoWriteLock &write_lock,
        const HUSD_Path &scene_prim_path,
        const UT_StringHolder &node_path,
        const UT_StringMap<UT_StringHolder> &args,
        const GU_ConstDetailHandle &gdh,
        bool inherit_animation_layers);

#endif
