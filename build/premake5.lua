-- premake5.lua

if not os.isfile("jln_flags.lua") then
   http.download("https://raw.githubusercontent.com/jonathanpoelen/cpp-compiler-options/master/output/premake5", "jln_flags.lua")
end
local jln=dofileopt "jln_flags.lua"

function generate_options(jln_opt)
   if jln then
      jln_newoptions({warnings='on'})
      options=jln_getoptions(jln_opt)
      options.buildoptions = options.buildoptions
         :gsub('%-Wfloat%-equal', '')
         :gsub('%-Wswitch%-enum', '')
         :gsub('%-Wnoexcept', '')
         :gsub('%-Wsuggest%-attribute=noreturn', '')
      buildoptions(options.buildoptions)
      linkoptions(options.linkoptions)
      -- printf("cxx=%s\nlink=%s", options.buildoptions, options.linkoptions)
   else
      warnings "Extra"
   end
end

workspace "Cpp.js"
   configurations { "Debug", "Release", "Tests" }

project "Cpp.js"
   kind "ConsoleApp"
   language "C++"
   targetdir "bin/%{cfg.buildcfg}"
   cppdialect "C++17"
   flags { "MultiProcessorCompile" }

   includedirs { "../external", "../external/Catch2/single_include" }

   files { "../src/**.h", "../src/**.cpp" }

   filter "configurations:Tests"
      includedirs { "../src" }
      files { "../tests/**.h", "../tests/**.cpp" }
      symbols "On"
      generate_options {warnings='on', debug='on', glibcxx_debug='on'}

   filter "configurations:Debug"
      includedirs { "../src" }
      files { "../console/**.h", "../console/**.cpp" }
      symbols "On"
      generate_options {warnings='on', debug='on', glibcxx_debug='on'}

   filter "configurations:Release"
      includedirs { "../src" }
      files { "../console/**.h", "../console/**.cpp" }
      defines { "NDEBUG" }
      optimize "On"
      generate_options {warnings='on'}