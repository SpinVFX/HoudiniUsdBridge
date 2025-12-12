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
 *
 * Produced by:
 *	Side Effects Software Inc.
 *	123 Front Street West, Suite 1401
 *	Toronto, Ontario
 *      Canada   M5J 2M2
 *	416-504-9876
 *
 */

#ifndef __HUSD_Imaging_h__
#define __HUSD_Imaging_h__

#include "HUSD_API.h"
#include "HUSD_DataHandle.h"
#include "HUSD_RenderBuffer.h"
#include "HUSD_RendererInfo.h"
#include "HUSD_Utils.h"
#include <UT/UT_BoundingBox.h>
#include <UT/UT_Function.h>
#include <UT/UT_Matrix3.h>
#include <UT/UT_Matrix4.h>
#include <UT/UT_NonCopyable.h>
#include <UT/UT_Options.h>
#include <UT/UT_Rect.h>
#include <UT/UT_StringArray.h>
#include <UT/UT_UniquePtr.h>
#include <UT/UT_Vector2.h>
#include <pxr/pxr.h>

PXR_NAMESPACE_OPEN_SCOPE
class VtValue;
class XUSD_RenderSettings;
class XUSD_RenderSettingsContext;
PXR_NAMESPACE_CLOSE_SCOPE

class COP_ApexProgram;
class HUSD_Compositor;
class HUSD_Scene;
class husd_DefaultRenderSettingContext;
class PXL_Raster;
class TIL_Raster;

class HUSD_API HUSD_Imaging : public UT_NonCopyable
{
public:
			 HUSD_Imaging();
			~HUSD_Imaging();

    // Definition for various callbacks from HUSD_Imaging.
    typedef UT_Function<void (HUSD_Imaging *imaging)> ImagingCallback;

    // The scene is not owned by this class.
    void setScene(HUSD_Scene *scene_ref);

    // only the USD modes that map to ours
    enum DrawMode
    {
	DRAW_WIRE,
	DRAW_SHADED_NO_LIGHTING,
	DRAW_SHADED_FLAT,
	DRAW_SHADED_SMOOTH,
	DRAW_WIRE_SHADED_SMOOTH
    };

    void		 showPurposeRender(bool enable);
    void		 showPurposeProxy(bool enable);
    void		 showPurposeGuide(bool enable);

    void		 setDrawMode(DrawMode mode);
    void		 setDrawComplexity(float complexity);
    void		 setBackfaceCull(bool cull);
    void		 setStage(const HUSD_DataHandle &data_handle,
				const HUSD_ConstOverridesPtr &overrides,
				const HUSD_ConstPostLayersPtr &postlayers);
    bool		 setFrame(fpreal frame);
    bool		 setDefaultLights(bool doheadlight, bool dodomelight);
    void                 setHeadlightIntensity(fpreal intensity);
    void		 setLighting(bool enable);
    void		 setMaterials(bool enable);
    void                 setAspectPolicy(HUSD_AspectConformPolicy p);

    // Pass false to this method before setting up the renderer. This
    // causes the XUSD_ImagingEngineGL object to be created with the
    // "Null Hgi" instead of the standard OpenGL Hgi.
    void                 setAllowStormRenderer(bool allow_storm);

    enum BufferSet
    {
        BUFFER_COLOR_DEPTH,
        BUFFER_COLOR,
        BUFFER_NONE,
    };
    BufferSet            hasAOVBuffers() const;
    
    // This callback is run after the UsdImagineEngineGL::_Execute method.
    // This method will clear the current VAO when it exits when running in
    // a core profile OpenGL context (i.e. always on Mac). So we need a
    // chance ot notify the RE_OGLRender that the VAO has been unbound.
    void		 setPostRenderCallback(ImagingCallback cb);
    bool		 getUsingCoreProfile();

    bool                 canBackgroundRender(const UT_StringRef &name) const;

    // Fire off a render and return immediately.
    // Only call if canBackgroundRender() returns true.
    bool                 launchBackgroundRender(const UT_Matrix4D &view_matrix,
                                                const UT_Matrix4D &proj_matrix,
                                                const UT_DimRect  &viewport_rect,
                                                const UT_StringRef &renderer,
                                                const UT_Options *render_opts,
                                                bool cam_effects);
    // Wait for the BG update to be finished.
    void                 waitForUpdateToComplete();
    // Check if the BG update is finished, and optionally do a render if it is.
    bool                 checkRender(bool do_render);
    
