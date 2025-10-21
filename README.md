# HoudiniUsdBridge
Houdini libraries that must be recompiled to use Houdini with a custom build of
the USD library.

## Building USD on Linux

In order for the HoudiniUsdBridge to work, you must build USD with the boost
python library replacement that ships in the USD code base.

## SideFX Changes to the USD Library

The USD library that ships with Houdini 21.0 is forked from the official USD
25.05 release. Some SideFX-specific changes have since been applied to fix
bugs, or deal with SideFX-specific build issues. Whenever possible we submit
pull requests to have our changes integrated into the next USD release, but
sometimes this isn't possible. And in any case, these changes are not part of
the USD 25.05 release. This section lists every deviation of the SideFX USD
library from Pixar's 25.05 branch, to help you decide which of these changes
you may want or need to incorporate into your own USD build.

Changes ordered from oldest to newest at initial release of Houdini 21.0:
- [f090fe00a9fa26d2fdf59d759c6f83414480bb71](https://github.com/sideeffects/USD/commit/f090fe00a9fa26d2fdf59d759c6f83414480bb71):
  - **Not required**: Provide a flag to turn on the MATERIALX_BUILD_SHARED_LIBS define in libraries that build against MaterialX. This is required to enable defines within MaterialX code that allow importing symbols from shared libraries on Windows.


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
* COPY_HOUDINI_USD_PLUGINS: Whether to copy the $HH/dso/usd_plugins directory
  from the Houdini install into the HoudiniUsdBridge install tree. This defaults  to ON.

Once the libraries are built, run `cmake --install`. This will copy the bridge
libraries into the installation directory structure (defaults to `/usr/local`,
but this location can be overridden in the CMake config).

## Running Houdini

To run Houdini using the HoudiniUsdBridge libraries, the following environment
variables must be set so that Houdini uses the bridge libraries and plugins
instead of the ones that ship with Houdini.

```
# The install location of your custom USD library.
export USD_ROOT=/path/to/USD
# The install location of the HoudiniUsdBridge libraries.
export BRIDGE_ROOT=/path/to/install/usr/local

# Ensure the bridge libHoudiniUSD.so library and the dummy libpxr_*
# libraries are used.
export LD_LIBRARY_PATH=$BRIDGE_ROOT/dsolib
# Give priority to the custom USD and bridge python libraries.
export PYTHONPATH=$USD_ROOT/lib/python:$BRIDGE_ROOT/houdini/python2.7libs
# Add the bridge houdini directory to the HOUDINI_PATH so that Houdini
# plugins will be loaded from here.
export HOUDINI_PATH=$BRIDGE_ROOT/houdini:\&
# Houdini will tell the USD library to load plugins explicitly from this
# directory. The default value of this variable loads USD plugins from the
# dso/usd_plugins subdirectory of every HOUDINI_PATH entry. But we
# explicitly don't want USD to try to load the plugins in
# $HFS/houdini/dso/usd_plugins.
export HOUDINI_USD_DSO_PATH=$BRIDGE_ROOT/houdini/dso/usd_plugins
# Tell Houdini to not load the USD related plugins that ship with Houdini.
export HOUDINI_DSO_EXCLUDE_PATTERN="{$HH/dso/USD_Ops.so} {$HH/dso/USD_SopVol.so}"
```

Some additional modifications to the environment may be necessary to use
the Shot Build package, which is also part of the HoudiniUsdBridge.
Alternatively, the Shot Builder package can simply be disabled.

Note that using these environment variables, the actual Houdini installation
does not need to be modified at all. And removing the environment variables
will therefore return Houdini to its native state, using the built-in USD
library.

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
