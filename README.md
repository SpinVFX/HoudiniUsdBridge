# HoudiniUsdBridge
Houdini libraries that must be recompiled to use Houdini with a custom build of
the USD library.

## Building USD on Linux

In order for this to work, there are a few requirements for how the USD library
is built:

1. You must define "_GLIBCXX_USE_CXX11_ABI=0"
2. You must set the compile flag "-std=c++14"

These conditions can be met by editing
USD/cmake/defaults/gccclangshareddefaults.cmake to set these options.
Specifically, the following line:

```
set(_PXR_GCC_CLANG_SHARED_CXX_FLAGS "${_PXR_GCC_CLANG_SHARED_CXX_FLAGS} -std=c++11")
```

Should be replaced by:

```
set(_PXR_GCC_CLANG_SHARED_CXX_FLAGS "${_PXR_GCC_CLANG_SHARED_CXX_FLAGS} -std=c++14")
set(_PXR_GCC_CLANG_SHARED_CXX_FLAGS "${_PXR_GCC_CLANG_SHARED_CXX_FLAGS} -D_GLIBCXX_USE_CXX11_ABI=0")
```

In addition, the boost build used by USD must also set these options, which can
be done by editing the USD/build_scripts/build_usd.py file. Specifically, the
following lines must be added to the b2_settings variable:

```python
'cflags="-fPIC -std=c++14 -D_GLIBCXX_USE_CXX11_ABI=0"'
'cxxflags="-fPIC -std=c++14 -D_GLIBCXX_USE_CXX11_ABI=0"'
```

The OpenSubdiv build must also set these options in the root CMakeLists.txt
file:

```
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_GLIBCXX_USE_CXX11_ABI=0")
```

This can be a challenge when using the USD build script, as the code is
downloaded and built in a single step. You must either interrupt this build
step to make the change, or make the change after the build, and then force
OpenSubdiv (and then USD) to rebuild with these changes.

## SideFX Changes to the USD Library

The USD library that ships with Houdini is forked from the official USD 22.05
release. Some SideFX-specific changes have since been applied to fix bugs, or
deal with SideFX-specific build issues. Whenever possible we submit pull
requests to have our changes integrated into the next USD release, but
sometimes this isn't possible. And in any case, these changes are not part
of the USD 22.05 release. This section lists every deviation of the SideFX
USD library from Pixar's 22.05 branch, to help you decide which of these
changes you may want or need to incorporate into your own USD build.

