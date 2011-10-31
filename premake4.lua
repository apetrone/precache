solution "precache"
configurations { "debug", "release" }
	
targetOS = "unknown";

if _ACTION == "vs2005" or _ACTION == "vs2008" or _ACTION == "vs2010" then
	targetOS = "windows"
elseif _ACTION == "codeblocks" or _ACTION == "gmake" then
	targetOS = "linux"
elseif _ACTION == "xcode3" then
	targetOS = "macosx"
end

project "precache"
	objdir "obj"
	targetdir "build/bin"
	uuid( "e713b970-81b8-11e0-b278-0800200c9a66" )
	platforms{ "x32", "x64" }

	kind "WindowedApp"
	language ("C")
	baseDefines = {}
	
	files
	{
		"src/**.c",	
		"include/**.h",	
	}

	includedirs 
	{ 
		"include",
	}	

	if targetOS == "windows" then
		-- use static runtime on Windows
		flags { "StaticRuntime" }
		
		files
		{
			"resources/precache.rc",
			"resources/resource.h"
		}
		
		includedirs
		{
			"resources/"
		}
	end
	
	if targetOS == "macosx" then
		files
		{
			"src/**.m"
		}
	end

	
	baseExcludes = 
	{
	}
	
	excludes { baseExcludes }
	if targetOS == "windows" then
		defines { "WIN32", "UNICODE", baseDefines }
		
		links
		{
			"opengl32"
		}
	elseif targetOS == "linux" then
		defines { "LINUX=1", baseDefines }
		links
		{
			"Xinerama",
			"X11",
			"GL"
		}
	elseif targetOS == "macosx" then
		defines { "__MACH__", baseDefines }
		files
		{
			"../xwl2/src/**.m"
		}
		
		linkoptions
		{
			"-framework Cocoa",
			"-framework OpenGL"
		}
	end
	
	
	configuration { "release" }
		if targetOS == "macosx" then
			postbuildcommands
			{
				"cp ./resources/Info.plist build/bin/precache.app/Contents/",
				"mkdir -p build/bin/precache.app/Contents/Resources/",
				"cp ./resources/precache.icns build/bin/precache.app/Contents/Resources/"
			}
		end

	configuration { "debug" }
		if targetOS == "macosx" then
			postbuildcommands
			{
				"cp ./resources/Info.plist build/bin/precached.app/Contents/",
				"mkdir -p build/bin/precached.app/Contents/Resources/",
				"cp ./resources/precache.icns build/bin/precached.app/Contents/Resources/"
			}
		end
		targetsuffix "d"		
		defines { "DEBUG" }
		flags { "Symbols" }

	
