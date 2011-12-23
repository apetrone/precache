solution "precache"
configurations { "debug", "release" }

project "precache"
	objdir "obj"
	targetdir "build/bin"
	uuid( "e713b970-81b8-11e0-b278-0800200c9a66" )
	platforms{ "native", "x32", "x64" }

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

	baseExcludes = 
	{
	}

	excludes { baseExcludes }

	configuration {"windows"}
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

		defines { "WIN32", "UNICODE", baseDefines }
		links
		{
			"opengl32"
		}

	configuration {"macosx"}
		files
		{
			"src/**.m",
			"../xwl2/src/**.m"
		}

		links { "Cocoa.framework", "OpenGL.framework" }

	configuration {"linux"}
		defines { "LINUX=1", baseDefines }
		links
		{
			"Xinerama",
			"X11",
			"GL"
		}
	
	configuration { "debug" }
		targetsuffix "d"	
		defines { "DEBUG" }
		flags { "Symbols" }
	
	configuration { "macosx", "release" }
		postbuildcommands
		{
			"cp ./resources/Info.plist build/bin/precache.app/Contents/",
			"mkdir -p build/bin/precache.app/Contents/Resources/",
			"cp ./resources/precache.icns build/bin/precache.app/Contents/Resources/"
		}

	-- workaround for issue w/ premake not applying the suffix to the bundle
	configuration { "macosx", "debug", "gmake" }
		targetname "precached"
		targetsuffix ""

	configuration { "macosx", "debug" }
			postbuildcommands
			{
				"cp ./resources/Info.plist build/bin/precached.app/Contents/",
				"mkdir -p build/bin/precached.app/Contents/Resources/",
				"cp ./resources/precache.icns build/bin/precached.app/Contents/Resources/"
			}

	
