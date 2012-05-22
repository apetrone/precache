### Precache

Precache is a cross-platform program for application distribution over HTTP. It is inspired by Mojang's Minecraft and other similar systems.


The design goals are:

-	cross-platform among Linux, Windows, and MacOSX.
-	Ease of use. Config files are written in JSON.
-	little or no external dependencies ([xwl](http://github.com/apetrone/xwl) and soon [libcurl](http://github.com/bagder/curl))
-	favor the smallest binary possible. Fonts are embedded and a simple font renderer is included.

### Contributors
-	Jason Kozak

### Known Issues and Limitations

-	The project has a built-in font renderer to reduce external dependencies. At the moment, this requires fonts are exported in a custom format. The plan is to have a better system in its place eventually.
-	The User Interface is very simplistic - again to reduce external dependencies. The source is available, so a developer could always hook up their own GUI in place of the minimalistic one.
-	Currently, only connections through HTTP are supported. I have tested with the following servers: Apache 2.2.16 (linux), nginx 1.1.8 (windows), Lighttpd 1.4.10 (windows). A plan is in the works to move this to libcurl.
-	It is very important precache.py is executed on a unix-based platform when generating precache.list for unix platforms to maintain file permissions
-	XWLE_TEXT events on Linux only generate ASCII equivalent Unicode values.


### Building

The code should successfully build under Linux, Windows, and Mac OSX 10.6.x+

It uses the [Premake](http://industriousone.com/premake "premake") build system which allows me enough control while balancing maintenance of the premake script.
Simply run premake4 from the command line to generate your favorite project files.

I have tested the following project files under the respective OS.

#### Linux
-	Dependencies: mesa-common-dev libxrandr-dev libxinerama-dev libgl1-mesa-dev

>	premake4 codeblocks

#### Mac OSX
>	premake4 xcode3

#### Windows
>	premake4 vs2008/vs2010

##### Project dependency on xwl:
-	This is included in the precache source, but may one day be removed (and re-added as a submodule) as it has its own github project: http://github.com/apetrone/xwl/. Be aware that it is a dependency for window code.


### Usage

The basic usage is this:

You have a deployment folder that you want to distribute to users, players, testers, etc.

1.	Create a configuration file (see examples below) for your project. I'll refer to this as "project.conf", but you could name it anything.
2.	Run the precache.py script and specify the configuration file you just created.
3.	If you're supporting unix systems; run precache.py from a unix-based system. File permissions would be lost if you copy files and run precache.py from Windows.

>
>	python precache.py -f /path/to/project.conf
>

4.	Copy the resulting "precache.list" and your deployment folder to a webserver.
5.	Rebuild the precache binary for each platform you support and distribute this binary to your users.


### Customization

-	Application icon: edit the file pertaining to the associated platform.
	You can create new icons using png2ico (Windows), Icon Composer (Mac OSX).
	Simply replace the file and build the binary again.

-	Windows:
>	resources/precache.ico

-	Mac OSX:
>	resources/precache.icns

At the top of include/precachelib.h, there are several defines which are useful for further customization and testing.

### Configuration

Configuration files are written in JSON. I'll give a couple examples as I extend the features and try to explain each.

This is a simple config used to specify variables to precache. This assumes an application is for a single platform.

-	install_path: The folder to place downloaded content (on a user's machine)
-	remote_project_path: The path on the remote server where the content will be hosted (relative to the root of the server).
	1.	If precache pointed to "http://localhost", then files should be located in "http://localhost/build1234/"
-	local_project_path: Used for generating precache.list; the folder, on your deployment machine which runs precache.py, where the application content root is.
	1.	"project" is a folder which contains "precache.conf", and a folder: "deploy". Deploy is used as a root for content.
-	app: A list of settings used to generate include/vars.h, see precache.py for default values.
	1.	"url" specifies the remote download source, and a "window_title" is set.
	2.	Also available for configuration are various colors used by the application including button color, button text color, window background color, progress bar colors.
-	excludes: A list of ignores that are applied when walking the local_project_path


		{
			"install_path" : "appname",
			"remote_project_path" : "build1234",
			"local_project_path" : "deploy",
	
			"app" :
			{
				"url": "http://localhost/project/",
				"window_title": "Project"
			}
	
			"excludes" :
			[
				"*.dSYM/*",
				"*.DS_Store"
			]
		}


Here is another configuration file which is platform aware - meaning it only syncs the binaries for that platform.

deploy contains a folder "bin", which contains three folders, one for each platform "windows", "linux", "macosx".

-	binary_path: Used in this example to specify a binary root path which all binaries are synced to on the local machine.
	It is needed because relative paths are resolved and would ordinarily be placed in the paths listed in "binaries".
	Instead, we want them to appear in a more generic "bin" folder inside our project folder.

		{
			"install_path" : "appname",
			"remote_project_path" : "build1234",
			"local_project_path" : "deploy",
			"binary_path" : "bin",

			"app" :
			{
				"url": "http://localhost/project/",
				"window_title": "Project"
			}

			"excludes" :
			[
				"*.dSYM/*",
				"*.DS_Store"
			],

			"binaries" :
			{
				"windows" : "bin/windows"
				"linux" : "bin/linux",
				"macosx" : "bin/macosx"
			}
		}