    /// This struct holds the view metadata that's applied to the input layers
    /// before they're piped to slapcomp.
    struct SlapcompViewInfo
    {
        fpreal          myViewAspectRatio       { 1.0 };
        fpreal          myAperture              { 20.955 };
        fpreal          myHApertureOffset       { 0.0 };
        fpreal          myVApertureOffset       { 0.0 };
        UT_Matrix4D     myTransformMatrix       { 1.0 };
        fpreal          myOrthoWidth            { 2.0 };
        fpreal          myFocalLength           { 50.0 };
        fpreal          myNearLimit             { 0.01 };
        fpreal          myFarLimit              { 100000.0 };
        int             myOrthoFlag             { 1 };
    };

    void                 updateComposite(bool free_buffers_if_missing,
                                         const SlapcompViewInfo *v = nullptr);

    /// Returns one of the primary buffers.
    HUSD_RenderBuffer    getAOVBuffer(const UT_StringRef &name) const;
    /// Returns a buffer even if it is an extra buffer.
    HUSD_RenderBuffer    getAOVBufferIncludingExtra(const UT_StringRef &name) const;
    void                 getAOVRasters(const UT_Vector2i &res,
                                const UT_StringHolder &aovpattern,
                                UT_StringArray &aovnames,
                                UT_StringMap<UT_UniquePtr<PXL_Raster>> &rasters,
                                const TIL_Raster *background_raster = nullptr,
                                int bleft = 0, int bbottom = 0,
                                int bright = 0, int btop = 0,
                                fpreal sx = 0.0, fpreal sy = 0.0,
                                fpreal sw = 1.0, fpreal sh = 1.0) const;
    bool                 getAOVBufferInfo(UT_Vector2i &resolution,
                                UT_DimRect &data_window) const;

    // Fire off a render and block until done. It may return false if the
    // render delegate fails to initialize, it which case another delegate
    // should be chosen.
    bool		 render(const UT_Matrix4D &view_matrix,
				const UT_Matrix4D &proj_matrix,
				const UT_DimRect  &viewport_rect,
				const UT_StringRef &renderer,
				const UT_Options *render_opts,
                                bool cam_effects,
                                const SlapcompViewInfo *v = nullptr);
    
    // Set the camera being viewed through (can be null for no camera);
    void                 setCameraPath(const UT_StringRef &path,
                                       bool camera_synced = true)
                         {
                             if(path != myCameraPath)
                             {
                                 myCameraPath = path;
                                 mySettingsChanged = true;
                             }
                             // use camera for sampling parms, not frustum.
                             if(camera_synced != myCameraSynced)
                             {
                                 myCameraSynced = camera_synced;
                                 mySettingsChanged = true;
                             }
                         }

    void		 setAOVCompositor(HUSD_Compositor *comp)
			 { myCompositor = comp; }

    HUSD_Scene		&scene()
			 { return *myScene; }
    bool		 isConverged() const
			 { return !isUpdateRunning() && myConverged; }
    void		 terminateRender(bool hard_halt = true);

    bool		 getBoundingBox(UT_BoundingBox &bbox,
				const UT_Matrix3R *rot) const;

    const UT_StringHolder &rendererName() const
			  { return myRendererName; }
    void                  getRendererCommands(
                                UT_StringArray &command_names,
                                UT_StringArray &command_descriptions) const;
    void                  invokeRendererCommand(
                                const UT_StringHolder &command_name) const;

    enum RunningStatus {
	RUNNING_UPDATE_NOT_STARTED = 0,
	RUNNING_UPDATE_IN_BACKGROUND,
	RUNNING_UPDATE_COMPLETE,
        RUNNING_UPDATE_FATAL
    };
    bool		 isUpdateRunning() const;
    bool                 isUpdateComplete() const;

    // Control the pause state of the render. Return true if it is paused.
    // Track pausing invoked by the user separately from "automatic" pausing
    // which happens when switching between Houdini GL and another renderer.
    void                 pauseRender();
    void                 resumeRender();
    bool                 canPause() const;
    bool                 isPausedByUser() const;
    bool                 isStoppedByUser() const;
    bool                 rendererCreated() const;

    // Track whether this object should process updates from the stage. Also
    // controls whether the renderer can be unpaused. We want to prevent the
    // automatic unpausing of the render when the user explicitly pauses it.
    bool                 allowUpdates() const
                         { return myAllowUpdates; }
    void                 setAllowUpdates(bool allow_updates)
                         { myAllowUpdates = allow_updates; }

    static void          initializeAvailableRenderers();
    static bool		 getAvailableRenderers(HUSD_RendererInfoMap &info_map);

