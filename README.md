# Intro

Cycles for Max is a plugin that allows Blender's Cycles renderer to be used from within Autodesk 3ds Max.

# Building

Visual Studio solutions are located inside the `proj` directory. These are split up so there is one solution per version of Visual Studio, with each version of visual Studio supporting specific versions of 3ds Max. The version mapping is:

| Visual Studio Version | 3ds Max Versions |
| --------------------- | ---------------- |
| 2015                  | 2017-2019        |
| 2017                  | 2020-2022        |
| 2019                  | 2023             |

Before building, you will need to modify a couple variables in the `.vcxproj` files. To begin, locate the vcxpoj that corresponds to the version of 3ds Max you would like to build for. For this example I'll use 3ds Max 2023.

1. Open `proj/max2023/cyclesrender_2023.vcxproj` in a text editor.
2. Inside the section `<PropertyGroup Label="Globals">` find the line `<MaxSDK>C:\Dev\CyclesMax\maxsdk\2023</MaxSDK>`
3. Change the above line to point to the root of your 3ds Max SDK. The SDK version must exactly match the plugin version.
4. In that same section find the line `<MyDepsRoot>C:\Dev\CyclesMax\plugin\dep_build\v142r\</MyDepsRoot>`
5. Change the above line to point to a directory where you keep all headers and .lib files needed to build the plugin. Details on the expected layout of this directory are in the next section.

# Preparing Dependencies

The provided visual studio projects should be able to build with only the two modifications listed above if you have your dependencies configured and arranged correctly.

First, to configure and build your dependencies I recommend using the tool in the [cyclesformax-dependencies](https://github.com/jlwitthuhn/cyclesformax-dependencies) repo. After building, copy the provided `collect_plugin_deps.bat` batch file into the newly-created directory corresponding to your visual studio version such as `cyclesformax-dependencies/v142r` and run it to copy all required libraries and headers into a single `_collected` directory with the following layout.

```
├── include
│   ├── OpenEXR
│   ├── OpenImageIO
│   ├── boost
│   ├── oneapi
│   ├── shader_core
│   ├── shader_editor
│   ├── shader_graph
│   └── tbb
└── lib
    ├── Half-2_5.lib
    ├── Iex-2_5.lib
    ├── IlmImf-2_5.lib
    ├── IlmThread-2_5.lib
    ├── Imath-2_5.lib
    ├── OpenImageIO.lib
    ├── glfw3.lib
    ├── imgui.lib
    ├── jpeg-static.lib
    ├── libboost_filesystem-vc142-mt-x64-1_80.lib
    ├── libboost_regex-vc142-mt-x64-1_80.lib
    ├── libboost_system-vc142-mt-x64-1_80.lib
    ├── libboost_thread-vc142-mt-x64-1_80.lib
    ├── libglew32.lib
    ├── libpng16_static.lib
    ├── shader_editor.lib
    ├── tbb.lib
    ├── tiff.lib
    └── zlibstatic.lib
```

Now you will need to build Cycles itself. The code itself and build instructions can be found in the [cyclesformax-cycles](https://github.com/jlwitthuhn/cyclesformax-cycles) repo. Inside that `_collected` directory create two new directories to hold the cycles-specific headers and libraries. Name these `include_cycles` and `lib_cycles`. Pull the headers from the cycles source and the .lib files from the Cycles build directory and lay them out like this:

```
├── include_cycles
│   ├── atomic_ops.h
│   ├── bvh
│   ├── device
│   ├── graph
│   ├── intern
│   ├── kernel
│   ├── render
│   ├── subd
│   └── util
└── lib_cycles
    ├── cycles_bvh.lib
    ├── cycles_device.lib
    ├── cycles_graph.lib
    ├── cycles_kernel.lib
    ├── cycles_render.lib
    ├── cycles_subd.lib
    ├── cycles_util.lib
    ├── extern_clew.lib
    ├── extern_cuew.lib
    ├── extern_numaapi.lib
    └── extern_sky.lib
```

Now you should have a single directory with all your dependencies that has `include`, `lib`, `include_cycles`, and `lib_cycles` in its top level. You can set this directory as `<MyDepsRoot>` in the project file.