Changes ordered from oldest to newest at initial release of Houdini 20.5:
- [bab810a255a4833aa92c15cf740c9d0cf7ab8ef7](https://github.com/sideeffects/USD/commit/bab810a255a4833aa92c15cf740c9d0cf7ab8ef7):
  - **Not required**: Changing usage of OSL namespace to HOSL.
- [260e062560b335ab30fea51bf9bf175b2d11c47e](https://github.com/sideeffects/USD/commit/260e062560b335ab30fea51bf9bf175b2d11c47e):
  - **Not required**: Provide a flag to turn on the MATERIALX_BUILD_SHARED_LIBS define in libraries that build against MaterialX. This is required to enable defines within MaterialX code that allow importing symbols from shared libraries on Windows.
- [85abe9e92430c2d06f0b485ffa58ffb304daf4a9](https://github.com/sideeffects/USD/commit/85abe9e92430c2d06f0b485ffa58ffb304daf4a9):
  - **Not required**: Attempt to link pxOsd against osdCPU only, not osdGPU. This might help eliminate indirect dependencies on OpenGL.
- [46a0a6f9dc3d2d288f92edf2fba70e691541b36b](https://github.com/sideeffects/USD/commit/46a0a6f9dc3d2d288f92edf2fba70e691541b36b):
  - **Not required**: Switch the default value for USD_ABC_XFORM_PRIM_COLLAPSE from true to false, since this seems to be what most Houdini users expect.
- [00efdf708a6018bc8d8f6e598c63f62dfa35b307](https://github.com/sideeffects/USD/commit/00efdf708a6018bc8d8f6e598c63f62dfa35b307):
  - **Not required**: Ensure default (public) symbol visibility on macOS
- [a0179d6cfc0253992fe7d70f5b68d8372260b598](https://github.com/sideeffects/USD/commit/a0179d6cfc0253992fe7d70f5b68d8372260b598):
  - **Not required**: Provide a define that will allow render delegate developers to disable python support while building their render delegates against Houdini's USD library (which is always built with python support enabled).
- [e7f6a2647bf259f0896dbd6bd6c6a299c0f66e58](https://github.com/sideeffects/USD/commit/e7f6a2647bf259f0896dbd6bd6c6a299c0f66e58):
  - **Not required**: Skip the building of the `extras` subdirectory (issues with Windows)
- [9c3ff1db733e15f00f8ea41d5e3a52f1bcd0f545](https://github.com/sideeffects/USD/commit/9c3ff1db733e15f00f8ea41d5e3a52f1bcd0f545):
  - **Not required**: Pick up all the required Alembic components
- [d17beb35fcc32f82f839195e72c91718431cc7a6](https://github.com/sideeffects/USD/commit/d17beb35fcc32f82f839195e72c91718431cc7a6):
  - **Not required**: Change RPATH locations
- [083ab0c0b7baa653846866e97f174f71c5f1f3b2](https://github.com/sideeffects/USD/commit/083ab0c0b7baa653846866e97f174f71c5f1f3b2):
  - **Not required**: Pick up Houdini Qt plugins and fonts
- [c5a5810755312d8156e05507b833fdb15dbe8a33](https://github.com/sideeffects/USD/commit/c5a5810755312d8156e05507b833fdb15dbe8a33):
  - **Not required**: Disable incremental linking on Windows
- [38e26f6ebd3636a01937a7a5a45aa30561ee2380](https://github.com/sideeffects/USD/commit/38e26f6ebd3636a01937a7a5a45aa30561ee2380):
  - **Not required**: Support for debug builds
- [39e4d3e2f807d14953b013fd8782e470a70cdd79](https://github.com/sideeffects/USD/commit/39e4d3e2f807d14953b013fd8782e470a70cdd79):
  - **Not required**: A few defines specific to how Houdini uses USD
- [b63f3a4360f8f18a85061df6de1b465d0551d5e6](https://github.com/sideeffects/USD/commit/b63f3a4360f8f18a85061df6de1b465d0551d5e6):
  - **Not required**: Change invocation of Python when finding PySide and running (py)uic
- [427dc934ed8bdad736966f4e7ed34b638a6c2b8b](https://github.com/sideeffects/USD/commit/427dc934ed8bdad736966f4e7ed34b638a6c2b8b):
  - **Required**: Register local constant-interpolation primvars on point instancer primitives as hydra-accessible primvars.
- [850599b68e3f9cc88555fb209578af45c5ff41a2](https://github.com/sideeffects/USD/commit/850599b68e3f9cc88555fb209578af45c5ff41a2):
  - **Required**: Support inherited primvars on point instancer prims.
- [f41c0f5cb012a60b48cf0dd2a8b956a9de1ece84](https://github.com/sideeffects/USD/commit/f41c0f5cb012a60b48cf0dd2a8b956a9de1ece84):
  - **Not required**: Add new HdSceneDelegate and HdPrim methods to get a "data sharing id" for an HdPrim.
- [a9a90da68c4746028359c2e181153e5c70a5aca0](https://github.com/sideeffects/USD/commit/a9a90da68c4746028359c2e181153e5c70a5aca0):
  - **Not required**: Incorporate change to set umask properly when creating USD files, as described here: https://github.com/PixarAnimationStudios/USD/issues/1604
- [c2e4464404b7c9a4f0c48d7cca4c864f18bdb34c](https://github.com/sideeffects/USD/commit/c2e4464404b7c9a4f0c48d7cca4c864f18bdb34c):
  - **Not required**: Ignore reparse points on network file systems. The destination is unlikely to be accessible from the remote machine. Unmerged PR https://github.com/PixarAnimationStudios/OpenUSD/pull/2934.
- [198e086d7ba04ec8ff12a56aade97b33d6e611a9](https://github.com/sideeffects/USD/commit/198e086d7ba04ec8ff12a56aade97b33d6e611a9):
  - **Not required**: Get correct invalidations to coordSys sprims when their target USD prim is updated.
- [e1fa40bab3154aa1e763d82ad2932fee99c60cfa](https://github.com/sideeffects/USD/commit/e1fa40bab3154aa1e763d82ad2932fee99c60cfa):
  - **Required**: Remove the DrawModeAdapter that ships with USD. Houdini provides its own version of this optimized for use with HGL. Having both installed leads to warnings on startup (and could lead to the use of the wrong one).
- [ca54b212055a5c336d52800ae1a9404448c25ffa](https://github.com/sideeffects/USD/commit/ca54b212055a5c336d52800ae1a9404448c25ffa):
  - **Required**: Make GlfSimpleLight more controllable, but hide these extra controls behind a flag so that non-Houdini applications won't be affected by the addition of these new parameters.
- [b823741f822f8b6da175a209b0764f666c11b029](https://github.com/sideeffects/USD/commit/b823741f822f8b6da175a209b0764f666c11b029):
  - **Required**: Fix a potential crash when updating skinned primitives. PR accepted after 24.03 https://github.com/PixarAnimationStudios/OpenUSD/pull/2931.
- [4fdda3f2ce80fcf3fe0d190132d98dcf75da654c](https://github.com/sideeffects/USD/commit/4fdda3f2ce80fcf3fe0d190132d98dcf75da654c):
  - **Required**: Improve performance of generating the change notice when adding or removing sublayers from a stage. PR accepted after 24.03 https://github.com/PixarAnimationStudios/OpenUSD/pull/2937.
- [1a504a0396d50781b2f660b7aef4fbeadd1e9856](https://github.com/sideeffects/USD/commit/1a504a0396d50781b2f660b7aef4fbeadd1e9856):
  - **Not required**: Adding $HDSO/usd_plugins to the rpath/runpath of MacOS and Linux USD builds.
- [962b223eacdb8efbbcfdf18f65071676babb6e84](https://github.com/sideeffects/USD/commit/962b223eacdb8efbbcfdf18f65071676babb6e84):
  - **Not required**: Fix incorrectly transformed points for GPU blendshapes with no skinning. Unmerged PR https://github.com/PixarAnimationStudios/OpenUSD/pull/2696.
- [8ee3464902083e36f2807fcdd9555552d5268691](https://github.com/sideeffects/USD/commit/8ee3464902083e36f2807fcdd9555552d5268691):
  - **Required**: Add a proper cast-to-bool operation to pxr.Pcp.NodeRef which returns false if the referenced PcpNodeRef cast to bool returns false. PR merged after 24.03 https://github.com/PixarAnimationStudios/OpenUSD/pull/2943.
- [452584b8a91ca2336f2ca68025fb61bb51a97bf1](https://github.com/sideeffects/USD/commit/452584b8a91ca2336f2ca68025fb61bb51a97bf1):
  - **Not required**: If an alembic file has authored the "DCCFPS" metadata in its config info, use that value as the TimeCodesPerSecond value of the equivalent USD data. PR merged after 24.03 https://github.com/PixarAnimationStudios/OpenUSD/pull/2944.
- [02d71c2effe0961cd7ea98b3ce9e31202009444b](https://github.com/sideeffects/USD/commit/02d71c2effe0961cd7ea98b3ce9e31202009444b):
  - **Not required**: Interpolate VtValue's containing GfMatrix*f types in HdResampleNeighbors().
- [2e9861da0660bf29782d1a260bf309fda50f216d](https://github.com/sideeffects/USD/commit/2e9861da0660bf29782d1a260bf309fda50f216d):
  - **Not required**: Fix an issue where an unexpected time sample could be reported for skinning computations. Unmerged PR https://github.com/PixarAnimationStudios/OpenUSD/pull/2956.
- [b76227b5c1e236c3a263d94dc8e3f150753913fb](https://github.com/sideeffects/USD/commit/b76227b5c1e236c3a263d94dc8e3f150753913fb):
  - **Not required**: Fix DeprecationWarning caused by regex containing an invalid escape sequence. Unmerged PR https://github.com/PixarAnimationStudios/OpenUSD/pull/2955.
- [4dba0c32fe5af3318f654d481118e43cb9fd99bf](https://github.com/sideeffects/USD/commit/4dba0c32fe5af3318f654d481118e43cb9fd99bf):
  - **Required**: Fix sizing of result array when calling GetScenePrimPaths with some invalid instance indices. PR merged after 24.03 https://github.com/PixarAnimationStudios/OpenUSD/pull/2960.
- [cf3a7cd6c232a858e9d80c25fe03d912486d153e](https://github.com/sideeffects/USD/commit/cf3a7cd6c232a858e9d80c25fe03d912486d153e):
  - **Required**: Ensure the output result vector from GetScenePrimPaths is the same length as the input instanceIndices vector even if none of the instance indices are valid. PR merged after 24.03 https://github.com/PixarAnimationStudios/OpenUSD/pull/2960.
- [dbcc20e26d267802e77905b3fd25541735eb8f39](https://github.com/sideeffects/USD/commit/dbcc20e26d267802e77905b3fd25541735eb8f39):
  - **Not required**: Remove camera adapter registration so that Houdini's camera adapter will be used instead.
- [781c387574e332565cc29e32a6831f36ae0c9208](https://github.com/sideeffects/USD/commit/781c387574e332565cc29e32a6831f36ae0c9208):
  - **Not required**: Export the protected _RemovePrim method so that we can subclass the UsdImagingCameraAdapter and call this method (though on Windows even if we don't call the method, linking fails when trying to create a subclass because of this inaccessible method).
- [5c08ef4dc506c89121fd945d21d38ea49cd7c339](https://github.com/sideeffects/USD/commit/5c08ef4dc506c89121fd945d21d38ea49cd7c339):
  - **Not required**: Add an environment variable that allows the overriding of the dome light texture file used for "simple light" dome lights.
- [473075bc6cdcae2c1ad923f4b96bd4cbb5e54319](https://github.com/sideeffects/USD/commit/473075bc6cdcae2c1ad923f4b96bd4cbb5e54319):
  - **Not required**: When modifying asset paths in a layer, don't remove empty elements from arrays. The empty elements may be meaningful, as in primvars that must be a specific length. Unmerged PR https://github.com/PixarAnimationStudios/OpenUSD/pull/3063.
- [63566469c132cfd143a3f0f3339d92969550a3ea](https://github.com/sideeffects/USD/commit/63566469c132cfd143a3f0f3339d92969550a3ea):
  - **Not required**: Add houdiniFieldAsset as a "legacy volume field prim type". Without this addition, Houdini fields are not tracked properly by Hydra for time varying attributes.
- [4f7e1c78e68eb5203851ee48f7ce873a11118cb4](https://github.com/sideeffects/USD/commit/4f7e1c78e68eb5203851ee48f7ce873a11118cb4):
  - **Not required**: Export UsdImagingDataSourceFieldAsset constructors so that external libraries can call the static New() method on these classes (which is inlined).
- [60dab1a9a0441b32d1dff49355286c121aef3629](https://github.com/sideeffects/USD/commit/60dab1a9a0441b32d1dff49355286c121aef3629):
  - **Not required**: Guard against warning C4003 on Windows. Change made after 24.03 to fix issue https://github.com/PixarAnimationStudios/OpenUSD/issues/2624.

## Building Houdini libraries

Once you have built USD, it is time to build the replacement libHoudiniUSD.so
(libHUSD.dll and libgusd.dll on Windows), USD_Plugins.so, and other libraries.
You must use CMake to build these libraries. CMake version 3.12 is required
(due to the use of add_compile_definitions).

The following variables are used to configure the CMake build:

* HOUDINI_PATH: The path to the root of the Houdini install. If this is
  omitted, the $HFS environment variable will be checked as well.
* USD_ROOT: The path to the USD install.
* USD_LIB_PREFIX: The naming prefix of the USD libraries to build/link against.
  This should match the value of the `PXR_LIB_PREFIX` CMake variable used to
  build USD, and defaults to "lib" (which matches the USD build default).
* BOOST_NAMESPACE: The namespace of the Boost build used by the external USD
  build. This defaults to "boost".
* COPY_HOUDINI_USD_PLUGINS: Whether to copy the $HH/dso/usd_plugins directory
  from the Houdini install into the HoudiniUsdBridge install tree. This defaults  to ON.

Once the libraries are built, run `make install`. This will copy the bridge libraries
into the installation directory structure (defaults to `/usr/local`, but this
location can be overridden by running `make DESTDIR=/path/to/install install`).

## Running Houdini

To run Houdini using the HoudiniUsdBridge libraries, the following environment variables
must be set so that Houdini uses the bridge libraries and plugins instead of the ones that
ship with Houdini.

```
# The install location of your custom USD library.
export USD_ROOT=/path/to/USD
# The install location of the HoudiniUsdBridge libraries.
export BRIDGE_ROOT=/path/to/install/usr/local

# Ensure the bridge libHoudiniUSD.so library and the dummy libpxr_* libraries are used.
export LD_LIBRARY_PATH=$BRIDGE_ROOT/dsolib
# Give priority to the custom USD and bridge python libraries.
export PYTHONPATH=$USD_ROOT/lib/python:$BRIDGE_ROOT/houdini/python2.7libs
# Add the bridge houdini directory to the HOUDINI_PATH so that Houdini plugins will be
# loaded from here.
export HOUDINI_PATH=$BRIDGE_ROOT/houdini:\&
# Houdini will tell the USD library to load plugins explicitly from this directory.
# The default value of this variable loads USD plugins from the dso/usd_plugins subdirectory
# of every HOUDINI_PATH entry. But we explicitly don't want USD to try to load the plugins
# in $HFS/houdini/dso/usd_plugins.
export HOUDINI_USD_DSO_PATH=$BRIDGE_ROOT/houdini/dso/usd_plugins
# Tell Houdini to not load the USD related plugins that ship with Houdini.
export HOUDINI_DSO_EXCLUDE_PATTERN="{$HH/dso/USD_Ops.so} {$HH/dso/USD_SopVol.so}"
```

Note that using these environment variables, the actual Houdini installation does not need to
be modified at all. And removing the environment variables will therefore return Houdini to its
native state, using the built-in USD library.

## Other Considerations

The Houdini USD build has a number of directories automatically added to the
USD plugin path ($HDSO/usd_plugins and $HH/dso/usd_plugins). You may need to
add these explicitly to your USD plugin path environment variable to ensure
that the Houdini USD plugins are loaded by your USD build.

Always build the HoudiniUsdBridge in Release mode. Even RelWithDebInfo has been
reported to cause crashes and other bad behavior.

## Acknowledgements

The USD library on which these libraries are built is created by Pixar:
https://github.com/PixarAnimationStudios/USD
SideFX also maintains a fork of this repository:
https://github.com/sideeffects/USD

The code in the 'src/houdini/lib/H_USD/gusd' directory of this repository
began as a direct copy of the 'third_party/houdini/lib/gusd' directory from
this USD repository, and so still contains the original Pixar copyright
notices.