    void                 setRenderSettings(const UT_StringRef &settings_path,
                                 int w=0, int h=0,
                                 fpreal resscale=0.0);

    const UT_StringArray  &rendererPlanes() const { return myPlaneList; }
    bool                   setOutputPlane(const UT_StringRef &name);
    const UT_StringHolder &outputPlane() const { return myOutputPlane; }
    const UT_StringHolder &currentAov() const { return myCurrentAOV; }

    void                 getRenderStats(UT_Options &stats);

    void                 setRenderFocus(int x, int y) const;
    void                 clearRenderFocus() const;

    // Apply options to the contained render settings context to control
    // whether or not the data window from the render settings prim should be
    // passed to the renderer or overridden (and the full image rendered).
    // We only pass along the render settings data window if the render
    // region is deactivated _and_ the data window is activated.
    void                 setRenderRegionActive(bool active);
    void                 setDataWindowActive(bool active);

    // Returns the paths associated with render keys from the primid and instid
    // buffers. Stores the result in myRenderKeyToPathMap so future lookups
    // are fast.

    void                 getPrimPathsFromRenderKeys(
                                const UT_Set<HUSD_RenderKey> &keys,
                                HUSD_RenderKeyPathMap &outkeypathmap);

    void		 updateDeferredPrims();

    /// Set the slapcomp program that should be used when slapcomp is enabled.
    void                 setSlapcompProgram(COP_ApexProgram* program)
                         { mySlapcompProgram = program; }
    /// Turn slapcomp on or off. setSlapcompProgram() should be called prior to
    /// this.
    void                 enableSlapcomp(bool do_slap);
    /// Returns true if slapcomp is enabled.
    bool                 isSlapcompEnabled() const
                         { return mySlapcompEnabled; }
    /// If this returns true, then the last attempted application of slapcomp
    /// may have changed the possible outputs and/or the current display AOV.
    /// This flag is reset before each application of slapcomp.
    bool                 slapcompStateChanged() const
                         { return mySlapcompStateChanged; }

    /// Returns true if the last run of slapcomp had errors.
    bool                 hasSlapcompErrors() const
                         { return mySlapcompEnabled && mySlapcompHasErrors; }
    /// Slapcomp error reporting. This also activates the mySlapcompHasErrors
    /// flag.
    void                 reportSlapcompError(const char* error);
    /// Slapcomp warning reporting.
    void                 reportSlapcompWarning(const char* error);
    /// Sets a callback that gets called when reporting errors.
    void                 setSlapcompErrorCallback(void (*cb)(void*,
                                                             const char*),
                                                  void* data)
    {
        mySlapcompErrorCB = cb;
        mySlapcompErrorCBData = data;
    }
    /// Sets a callback that gets called when reporting warnings.
    void                 setSlapcompWarningCallback(void (*cb)(void*,
                                                               const char*),
                                                    void* data)
    {
        mySlapcompWarningCB = cb;
        mySlapcompWarningCBData = data;
    }

    /// Called by our data micro node that monitors for changes to COP textures
    /// used by karma renders. When this function is called we need to restart
    /// the render.
    void                 handleCopTextureChange(bool time_changed);
    /// Set a callback that is run at the end of handleCopTextureChange.
    void		 setCopTextureChangeCallback(ImagingCallback cb);

private:
    class husd_ImagingPrivate;
    class husd_IMXRenderBuffer;

    /// Returns true if the given AOV is an output of slapcomp.
    bool                isSlapcompAOV(const UT_StringHolder& name) const;

    /// Registers all active AOVs with the slapcomp registry,
    /// run on terminate or on convergence
    void                registerSlapcompAOVs(bool dostash, const SlapcompViewInfo *view_info);

    /// Internal helper function to run slapcomp. This function resets both
    /// mySlapcompStateChanged and mySlapcompHasErrors before doing its work.
    void                runSlapcompIfNeeded(const SlapcompViewInfo* view_info);
    /// Returns a pointer to HdRenderBuffer. This method fetches an output of
    /// the specified name (slapcomp layer if it has an output of the given
    /// name, otherwise the renderer's result).
    void*               getRenderOrSlapcompOutput(const UT_StringHolder& name,
                                                  husd_IMXRenderBuffer* b)
        const;
    /// Builds the slapcomp program and updates the list of available planes. If
    /// the operation makes any changes to output planes, mySlapcompStateChanged
    /// flag is activated. Returns true if everything succeeded, false
    /// otherwise. If force is true, the plane list is reset and built anew;
    /// otherwise, it's only rebuilt if the program required recompilation.
    bool                buildSlapcompProgramAndUpdatePlanes(bool force);

