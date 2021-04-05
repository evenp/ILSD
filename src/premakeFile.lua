
SrcDir = os.getcwd()
UsingQT = false;

function includeGlew()
	filter { "system:macosx" }
		libdirs("/usr/local/Cellar/glew/2.1.0_1/lib")	
		includedirs("/usr/local/Cellar/glew/2.1.0_1/include")
		links("libGLEW.2.1.0.dylib") --Placer entre parentheses le nom de la lib sur mac
	filter { }
end

function includeGlad()
	filter { "system:not macosx" }
		includedirs(SrcDir.."/../src/Libs/glad/include")
	filter { }
end

function includeImgui()
	includedirs(SrcDir.."/../src/Libs/imgui")
end

function includeGlfw()
	includedirs(SrcDir.."/../deps/glfw/include")
	filter { "system:not windows" }
		links { "glfw", "dl" }
	filter { "system:Windows", "configurations:Debug" }
		libdirs(SrcDir.."/../deps/glfw/build/Debug")
		links { "glfw3.lib" }
	filter { "system:Windows", "configurations:Release" }
		libdirs(SrcDir.."/../deps/glfw/build/Release")
		links { "glfw3.lib" }
	filter { }
end

function includeShapeLib()
	includedirs(SrcDir.."/../deps/shapelib")
	filter { "system:not windows" }
		links { "shp" }
	filter { "system:Windows", "configurations:Debug" }
		libdirs(SrcDir.."/../deps/shapelib/build/dll/Debug")
		links { "shp.lib" }
	filter { "system:Windows", "configurations:Release" }
		libdirs(SrcDir.."/../deps/shapelib/build/dll/Release")
		links { "shp.lib" }
	filter { }
end

function includeStbi()
	includedirs(SrcDir.."/../src/Libs/stbi")
end

workspace "ILSD"
	configurations { "Debug", "Release" }
	startproject "ILSD"
	architecture "x86_64"
	location (SrcDir.."/../")

project "ILSD"
	--project configuration
	kind ("ConsoleApp")
	language "C++"
	cppdialect "C++17"
	files { "**.cpp", "**.hpp", "**.h", "**.c", "**.cxx" }

	--vs paths
	targetdir (SrcDir.."/../binaries/".."%{prj.name}".."/".."%{cfg.longname}")
	objdir (SrcDir.."/../intermediate/".."%{prj.name}".."/".."%{cfg.longname}")
	debugdir(SrcDir.."/../resources")

	filter "configurations:Debug"
		defines { "DEBUG" }
		symbols "On"
	filter "configurations:Release"
		defines { "NDEBUG" }
		optimize "On"
	filter { }

	filter "system:windows"
		buildoptions { "/Ot", "/MP" }
	filter { }

	--Includes
	includedirs(SrcDir.."/ILSDInterface")
	includedirs(SrcDir.."/GLTools")
	includedirs(SrcDir.."/ASDetector")
	includedirs(SrcDir.."/BlurredSegment")
	includedirs(SrcDir.."/ConvexHull")
	includedirs(SrcDir.."/DirectionalScanner")
	includedirs(SrcDir.."/ImageTools")
	includedirs(SrcDir.."/PointCloud")
	includeImgui()
	includeGlfw()
	includeShapeLib()
	includeStbi()
	includeGlew()
	includeGlad()
