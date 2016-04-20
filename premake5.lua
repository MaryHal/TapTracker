
workspace "TapTracker"
configurations { "debug", "release" }


project "TapTracker"
kind "ConsoleApp"
language "C"
targetdir "bin/%{cfg.buildcfg}"

includedirs {
   "ext/uthash/include",
   "ext/stb",
   "ext/incbin",
   "ext/flag",
   "ext/parson",
   "ext/zf_log/zf_log"
}

libdirs {}

links {
   "GL",
   "glfw",
   "m",
   "rt"
}

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

filter "configurations:debug"
defines { "DEBUG", "ZF_LOG_DEF_LEVEL=ZF_LOG_VERBOSE" }
flags { "Symbols" }

filter "configurations:release"
defines { "NDEBUG", "ZF_LOG_DEF_LEVEL=ZF_LOG_WARN"  }
optimize "On"