    void                 resetImagingEngine();
    const HUSD_DataHandle &viewerLopDataHandle() const;
    bool                 updateRestartCameraSettings(bool cam_effects) const;
    bool                 anyRestartRenderSettingsChanged() const;
    bool		 setupRenderer(const UT_StringRef &renderer_name,
                                       const UT_Options *render_opts,
                                       bool cam_effects);
    void                 updateSettingIfRequired(const UT_StringRef &key,
                                const PXR_NS::VtValue &value,
                                bool from_usd_prim = false);
    void                 updateSettingsIfRequired(HUSD_AutoReadLock &lock);
    RunningStatus	 updateRenderData(const UT_Matrix4D &view_matrix,
                                          const UT_Matrix4D &proj_matrix,
                                          const UT_DimRect &viewport_rect,
                                          bool cam_effects);
    void                 gatherCopResolverDependencies();
    void		 finishRender(bool do_render);

    UT_UniquePtr<husd_ImagingPrivate>	 myPrivate;
    fpreal				 myFrame;
    HUSD_DataHandle			 myDataHandle;
    UT_UniquePtr<HUSD_AutoReadLock>      myReadLock;
    HUSD_ConstOverridesPtr		 myOverrides;
    HUSD_ConstPostLayersPtr              myPostLayers;
    unsigned				 myWantsHeadlight : 1,
					 myHasHeadlight : 1,
                                         myWantsDomelight : 1,
                                         myHasDomelight : 1,
					 myDoLighting : 1,
					 myDoMaterials : 1,
					 myConverged : 1,
                                         mySettingsChanged : 1,
                                         myCameraSynced : 1,
                                         myValidRenderSettingsPrim : 1,
                                         myAOVsStashed : 1;
    bool                                 myIsPaused;
    bool                                 myAllowUpdates;
    bool                                 myAllowStormRenderer;
    HUSD_Scene				*myScene;
    UT_StringHolder			 myRendererName;
    HUSD_Compositor			*myCompositor;
    ImagingCallback			 myPostRenderCallback;
    ImagingCallback			 myCopTextureChangeCallback;
    UT_Options				 myCurrentDisplayOptions;
    SYS_AtomicInt32			 myRunningInBackground;
    UT_StringHolder                      myOutputPlane;
    UT_StringHolder                      myCurrentAOV;
    UT_StringHolder                      myCameraPath;
    UT_UniquePtr<PXR_NS::XUSD_RenderSettings> myRenderSettings;
    UT_UniquePtr<husd_DefaultRenderSettingContext> myRenderSettingsContext;
    int                                  myConformPolicy;
    BufferSet                            myLastCompositedBufferSet;
    UT_Map<HUSD_RenderKey, UT_StringHolder> myRenderKeyToPathMap;
    fpreal                               myHeadlightIntensity;

    /// Track whether we are currently handling a COP texture change (we want
    /// to avoid getting into a recursive loop of such calls, which should be
    /// impossible, but better safe than sorry).
    bool                                 myHandlingCopTextureChange;

    /// The list of all planes that this object knows about. This is a union of
    /// renderer's and slapcomp's plane, with duplicates removed (as slapcomp's
    /// results take precedence over the renderer's).
    UT_StringArray                       myPlaneList;
    /// The list of planes that are outputs of the renderer.
    UT_StringArray                       myRendererPlaneList;
    /// The list of planes that are outputs of slapcomp.
    UT_StringArray                       mySlapcompPlaneList;

    /// The slapcomp program.
    COP_ApexProgram                     *mySlapcompProgram;
    /// Callback function to report errors to the outside world.
    void                               (*mySlapcompErrorCB)(void*, const char*);
    /// Data to send to the error reporting callback.
    void*                                mySlapcompErrorCBData;
    /// Callback function to report warnings to the outside world.
    void                               (*mySlapcompWarningCB)(void*,
                                                              const char*);
    /// Data to send to the warning reporting callback.
    void*                                mySlapcompWarningCBData;
    /// Enable state of slapcomp.
    bool                                 mySlapcompEnabled;
    /// If this flag is true, last application of slapcomp incurred a rebuild of
    /// the program that may have changed the available output layers or the
    /// current one.
    bool                                 mySlapcompStateChanged;
    /// If this flag is true, last application of slapcomp had errors and
    /// failed.
    bool                                 mySlapcompHasErrors;

    /// Tracks last view info so we can update on terminate.
    SlapcompViewInfo                     myLastSlapcompViewInfo;
};

#endif

