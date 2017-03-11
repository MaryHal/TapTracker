
workspace "TapTracker"
configurations { "debug", "release" }

project "TapTracker"
kind "ConsoleApp"
language "C"
targetdir "bin/%{cfg.buildcfg}"

sysincludedirs {
   "ext/uthash/src",
   "ext/stb",
   "ext/incbin",
   "ext/flag",
   "ext/parson",
   "ext/zf_log/zf_log"
}

includedirs {}

libdirs {}

files {
   "src/**.c",
   "ext/flag/flag.c",
   "ext/parson/parson.c",
   "ext/zf_log/zf_log/zf_log.c"
}

buildoptions {
   "-Wall",
   "-Wextra",
   "-pedantic",
   "-std=gnu11"
}

filter "system:windows"
links {
   "opengl32",
   "glfw3",
   "m",
}

filter "system:not windows"
links {
   "GL",
   "glfw",
   "m",
   "rt"
}

filter "configurations:debug"
defines { "DEBUG", "ZF_LOG_DEF_LEVEL=ZF_LOG_VERBOSE" }
symbols "On"

filter "configurations:release"
defines { "NDEBUG", "ZF_LOG_DEF_LEVEL=ZF_LOG_WARN"  }
optimize "On"
