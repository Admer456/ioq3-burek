newoption
{
	trigger = "disable-client",
	description = "Disable the ioquake3 project"
}

newoption
{
	trigger = "disable-server",
	description = "Disable the dedicated server project"
}

newoption
{
	trigger = "disable-baseq3",
	description = "Disable the baseq3 projects"
}

newoption
{
	trigger = "disable-missionpack",
	description = "Disable the missionpack projects"
}

newoption
{
	trigger = "disable-renderer-gl1",
	description = "Disable the OpenGL 1 renderer project"
}

newoption
{
	trigger = "disable-renderer-gl2",
	description = "Disable the OpenGL 2 renderer project"
}

local IOQ3_RENDERER_BGFX = path.join(path.getabsolute(".."), "ioq3-renderer-bgfx")

if os.isdir(IOQ3_RENDERER_BGFX) then
	newoption
	{
		trigger = "disable-renderer-bgfx",
		description = "Disable the bgfx renderer project"
	}
end

newoption
{
	trigger = "disable-game-dll",
	description = "Disable the game DLL projects"
}

newoption
{
	trigger = "disable-game-qvm",
	description = "Disable the game QVM projects"
}

newoption
{
	trigger = "disable-ogg",
	description = "Disable Ogg Opus and Vorbis support"
}

newoption
{
	trigger = "rename-baseq3",
	description = "Rename the baseq3 project",
	value = "NAME"
}

newoption
{
	trigger = "rename-missionpack",
	description = "Rename the missionpack project",
	value = "NAME"
}

newoption
{
	trigger = "standalone",
	description = "Remove the dependency on Q3A"
}

if _ACTION == nil then
	return
end

-- ioq3 did bad stuff -w-
-- so I changed it to ioq3-burek
local BUILD_PATH = path.join("build", _ACTION)
local IOQ3_PATH = path.join(path.getabsolute(".."), "ioq3-burek")

if not os.isdir(IOQ3_PATH) then
	print("ioquake3 not found at " .. IOQ3_PATH)
	os.exit()
end

local IOQ3_CODE_PATH = path.join(IOQ3_PATH, "code")
local CURL_PATH = path.join(IOQ3_CODE_PATH, "curl-7.54.0")
local JPEG_PATH = path.join(IOQ3_CODE_PATH, "jpeg-8c")
local OGG_PATH = path.join(IOQ3_CODE_PATH, "libogg-1.3.3")
local OPUS_PATH = path.join(IOQ3_CODE_PATH, "opus-1.2.1")
local OPUSFILE_PATH = path.join(IOQ3_CODE_PATH, "opusfile-0.9")
local SDL_PATH = path.join(path.getabsolute("."), "SDL2")
local VORBIS_PATH = path.join(IOQ3_CODE_PATH, "libvorbis-1.3.6")

if not _OPTIONS["disable-renderer-bgfx"] and os.isdir(IOQ3_RENDERER_BGFX) then
	dofile(path.join(IOQ3_RENDERER_BGFX, "renderer_bgfx.lua"))
end	

solution "ioquake3"
	language "C"
	location(BUILD_PATH)
	startproject "ioquake3"
	platforms { "x86", "x64" }
	configurations { "Release", "Debug" }
	defines { "_CRT_SECURE_NO_DEPRECATE" }
	configuration "platforms:x86"
		architecture "x86"
	configuration "platforms:x64"
		architecture "x64"
	configuration "Debug"
		optimize "Debug"
		defines { "_DEBUG" }
		symbols "On"
	configuration "Release"
		optimize "Full"
		defines "NDEBUG"
	configuration { "Debug", "x86" }
		targetdir(path.join(BUILD_PATH, "bin_x86_debug"))
	configuration { "Release", "x86" }
		targetdir(path.join(BUILD_PATH, "bin_x86"))
	configuration { "Debug", "x64" }
		targetdir(path.join(BUILD_PATH, "bin_x64_debug"))
	configuration { "Release", "x64" }
		targetdir(path.join(BUILD_PATH, "bin_x64"))
	configuration "x64"
		defines { "_WIN64", "__WIN64__" }
	
group "engine"

if not _OPTIONS["disable-client"] then
project "ioquake3"
	kind "WindowedApp"
	characterset "MBCS"
	configuration "x64"
		targetname "ioquake3.x86_64"
		defines "__x86_64__"
	configuration "x86"
		targetname "ioquake3.x86"
		defines "__i386__"
	configuration {}
	defines
	{
		"_WIN32",
		"WIN32",
		"_WINSOCK_DEPRECATED_NO_WARNINGS",
		"BOTLIB",
		"USE_CURL",
		"USE_CURL_DLOPEN",
		"USE_OPENAL",
		"USE_OPENAL_DLOPEN",
		"USE_RENDERER_DLOPEN",
		"USE_LOCAL_HEADERS"
	}
	if _OPTIONS["standalone"] then
		defines "STANDALONE"
	end
	files
	{
		path.join(IOQ3_CODE_PATH, "asm/ftola.asm"),
		path.join(IOQ3_CODE_PATH, "asm/snapvector.asm"),
		path.join(IOQ3_CODE_PATH, "botlib/*.c"),
		path.join(IOQ3_CODE_PATH, "botlib/*.h"),
		path.join(IOQ3_CODE_PATH, "client/*.c"),
		path.join(IOQ3_CODE_PATH, "client/*.h"),
		path.join(IOQ3_CODE_PATH, "qcommon/*.c"),
		path.join(IOQ3_CODE_PATH, "qcommon/*.h"),
		path.join(IOQ3_CODE_PATH, "sdl/sdl_input.c"),
		path.join(IOQ3_CODE_PATH, "sdl/sdl_snd.c"),
		path.join(IOQ3_CODE_PATH, "server/*.c"),
		path.join(IOQ3_CODE_PATH, "server/*.h"),
		path.join(IOQ3_CODE_PATH, "sys/con_log.c"),
		path.join(IOQ3_CODE_PATH, "sys/con_passive.c"),
		path.join(IOQ3_CODE_PATH, "sys/sys_autoupdater.c"),
		path.join(IOQ3_CODE_PATH, "sys/sys_main.c"),
		path.join(IOQ3_CODE_PATH, "sys/sys_win32.c"),
		path.join(IOQ3_CODE_PATH, "sys/*.h"),
		"res/win_manifest.manifest",
		"res/win_resource.rc",
		"res/win_resource.h"
	}
	excludes
	{
		path.join(IOQ3_CODE_PATH, "sys/win_resource.h")
	}
	vpaths
	{
		["*"] = IOQ3_CODE_PATH
	}
	configuration "x64"
		files { path.join(IOQ3_CODE_PATH, "asm/vm_x86_64.asm") }
	configuration {}
	excludes
	{
		path.join(IOQ3_CODE_PATH, "client/libmumblelink.*"),
		path.join(IOQ3_CODE_PATH, "qcommon/vm_armv7l.c"),
		path.join(IOQ3_CODE_PATH, "qcommon/vm_none.c"),
		path.join(IOQ3_CODE_PATH, "qcommon/vm_powerpc*.*"),
		path.join(IOQ3_CODE_PATH, "qcommon/vm_sparc.*"),
		path.join(IOQ3_CODE_PATH, "server/sv_rankings.c")
	}
	includedirs
	{
		path.join(IOQ3_CODE_PATH, "AL"),
		JPEG_PATH,
		CURL_PATH,
		path.join(IOQ3_CODE_PATH, "SDL2/include"),
		path.join(IOQ3_CODE_PATH, "zlib")
	}
	links
	{
		"user32",
		"advapi32",
		"winmm",
		"wsock32",
		"ws2_32",
		"OpenGL32",
		"psapi",
		"gdi32",
		-- Other projects
		"SDL2",
		"SDL2main",
		"zlib"
	}
	if not _OPTIONS["disable-ogg"] then
		defines
		{
			"USE_CODEC_OPUS",
			"USE_CODEC_VORBIS",
			"USE_VOIP"
		}
		includedirs
		{
			path.join(OGG_PATH, "include"),
			path.join(OPUS_PATH, "include"),
			path.join(OPUSFILE_PATH, "include"),
			path.join(VORBIS_PATH, "include")
		}
		links { "ogg", "opus", "vorbis" }
	end
	linkoptions "/SAFESEH:NO" -- for MSVC2012
	configuration { "x86", "**.asm" }
		buildmessage "Assembling..."
		buildcommands('ml /c /Zi /Fo"%{cfg.objdir}/%{file.basename}.asm.obj" "%{file.relpath}"')
		buildoutputs '%{cfg.objdir}/%{file.basename}.asm.obj'
	configuration { "x64", "**.asm" }
		buildmessage "Assembling..."
		buildcommands('ml64 /c /D idx64 /Zi /Fo"%{cfg.objdir}/%{file.basename}.asm.obj" "%{file.relpath}"')
		buildoutputs '%{cfg.objdir}/%{file.basename}.asm.obj'
end

if not _OPTIONS["disable-server"] then
project "ioq3ded"
	kind "ConsoleApp"
	characterset "MBCS"
	configuration "x64"
		targetname "ioq3ded.x86_64"
	configuration "x86"
		targetname "ioq3ded.x86"
	configuration {}
	defines
	{
		"_WINSOCK_DEPRECATED_NO_WARNINGS",
		"DEDICATED",
		"BOTLIB",
		"USE_VOIP",
		"USE_LOCAL_HEADERS"
	}
	if _OPTIONS["standalone"] then
		defines "STANDALONE"
	end
	files
	{
		path.join(IOQ3_CODE_PATH, "asm/ftola.asm"),
		path.join(IOQ3_CODE_PATH, "asm/snapvector.asm"),
		path.join(IOQ3_CODE_PATH, "botlib/*.c"),
		path.join(IOQ3_CODE_PATH, "botlib/*.h"),
		path.join(IOQ3_CODE_PATH, "null/null_client.c"),
		path.join(IOQ3_CODE_PATH, "null/null_input.c"),
		path.join(IOQ3_CODE_PATH, "null/null_snddma.c"),
		path.join(IOQ3_CODE_PATH, "qcommon/*.c"),
		path.join(IOQ3_CODE_PATH, "qcommon/*.h"),
		path.join(IOQ3_CODE_PATH, "server/*.c"),
		path.join(IOQ3_CODE_PATH, "server/*.h"),
		path.join(IOQ3_CODE_PATH, "sys/con_log.c"),
		path.join(IOQ3_CODE_PATH, "sys/con_win32.c"),
		path.join(IOQ3_CODE_PATH, "sys/sys_autoupdater.c"),
		path.join(IOQ3_CODE_PATH, "sys/sys_main.c"),
		path.join(IOQ3_CODE_PATH, "sys/sys_win32.c"),
		path.join(IOQ3_CODE_PATH, "sys/*.h"),
		"res/win_manifest.manifest",
		"res/win_resource.rc",
		"res/win_resource.h"
	}
	excludes
	{
		path.join(IOQ3_CODE_PATH, "sys/win_resource.h")
	}
	vpaths
	{
		["*"] = IOQ3_CODE_PATH
	}
	configuration "x64"
		files { path.join(IOQ3_CODE_PATH, "asm/vm_x86_64.asm") }
	configuration {}
	excludes
	{
		path.join(IOQ3_CODE_PATH, "qcommon/vm_armv7l.c"),
		path.join(IOQ3_CODE_PATH, "qcommon/vm_none.c"),
		path.join(IOQ3_CODE_PATH, "qcommon/vm_powerpc*.*"),
		path.join(IOQ3_CODE_PATH, "qcommon/vm_sparc.*"),
		path.join(IOQ3_CODE_PATH, "server/sv_rankings.c")
	}
	includedirs { path.join(IOQ3_CODE_PATH, "zlib") }
	links
	{
		"winmm",
		"wsock32",
		"ws2_32",
		"psapi",
		-- Other projects
		"zlib"
	}
	linkoptions "/SAFESEH:NO" -- for MSVC2012
	configuration { "x86", "**.asm" }
		buildmessage "Assembling..."
		buildcommands('ml /c /Zi /Fo"%{cfg.objdir}/%{file.basename}.asm.obj" "%{file.relpath}"')
		buildoutputs '%{cfg.objdir}/%{file.basename}.asm.obj'
	configuration { "x64", "**.asm" }
		buildmessage "Assembling..."
		buildcommands('ml64 /c /D idx64 /Zi /Fo"%{cfg.objdir}/%{file.basename}.asm.obj" "%{file.relpath}"')
		buildoutputs '%{cfg.objdir}/%{file.basename}.asm.obj'
end

group "renderer"

if not _OPTIONS["disable-renderer-gl1"] then
project "renderer_opengl1"
	kind "SharedLib"
	characterset "MBCS"
	configuration "x64"
		targetname "renderer_opengl1_x86_64"
	configuration "x86"
		targetname "renderer_opengl1_x86"
	configuration {}
	defines
	{
		"_WIN32",
		"WIN32",
		"_WINDOWS",
		"USE_INTERNAL_JPEG",
		"USE_RENDERER_DLOPEN",
		"USE_LOCAL_HEADERS"
	}
	if _OPTIONS["standalone"] then
		defines "STANDALONE"
	end
	files
	{
		path.join(IOQ3_CODE_PATH, "jpeg-8c/*.c"),
		path.join(IOQ3_CODE_PATH, "jpeg-8c/*.h"),
		path.join(IOQ3_CODE_PATH, "qcommon/q_math.c"),
		path.join(IOQ3_CODE_PATH, "qcommon/q_shared.c"),
		path.join(IOQ3_CODE_PATH, "qcommon/q_shared.h"),
		path.join(IOQ3_CODE_PATH, "qcommon/qcommon.h"),
		path.join(IOQ3_CODE_PATH, "qcommon/qfiles.h"),
		path.join(IOQ3_CODE_PATH, "qcommon/puff.c"),
		path.join(IOQ3_CODE_PATH, "qcommon/puff.h"),
		path.join(IOQ3_CODE_PATH, "qcommon/surfaceflags.h"),
		path.join(IOQ3_CODE_PATH, "renderergl1/*.c"),
		path.join(IOQ3_CODE_PATH, "renderergl1/*.h"),
		path.join(IOQ3_CODE_PATH, "renderercommon/*.c"),
		path.join(IOQ3_CODE_PATH, "renderercommon/*.h"),
		path.join(IOQ3_CODE_PATH, "sdl/sdl_gamma.c"),
		path.join(IOQ3_CODE_PATH, "sdl/sdl_glimp.c")
	}
	includedirs
	{
		path.join(IOQ3_CODE_PATH, "AL"),
		JPEG_PATH,
		CURL_PATH,
		path.join(IOQ3_CODE_PATH, "SDL2/include"),
		path.join(IOQ3_CODE_PATH, "zlib")
		
	}
	links
	{
		"OpenGL32",
		-- Other projects
		"SDL2",
		"zlib"
	}
	configuration "x64"
		buildoptions { "/wd\"4267\""} -- Silence size_t type conversion warnings
end

if not _OPTIONS["disable-renderer-gl2"] then
project "renderer_opengl2"
	kind "SharedLib"
	characterset "MBCS"
	configuration "x64"
		targetname "renderer_opengl2_x86_64"
	configuration "x86"
		targetname "renderer_opengl2_x86"
	configuration {}
	defines
	{
		"_WIN32",
		"WIN32",
		"USE_INTERNAL_JPEG",
		"USE_RENDERER_DLOPEN",
		"USE_LOCAL_HEADERS"
	}
	if _OPTIONS["standalone"] then
		defines "STANDALONE"
	end
	files
	{
		-- Name the stringified GLSL files explicitly (without * wildcard) so they're added to the project even when they don't exist yet
		path.join(BUILD_PATH, "dynamic/renderergl2/bokeh_fp.c"),
		path.join(BUILD_PATH, "dynamic/renderergl2/bokeh_vp.c"),
		path.join(BUILD_PATH, "dynamic/renderergl2/calclevels4x_fp.c"),
		path.join(BUILD_PATH, "dynamic/renderergl2/calclevels4x_vp.c"),
		path.join(BUILD_PATH, "dynamic/renderergl2/depthblur_fp.c"),
		path.join(BUILD_PATH, "dynamic/renderergl2/depthblur_vp.c"),
		path.join(BUILD_PATH, "dynamic/renderergl2/dlight_fp.c"),
		path.join(BUILD_PATH, "dynamic/renderergl2/dlight_vp.c"),
		path.join(BUILD_PATH, "dynamic/renderergl2/down4x_fp.c"),
		path.join(BUILD_PATH, "dynamic/renderergl2/down4x_vp.c"),
		path.join(BUILD_PATH, "dynamic/renderergl2/fogpass_fp.c"),
		path.join(BUILD_PATH, "dynamic/renderergl2/fogpass_vp.c"),
		path.join(BUILD_PATH, "dynamic/renderergl2/generic_fp.c"),
		path.join(BUILD_PATH, "dynamic/renderergl2/generic_vp.c"),
		path.join(BUILD_PATH, "dynamic/renderergl2/lightall_fp.c"),
		path.join(BUILD_PATH, "dynamic/renderergl2/lightall_vp.c"),
		path.join(BUILD_PATH, "dynamic/renderergl2/pshadow_fp.c"),
		path.join(BUILD_PATH, "dynamic/renderergl2/pshadow_vp.c"),
		path.join(BUILD_PATH, "dynamic/renderergl2/shadowfill_fp.c"),
		path.join(BUILD_PATH, "dynamic/renderergl2/shadowfill_vp.c"),
		path.join(BUILD_PATH, "dynamic/renderergl2/shadowmask_fp.c"),
		path.join(BUILD_PATH, "dynamic/renderergl2/shadowmask_vp.c"),
		path.join(BUILD_PATH, "dynamic/renderergl2/ssao_fp.c"),
		path.join(BUILD_PATH, "dynamic/renderergl2/ssao_vp.c"),
		path.join(BUILD_PATH, "dynamic/renderergl2/texturecolor_fp.c"),
		path.join(BUILD_PATH, "dynamic/renderergl2/texturecolor_vp.c"),
		path.join(BUILD_PATH, "dynamic/renderergl2/tonemap_fp.c"),
		path.join(BUILD_PATH, "dynamic/renderergl2/tonemap_vp.c"),
		path.join(IOQ3_CODE_PATH, "jpeg-8c/*.c"),
		path.join(IOQ3_CODE_PATH, "jpeg-8c/*.h"),
		path.join(IOQ3_CODE_PATH, "qcommon/q_math.c"),
		path.join(IOQ3_CODE_PATH, "qcommon/q_shared.c"),
		path.join(IOQ3_CODE_PATH, "qcommon/q_shared.h"),
		path.join(IOQ3_CODE_PATH, "qcommon/qcommon.h"),
		path.join(IOQ3_CODE_PATH, "qcommon/qfiles.h"),
		path.join(IOQ3_CODE_PATH, "qcommon/puff.c"),
		path.join(IOQ3_CODE_PATH, "qcommon/puff.h"),
		path.join(IOQ3_CODE_PATH, "qcommon/surfaceflags.h"),
		path.join(IOQ3_CODE_PATH, "renderergl2/*.c"),
		path.join(IOQ3_CODE_PATH, "renderergl2/*.h"),
		path.join(IOQ3_CODE_PATH, "renderergl2/glsl/*.glsl"),
		path.join(IOQ3_CODE_PATH, "renderercommon/*.c"),
		path.join(IOQ3_CODE_PATH, "renderercommon/*.h"),
		path.join(IOQ3_CODE_PATH, "sdl/sdl_gamma.c"),
		path.join(IOQ3_CODE_PATH, "sdl/sdl_glimp.c")
	}
	-- The stringified GLSL files cause virtual paths to be a little too deeply nested
	vpaths
	{
		["dynamic"] = path.join(BUILD_PATH, "dynamic/renderergl2/*.c"),
		["*"] = IOQ3_CODE_PATH
	}
	includedirs
	{
		path.join(IOQ3_CODE_PATH, "SDL2/include"),
		path.join(IOQ3_CODE_PATH, "libcurl"),
		path.join(IOQ3_CODE_PATH, "AL"),
		path.join(IOQ3_CODE_PATH, "zlib"),
		JPEG_PATH
	}
	links
	{
		"OpenGL32",
		-- Other projects
		"SDL2",
		"zlib"
	}
	configuration "x64"
		buildoptions { "/wd\"4267\""} -- Silence size_t type conversion warnings
	configuration {}
	configuration "**.glsl"
		buildmessage "Stringifying %{file.name}"
		buildcommands("cscript.exe \"" .. path.join(IOQ3_PATH, "misc/msvc/glsl_stringify.vbs") .. "\" //Nologo \"%{file.relpath}\" \"dynamic\\renderergl2\\%{file.basename}.c\"")
		buildoutputs(path.join(BUILD_PATH, "dynamic\\renderergl2\\%{file.basename}.c"))
end

if not _OPTIONS["disable-renderer-bgfx"] and os.isdir(IOQ3_RENDERER_BGFX) then
	rendererProject("ioq3", IOQ3_RENDERER_BGFX)
	includedirs(path.join(IOQ3_PATH, "code/SDL2/include"))
	links { "SDL2" }
end

function setupGameDllProject(mod, name)
	project(mod .. "_" .. name .. "_dll")
	kind "SharedLib"
	if _OPTIONS["standalone"] then
		defines "STANDALONE"
	end
	configuration { "Debug", "x86" }
		targetdir(path.join(BUILD_PATH, "bin_x86_debug/" .. mod))
		targetname(name .. "x86")
	configuration { "Release", "x86" }
		targetdir(path.join(BUILD_PATH, "bin_x86/" .. mod))
		targetname(name .. "x86")
	configuration { "Debug", "x64" }
		targetdir(path.join(BUILD_PATH, "bin_x64_debug/" .. mod))
		targetname(name .. "x86_64")
	configuration { "Release", "x64" }
		targetdir(path.join(BUILD_PATH, "bin_x64/" .. mod))
		targetname(name .. "x86_64")
	configuration {}
	links "winmm"
end

group "game_dll"

if not (_OPTIONS["disable-baseq3"] or _OPTIONS["disable-game-dll"]) then

setupGameDllProject(_OPTIONS["rename-baseq3"] or "baseq3", "cgame")
	files
	{
		path.join(IOQ3_CODE_PATH, "cgame/*.c"),
		path.join(IOQ3_CODE_PATH, "cgame/*.h"),
		path.join(IOQ3_CODE_PATH, "game/bg_*.c"),
		path.join(IOQ3_CODE_PATH, "game/bg_*.h"),
		path.join(IOQ3_CODE_PATH, "qcommon/q_math.c"),
		path.join(IOQ3_CODE_PATH, "qcommon/q_shared.c"),
		path.join(IOQ3_CODE_PATH, "qcommon/q_shared.h"),
		path.join(IOQ3_CODE_PATH, "qcommon/surfaceflags.h")
	}
	excludes
	{
		path.join(IOQ3_CODE_PATH, "cgame/cg_newdraw.c"),
		path.join(IOQ3_CODE_PATH, "game/bg_lib.*")
	}
	
setupGameDllProject(_OPTIONS["rename-baseq3"] or "baseq3", "qagame")
	files
	{
		path.join(IOQ3_CODE_PATH, "game/*.c"),
		path.join(IOQ3_CODE_PATH, "game/*.h"),
		path.join(IOQ3_CODE_PATH, "qcommon/q_math.c"),
		path.join(IOQ3_CODE_PATH, "qcommon/q_shared.c"),
		path.join(IOQ3_CODE_PATH, "qcommon/q_shared.h"),
		path.join(IOQ3_CODE_PATH, "qcommon/surfaceflags.h")
	}
	excludes
	{
		path.join(IOQ3_CODE_PATH, "game/bg_lib.*"),
		path.join(IOQ3_CODE_PATH, "game/g_rankings.c")
	}

setupGameDllProject(_OPTIONS["rename-baseq3"] or "baseq3", "ui")
	files
	{
		path.join(IOQ3_CODE_PATH, "game/bg_misc.c"),
		path.join(IOQ3_CODE_PATH, "q3_ui/*.c"),
		path.join(IOQ3_CODE_PATH, "q3_ui/*.h"),
		path.join(IOQ3_CODE_PATH, "qcommon/q_math.c"),
		path.join(IOQ3_CODE_PATH, "qcommon/q_shared.c"),
		path.join(IOQ3_CODE_PATH, "qcommon/q_shared.h"),
		path.join(IOQ3_CODE_PATH, "ui/ui_syscalls.c")
	}
	excludes
	{
		path.join(IOQ3_CODE_PATH, "q3_ui/ui_loadconfig.c"),
		path.join(IOQ3_CODE_PATH, "q3_ui/ui_login.c"),
		path.join(IOQ3_CODE_PATH, "q3_ui/ui_rankings.c"),
		path.join(IOQ3_CODE_PATH, "q3_ui/ui_rankstatus.c"),
		path.join(IOQ3_CODE_PATH, "q3_ui/ui_saveconfig.c"),
		path.join(IOQ3_CODE_PATH, "q3_ui/ui_signup.c"),
		path.join(IOQ3_CODE_PATH, "q3_ui/ui_specifyleague.c")
	}
end

if not (_OPTIONS["disable-missionpack"] or _OPTIONS["disable-game-dll"]) then

setupGameDllProject(_OPTIONS["rename-missionpack"] or "missionpack", "cgame")
	defines	{ "MISSIONPACK" }
	files
	{
		path.join(IOQ3_CODE_PATH, "cgame/*.c"),
		path.join(IOQ3_CODE_PATH, "cgame/*.h"),
		path.join(IOQ3_CODE_PATH, "game/bg_*.c"),
		path.join(IOQ3_CODE_PATH, "game/bg_*.h"),
		path.join(IOQ3_CODE_PATH, "qcommon/q_math.c"),
		path.join(IOQ3_CODE_PATH, "qcommon/q_shared.c"),
		path.join(IOQ3_CODE_PATH, "qcommon/q_shared.h"),
		path.join(IOQ3_CODE_PATH, "qcommon/surfaceflags.h"),
		path.join(IOQ3_CODE_PATH, "ui/ui_shared.*")
	}
	excludes
	{
		path.join(IOQ3_CODE_PATH, "game/bg_lib.*")
	}

setupGameDllProject(_OPTIONS["rename-missionpack"] or "missionpack", "qagame")
	defines	{ "MISSIONPACK" }
	files
	{
		path.join(IOQ3_CODE_PATH, "game/*.c"),
		path.join(IOQ3_CODE_PATH, "game/*.h"),
		path.join(IOQ3_CODE_PATH, "qcommon/q_math.c"),
		path.join(IOQ3_CODE_PATH, "qcommon/q_shared.c"),
		path.join(IOQ3_CODE_PATH, "qcommon/q_shared.h"),
		path.join(IOQ3_CODE_PATH, "qcommon/surfaceflags.h")
	}
	excludes
	{
		path.join(IOQ3_CODE_PATH, "game/bg_lib.*"),
		path.join(IOQ3_CODE_PATH, "game/g_rankings.c")
	}

setupGameDllProject(_OPTIONS["rename-missionpack"] or "missionpack", "ui")
	defines	{ "MISSIONPACK" }
	files
	{
		path.join(IOQ3_CODE_PATH, "game/bg_misc.c"),
		path.join(IOQ3_CODE_PATH, "game/bg_public.h"),
		path.join(IOQ3_CODE_PATH, "qcommon/q_math.c"),
		path.join(IOQ3_CODE_PATH, "qcommon/q_shared.c"),
		path.join(IOQ3_CODE_PATH, "qcommon/q_shared.h"),
		path.join(IOQ3_CODE_PATH, "ui/*.c"),
		path.join(IOQ3_CODE_PATH, "ui/*.h")
	}
	links { "odbc32", "odbccp32" }
end

group "game_qvm"

function gameQvmProject(_mod, _qvm, _defines, _files, _asmFile)
	project(_mod .. "_" .. _qvm .. "_qvm")
	kind "StaticLib"
	files(_files)
	links { "lcc", "q3asm", "q3cpp", "q3rcc" } -- build dependencies
	configuration "**.c"
		buildmessage "lcc %{file.name}"
		buildcommands("\"%{cfg.targetdir}\\lcc.exe\" " .. _defines .. " -Wo-lccdir=\"%{cfg.targetdir}\" -o \"%{cfg.objdir}\\%{file.basename}.asm\" \"%{file.relpath}\"")
		buildoutputs "%{cfg.objdir}\\%{file.basename}.asm"
	configuration {}
	local asmFiles = ""
	for _,v in pairs(_files) do
		local ext = path.getextension(v)
		if ext == ".c" then
			asmFiles = asmFiles .. path.getbasename(v) .. ".asm "
		end
	end
	asmFiles = asmFiles .. _asmFile
	postbuildcommands
	{
		"cd %{cfg.objdir}",
		"\"$(TargetDir)/q3asm.exe\" -o \"$(TargetDir)/" .. _mod .. "/vm/" .. _qvm .. "\" " .. asmFiles
	}
end

if not (_OPTIONS["disable-baseq3"] or _OPTIONS["disable-game-qvm"]) then
	local baseq3_cgame_files =
	{
		path.join(IOQ3_CODE_PATH, "cgame/cg_main.c"), -- must be first for q3asm
		path.join(IOQ3_CODE_PATH, "cgame/cg_consolecmds.c"),
		path.join(IOQ3_CODE_PATH, "cgame/cg_draw.c"),
		path.join(IOQ3_CODE_PATH, "cgame/cg_drawtools.c"),
		path.join(IOQ3_CODE_PATH, "cgame/cg_effects.c"),
		path.join(IOQ3_CODE_PATH, "cgame/cg_ents.c"),
		path.join(IOQ3_CODE_PATH, "cgame/cg_event.c"),
		path.join(IOQ3_CODE_PATH, "cgame/cg_info.c"),
		path.join(IOQ3_CODE_PATH, "cgame/cg_local.h"),
		path.join(IOQ3_CODE_PATH, "cgame/cg_localents.c"),
		path.join(IOQ3_CODE_PATH, "cgame/cg_marks.c"),
		path.join(IOQ3_CODE_PATH, "cgame/cg_particles.c"),
		path.join(IOQ3_CODE_PATH, "cgame/cg_players.c"),
		path.join(IOQ3_CODE_PATH, "cgame/cg_playerstate.c"),
		path.join(IOQ3_CODE_PATH, "cgame/cg_predict.c"),
		path.join(IOQ3_CODE_PATH, "cgame/cg_public.h"),
		path.join(IOQ3_CODE_PATH, "cgame/cg_scoreboard.c"),
		path.join(IOQ3_CODE_PATH, "cgame/cg_servercmds.c"),
		path.join(IOQ3_CODE_PATH, "cgame/cg_snapshot.c"),
		path.join(IOQ3_CODE_PATH, "cgame/cg_view.c"),
		path.join(IOQ3_CODE_PATH, "cgame/cg_weapons.c"),
		path.join(IOQ3_CODE_PATH, "game/bg_lib.c"),
		path.join(IOQ3_CODE_PATH, "game/bg_lib.h"),
		path.join(IOQ3_CODE_PATH, "game/bg_local.h"),
		path.join(IOQ3_CODE_PATH, "game/bg_misc.c"),
		path.join(IOQ3_CODE_PATH, "game/bg_pmove.c"),
		path.join(IOQ3_CODE_PATH, "game/bg_public.h"),
		path.join(IOQ3_CODE_PATH, "game/bg_slidemove.c"),
		path.join(IOQ3_CODE_PATH, "qcommon/q_math.c"),
		path.join(IOQ3_CODE_PATH, "qcommon/q_shared.c"),
		path.join(IOQ3_CODE_PATH, "qcommon/q_shared.h"),
		path.join(IOQ3_CODE_PATH, "qcommon/surfaceflags.h")
	}
	local baseq3_qagame_files =
	{
		path.join(IOQ3_CODE_PATH, "game/g_main.c"), -- must be first for q3asm
		path.join(IOQ3_CODE_PATH, "game/ai_chat.c"),
		path.join(IOQ3_CODE_PATH, "game/ai_chat.h"),
		path.join(IOQ3_CODE_PATH, "game/ai_cmd.c"),
		path.join(IOQ3_CODE_PATH, "game/ai_cmd.h"),
		path.join(IOQ3_CODE_PATH, "game/ai_dmnet.c"),
		path.join(IOQ3_CODE_PATH, "game/ai_dmnet.h"),
		path.join(IOQ3_CODE_PATH, "game/ai_dmq3.c"),
		path.join(IOQ3_CODE_PATH, "game/ai_dmq3.h"),
		path.join(IOQ3_CODE_PATH, "game/ai_main.c"),
		path.join(IOQ3_CODE_PATH, "game/ai_main.h"),
		path.join(IOQ3_CODE_PATH, "game/ai_team.c"),
		path.join(IOQ3_CODE_PATH, "game/ai_team.h"),
		path.join(IOQ3_CODE_PATH, "game/ai_vcmd.c"),
		path.join(IOQ3_CODE_PATH, "game/ai_vcmd.h"),
		path.join(IOQ3_CODE_PATH, "game/bg_lib.c"),
		path.join(IOQ3_CODE_PATH, "game/bg_lib.h"),
		path.join(IOQ3_CODE_PATH, "game/bg_local.h"),
		path.join(IOQ3_CODE_PATH, "game/bg_misc.c"),
		path.join(IOQ3_CODE_PATH, "game/bg_pmove.c"),
		path.join(IOQ3_CODE_PATH, "game/bg_public.h"),
		path.join(IOQ3_CODE_PATH, "game/bg_slidemove.c"),
		path.join(IOQ3_CODE_PATH, "game/chars.h"),
		path.join(IOQ3_CODE_PATH, "game/g_active.c"),
		path.join(IOQ3_CODE_PATH, "game/g_arenas.c"),
		path.join(IOQ3_CODE_PATH, "game/g_bot.c"),
		path.join(IOQ3_CODE_PATH, "game/g_client.c"),
		path.join(IOQ3_CODE_PATH, "game/g_cmds.c"),
		path.join(IOQ3_CODE_PATH, "game/g_combat.c"),
		path.join(IOQ3_CODE_PATH, "game/g_items.c"),
		path.join(IOQ3_CODE_PATH, "game/g_local.h"),
		path.join(IOQ3_CODE_PATH, "game/g_mem.c"),
		path.join(IOQ3_CODE_PATH, "game/g_misc.c"),
		path.join(IOQ3_CODE_PATH, "game/g_missile.c"),
		path.join(IOQ3_CODE_PATH, "game/g_mover.c"),
		path.join(IOQ3_CODE_PATH, "game/g_public.h"),
		path.join(IOQ3_CODE_PATH, "game/g_session.c"),
		path.join(IOQ3_CODE_PATH, "game/g_spawn.c"),
		path.join(IOQ3_CODE_PATH, "game/g_svcmds.c"),
		path.join(IOQ3_CODE_PATH, "game/g_target.c"),
		path.join(IOQ3_CODE_PATH, "game/g_team.c"),
		path.join(IOQ3_CODE_PATH, "game/g_team.h"),
		path.join(IOQ3_CODE_PATH, "game/g_trigger.c"),
		path.join(IOQ3_CODE_PATH, "game/g_utils.c"),
		path.join(IOQ3_CODE_PATH, "game/g_weapon.c"),
		path.join(IOQ3_CODE_PATH, "game/inv.h"),
		path.join(IOQ3_CODE_PATH, "game/match.h"),
		path.join(IOQ3_CODE_PATH, "game/syn.h"),
		path.join(IOQ3_CODE_PATH, "qcommon/q_math.c"),
		path.join(IOQ3_CODE_PATH, "qcommon/q_shared.c"),
		path.join(IOQ3_CODE_PATH, "qcommon/q_shared.h"),
		path.join(IOQ3_CODE_PATH, "qcommon/surfaceflags.h")
	}
	local baseq3_ui_files =
	{
		path.join(IOQ3_CODE_PATH, "q3_ui/ui_main.c"), -- must be first for q3asm
		path.join(IOQ3_CODE_PATH, "game/bg_lib.c"),
		path.join(IOQ3_CODE_PATH, "game/bg_lib.h"),
		path.join(IOQ3_CODE_PATH, "game/bg_misc.c"),
		path.join(IOQ3_CODE_PATH, "q3_ui/ui_addbots.c"),
		path.join(IOQ3_CODE_PATH, "q3_ui/ui_atoms.c"),
		path.join(IOQ3_CODE_PATH, "q3_ui/ui_cdkey.c"),
		path.join(IOQ3_CODE_PATH, "q3_ui/ui_cinematics.c"),
		path.join(IOQ3_CODE_PATH, "q3_ui/ui_confirm.c"),
		path.join(IOQ3_CODE_PATH, "q3_ui/ui_connect.c"),
		path.join(IOQ3_CODE_PATH, "q3_ui/ui_controls2.c"),
		path.join(IOQ3_CODE_PATH, "q3_ui/ui_credits.c"),
		path.join(IOQ3_CODE_PATH, "q3_ui/ui_demo2.c"),
		path.join(IOQ3_CODE_PATH, "q3_ui/ui_display.c"),
		path.join(IOQ3_CODE_PATH, "q3_ui/ui_gameinfo.c"),
		path.join(IOQ3_CODE_PATH, "q3_ui/ui_ingame.c"),
		path.join(IOQ3_CODE_PATH, "q3_ui/ui_loadconfig.c"),
		path.join(IOQ3_CODE_PATH, "q3_ui/ui_local.h"),
		path.join(IOQ3_CODE_PATH, "q3_ui/ui_menu.c"),
		path.join(IOQ3_CODE_PATH, "q3_ui/ui_mfield.c"),
		path.join(IOQ3_CODE_PATH, "q3_ui/ui_mods.c"),
		path.join(IOQ3_CODE_PATH, "q3_ui/ui_network.c"),
		path.join(IOQ3_CODE_PATH, "q3_ui/ui_options.c"),
		path.join(IOQ3_CODE_PATH, "q3_ui/ui_playermodel.c"),
		path.join(IOQ3_CODE_PATH, "q3_ui/ui_players.c"),
		path.join(IOQ3_CODE_PATH, "q3_ui/ui_playersettings.c"),
		path.join(IOQ3_CODE_PATH, "q3_ui/ui_preferences.c"),
		path.join(IOQ3_CODE_PATH, "q3_ui/ui_qmenu.c"),
		path.join(IOQ3_CODE_PATH, "q3_ui/ui_removebots.c"),
		path.join(IOQ3_CODE_PATH, "q3_ui/ui_saveconfig.c"),
		path.join(IOQ3_CODE_PATH, "q3_ui/ui_serverinfo.c"),
		path.join(IOQ3_CODE_PATH, "q3_ui/ui_servers2.c"),
		path.join(IOQ3_CODE_PATH, "q3_ui/ui_setup.c"),
		path.join(IOQ3_CODE_PATH, "q3_ui/ui_sound.c"),
		path.join(IOQ3_CODE_PATH, "q3_ui/ui_sparena.c"),
		path.join(IOQ3_CODE_PATH, "q3_ui/ui_specifyserver.c"),
		path.join(IOQ3_CODE_PATH, "q3_ui/ui_splevel.c"),
		path.join(IOQ3_CODE_PATH, "q3_ui/ui_sppostgame.c"),
		path.join(IOQ3_CODE_PATH, "q3_ui/ui_spskill.c"),
		path.join(IOQ3_CODE_PATH, "q3_ui/ui_startserver.c"),
		path.join(IOQ3_CODE_PATH, "q3_ui/ui_team.c"),
		path.join(IOQ3_CODE_PATH, "q3_ui/ui_teamorders.c"),
		path.join(IOQ3_CODE_PATH, "q3_ui/ui_video.c"),
		path.join(IOQ3_CODE_PATH, "qcommon/q_math.c"),
		path.join(IOQ3_CODE_PATH, "qcommon/q_shared.c"),
		path.join(IOQ3_CODE_PATH, "qcommon/q_shared.h")
	}
	gameQvmProject(_OPTIONS["rename-baseq3"] or "baseq3", "cgame", "-DCGAME", baseq3_cgame_files, path.join(IOQ3_CODE_PATH, "cgame/cg_syscalls.asm"))
	gameQvmProject(_OPTIONS["rename-baseq3"] or "baseq3", "qagame", "-DQAGAME", baseq3_qagame_files, path.join(IOQ3_CODE_PATH, "game/g_syscalls.asm"))
	gameQvmProject(_OPTIONS["rename-baseq3"] or "baseq3", "ui", "-DUI", baseq3_ui_files, path.join(IOQ3_CODE_PATH, "ui/ui_syscalls.asm"))
end

if not (_OPTIONS["disable-missionpack"] or _OPTIONS["disable-game-qvm"]) then
	local missionpack_cgame_files =
	{
		path.join(IOQ3_CODE_PATH, "cgame/cg_main.c"), -- must be first for q3asm
		path.join(IOQ3_CODE_PATH, "cgame/cg_consolecmds.c"),
		path.join(IOQ3_CODE_PATH, "cgame/cg_draw.c"),
		path.join(IOQ3_CODE_PATH, "cgame/cg_drawtools.c"),
		path.join(IOQ3_CODE_PATH, "cgame/cg_effects.c"),
		path.join(IOQ3_CODE_PATH, "cgame/cg_ents.c"),
		path.join(IOQ3_CODE_PATH, "cgame/cg_event.c"),
		path.join(IOQ3_CODE_PATH, "cgame/cg_info.c"),
		path.join(IOQ3_CODE_PATH, "cgame/cg_local.h"),
		path.join(IOQ3_CODE_PATH, "cgame/cg_localents.c"),
		path.join(IOQ3_CODE_PATH, "cgame/cg_marks.c"),
		path.join(IOQ3_CODE_PATH, "cgame/cg_newdraw.c"),
		path.join(IOQ3_CODE_PATH, "cgame/cg_particles.c"),
		path.join(IOQ3_CODE_PATH, "cgame/cg_players.c"),
		path.join(IOQ3_CODE_PATH, "cgame/cg_playerstate.c"),
		path.join(IOQ3_CODE_PATH, "cgame/cg_predict.c"),
		path.join(IOQ3_CODE_PATH, "cgame/cg_public.h"),
		path.join(IOQ3_CODE_PATH, "cgame/cg_scoreboard.c"),
		path.join(IOQ3_CODE_PATH, "cgame/cg_servercmds.c"),
		path.join(IOQ3_CODE_PATH, "cgame/cg_snapshot.c"),
		path.join(IOQ3_CODE_PATH, "cgame/cg_view.c"),
		path.join(IOQ3_CODE_PATH, "cgame/cg_weapons.c"),
		path.join(IOQ3_CODE_PATH, "game/bg_lib.c"),
		path.join(IOQ3_CODE_PATH, "game/bg_lib.h"),
		path.join(IOQ3_CODE_PATH, "game/bg_local.h"),
		path.join(IOQ3_CODE_PATH, "game/bg_misc.c"),
		path.join(IOQ3_CODE_PATH, "game/bg_pmove.c"),
		path.join(IOQ3_CODE_PATH, "game/bg_public.h"),
		path.join(IOQ3_CODE_PATH, "game/bg_slidemove.c"),
		path.join(IOQ3_CODE_PATH, "qcommon/q_math.c"),
		path.join(IOQ3_CODE_PATH, "qcommon/q_shared.c"),
		path.join(IOQ3_CODE_PATH, "qcommon/q_shared.h"),
		path.join(IOQ3_CODE_PATH, "qcommon/surfaceflags.h"),
		path.join(IOQ3_CODE_PATH, "ui/ui_shared.c"),
		path.join(IOQ3_CODE_PATH, "ui/ui_shared.h")
	}
	local missionpack_qagame_files =
	{
		path.join(IOQ3_CODE_PATH, "game/g_main.c"), -- must be first for q3asm
		path.join(IOQ3_CODE_PATH, "game/ai_chat.c"),
		path.join(IOQ3_CODE_PATH, "game/ai_chat.h"),
		path.join(IOQ3_CODE_PATH, "game/ai_cmd.c"),
		path.join(IOQ3_CODE_PATH, "game/ai_cmd.h"),
		path.join(IOQ3_CODE_PATH, "game/ai_dmnet.c"),
		path.join(IOQ3_CODE_PATH, "game/ai_dmnet.h"),
		path.join(IOQ3_CODE_PATH, "game/ai_dmq3.c"),
		path.join(IOQ3_CODE_PATH, "game/ai_dmq3.h"),
		path.join(IOQ3_CODE_PATH, "game/ai_main.c"),
		path.join(IOQ3_CODE_PATH, "game/ai_main.h"),
		path.join(IOQ3_CODE_PATH, "game/ai_team.c"),
		path.join(IOQ3_CODE_PATH, "game/ai_team.h"),
		path.join(IOQ3_CODE_PATH, "game/ai_vcmd.c"),
		path.join(IOQ3_CODE_PATH, "game/ai_vcmd.h"),
		path.join(IOQ3_CODE_PATH, "game/bg_lib.c"),
		path.join(IOQ3_CODE_PATH, "game/bg_lib.h"),
		path.join(IOQ3_CODE_PATH, "game/bg_local.h"),
		path.join(IOQ3_CODE_PATH, "game/bg_misc.c"),
		path.join(IOQ3_CODE_PATH, "game/bg_pmove.c"),
		path.join(IOQ3_CODE_PATH, "game/bg_public.h"),
		path.join(IOQ3_CODE_PATH, "game/bg_slidemove.c"),
		path.join(IOQ3_CODE_PATH, "game/chars.h"),
		path.join(IOQ3_CODE_PATH, "game/g_active.c"),
		path.join(IOQ3_CODE_PATH, "game/g_arenas.c"),
		path.join(IOQ3_CODE_PATH, "game/g_bot.c"),
		path.join(IOQ3_CODE_PATH, "game/g_client.c"),
		path.join(IOQ3_CODE_PATH, "game/g_cmds.c"),
		path.join(IOQ3_CODE_PATH, "game/g_combat.c"),
		path.join(IOQ3_CODE_PATH, "game/g_items.c"),
		path.join(IOQ3_CODE_PATH, "game/g_local.h"),
		path.join(IOQ3_CODE_PATH, "game/g_mem.c"),
		path.join(IOQ3_CODE_PATH, "game/g_misc.c"),
		path.join(IOQ3_CODE_PATH, "game/g_missile.c"),
		path.join(IOQ3_CODE_PATH, "game/g_mover.c"),
		path.join(IOQ3_CODE_PATH, "game/g_public.h"),
		path.join(IOQ3_CODE_PATH, "game/g_session.c"),
		path.join(IOQ3_CODE_PATH, "game/g_spawn.c"),
		path.join(IOQ3_CODE_PATH, "game/g_svcmds.c"),
		path.join(IOQ3_CODE_PATH, "game/g_target.c"),
		path.join(IOQ3_CODE_PATH, "game/g_team.c"),
		path.join(IOQ3_CODE_PATH, "game/g_team.h"),
		path.join(IOQ3_CODE_PATH, "game/g_trigger.c"),
		path.join(IOQ3_CODE_PATH, "game/g_utils.c"),
		path.join(IOQ3_CODE_PATH, "game/g_weapon.c"),
		path.join(IOQ3_CODE_PATH, "game/inv.h"),
		path.join(IOQ3_CODE_PATH, "game/match.h"),
		path.join(IOQ3_CODE_PATH, "game/syn.h"),
		path.join(IOQ3_CODE_PATH, "qcommon/q_math.c"),
		path.join(IOQ3_CODE_PATH, "qcommon/q_shared.c"),
		path.join(IOQ3_CODE_PATH, "qcommon/q_shared.h"),
		path.join(IOQ3_CODE_PATH, "qcommon/surfaceflags.h")
	}
	local missionpack_ui_files =
	{
		path.join(IOQ3_CODE_PATH, "ui/ui_main.c"), -- must be first for q3asm
		path.join(IOQ3_CODE_PATH, "game/bg_lib.c"),
		path.join(IOQ3_CODE_PATH, "game/bg_lib.h"),
		path.join(IOQ3_CODE_PATH, "game/bg_misc.c"),
		path.join(IOQ3_CODE_PATH, "game/bg_public.h"),
		path.join(IOQ3_CODE_PATH, "qcommon/q_math.c"),
		path.join(IOQ3_CODE_PATH, "qcommon/q_shared.c"),
		path.join(IOQ3_CODE_PATH, "qcommon/q_shared.h"),
		path.join(IOQ3_CODE_PATH, "ui/ui_atoms.c"),
		path.join(IOQ3_CODE_PATH, "ui/ui_gameinfo.c"),
		path.join(IOQ3_CODE_PATH, "ui/ui_local.h"),
		path.join(IOQ3_CODE_PATH, "ui/ui_players.c"),
		path.join(IOQ3_CODE_PATH, "ui/ui_public.h"),
		path.join(IOQ3_CODE_PATH, "ui/ui_shared.c"),
		path.join(IOQ3_CODE_PATH, "ui/ui_shared.h")
	}
	gameQvmProject(_OPTIONS["rename-missionpack"] or "missionpack", "cgame", "-DCGAME -DMISSIONPACK", missionpack_cgame_files, path.join(IOQ3_CODE_PATH, "cgame/cg_syscalls.asm"))
	gameQvmProject(_OPTIONS["rename-missionpack"] or "missionpack", "qagame", "-DQAGAME -DMISSIONPACK", missionpack_qagame_files, path.join(IOQ3_CODE_PATH, "game/g_syscalls.asm"))
	gameQvmProject(_OPTIONS["rename-missionpack"] or "missionpack", "ui", "-DUI -DMISSIONPACK", missionpack_ui_files, path.join(IOQ3_CODE_PATH, "ui/ui_syscalls.asm"))
end

group "lib"

if not (_OPTIONS["disable-client"] or _OPTIONS["disable-ogg"]) then
project "ogg"
	kind "StaticLib"
	characterset "MBCS"
	files { path.join(OGG_PATH, "src/*.c") }
	includedirs { path.join(OGG_PATH, "include") }

project "opus"
	kind "StaticLib"
	characterset "MBCS"
	defines { "OPUS_BUILD", "HAVE_LRINTF", "FLOATING_POINT", "USE_ALLOCA" }
	files
	{
		path.join(OPUS_PATH, "celt/*.c"),
		path.join(OPUS_PATH, "silk/*.c"),
		path.join(OPUS_PATH, "silk/float/*.c"),
		path.join(OPUS_PATH, "src/*.c"),
		path.join(OPUSFILE_PATH, "src/*.c")
	}
	includedirs
	{
		path.join(OGG_PATH, "include"),
		path.join(OPUS_PATH, "include"),
		path.join(OPUS_PATH, "celt"),
		path.join(OPUS_PATH, "silk"),
		path.join(OPUS_PATH, "silk/float"),
		path.join(OPUSFILE_PATH, "include")
	}
	buildoptions
	{
		"/wd\"4244\"" -- "conversion from 'x' to 'y', possible loss of data"
	}
	
project "vorbis"
	kind "StaticLib"
	characterset "MBCS"
	files { 	path.join(VORBIS_PATH, "lib/*.c") }
	includedirs
	{
		path.join(OGG_PATH, "include"),
		path.join(VORBIS_PATH, "include"),
		path.join(VORBIS_PATH, "lib")
	}
	buildoptions
	{
		"/wd\"4305\"", -- "truncation from 'x' to 'y'"
		"/wd\"4244\"" -- "conversion from 'x' to 'y', possible loss of data"
	}
end

if not (_OPTIONS["disable-client"] and _OPTIONS["disable-server"]) then
project "SDL2"
	characterset "MBCS"
	kind "SharedLib"
	files
	{
		path.join(SDL_PATH, "include/*.h"),
		path.join(SDL_PATH, "src/*.c"),
		path.join(SDL_PATH, "src/atomic/*.c"),
		path.join(SDL_PATH, "src/audio/*.c"),
		path.join(SDL_PATH, "src/audio/disk/*.c"),
		path.join(SDL_PATH, "src/audio/dummy/*.c"),
		path.join(SDL_PATH, "src/cpuinfo/*.c"),
		path.join(SDL_PATH, "src/dynapi/*.c"),
		path.join(SDL_PATH, "src/events/*.c"),
		path.join(SDL_PATH, "src/file/*.c"),
		path.join(SDL_PATH, "src/haptic/*.c"),
		path.join(SDL_PATH, "src/joystick/*.c"),
		path.join(SDL_PATH, "src/joystick/hidapi/*.c"),
		path.join(SDL_PATH, "src/libm/*.c"),
		path.join(SDL_PATH, "src/power/*.c"),
		path.join(SDL_PATH, "src/render/*.c"),
		path.join(SDL_PATH, "src/render/opengl/*.c"),
		path.join(SDL_PATH, "src/render/opengles2/*.c"),
		path.join(SDL_PATH, "src/render/software/*.c"),
		path.join(SDL_PATH, "src/sensor/*.c"),
		path.join(SDL_PATH, "src/sensor/dummy/*.c"),
		path.join(SDL_PATH, "src/stdlib/*.c"),
		path.join(SDL_PATH, "src/thread/*.c"),
		path.join(SDL_PATH, "src/thread/generic/SDL_syscond.c"),
		path.join(SDL_PATH, "src/timer/*.c"),
		path.join(SDL_PATH, "src/video/*.c"),
		path.join(SDL_PATH, "src/video/dummy/*.c"),
		path.join(SDL_PATH, "src/video/yuv2rgb/*.c"),
		-- windows specific
		path.join(SDL_PATH, "src/audio/directsound/*.c"),
		path.join(SDL_PATH, "src/audio/wasapi/*.c"),
		path.join(SDL_PATH, "src/audio/winmm/*.c"),
		path.join(SDL_PATH, "src/audio/xaudio2/*.c"),
		path.join(SDL_PATH, "src/core/windows/SDL_windows.c"),
		path.join(SDL_PATH, "src/core/windows/SDL_xinput.c"),
		path.join(SDL_PATH, "src/filesystem/windows/*.c"),
		path.join(SDL_PATH, "src/haptic/windows/*.c"),
		path.join(SDL_PATH, "src/hidapi/windows/*.c"),
		path.join(SDL_PATH, "src/joystick/windows/*.c"),
		path.join(SDL_PATH, "src/loadso/windows/*.c"),
		path.join(SDL_PATH, "src/power/windows/*.c"),
		path.join(SDL_PATH, "src/render/direct3d/*.c"),
		path.join(SDL_PATH, "src/timer/windows/*.c"),
		path.join(SDL_PATH, "src/thread/windows/*.c"),
		path.join(SDL_PATH, "src/video/windows/*.c"),
		path.join(SDL_PATH, "src/main/windows/version.rc")
	}
	includedirs
	{
		path.join(SDL_PATH, "include"),
		path.join(SDL_PATH, "src")
	}
	links
	{
		"imm32",
		"setupapi",
		"version",
		"winmm"
	}
	-- msvcrt linker errors - VS bug?
	configuration "Debug"
		links { "ucrtd", "vcruntimed" }
	configuration "Release"
		links { "ucrt" }

project "SDL2main"
	characterset "MBCS"
	kind "StaticLib"
	files { path.join(SDL_PATH, "src/main/windows/*.c") }
	includedirs { path.join(SDL_PATH, "include") }

project "zlib"
	characterset "MBCS"
	kind "StaticLib"
	files { path.join(IOQ3_CODE_PATH, "zlib/*.c"), path.join(IOQ3_CODE_PATH, "zlib/*.h") }
end

-- Don't build tools used to build QVMs if they aren't used.
if not ((_OPTIONS["disable-baseq3"] and _OPTIONS["disable-missionpack"]) or _OPTIONS["disable-game-qvm"]) then
group "tool"

project "lburg"
	kind "ConsoleApp"
	characterset "MBCS"
	defines { "WIN32" }
	files { path.join(IOQ3_CODE_PATH, "tools/lcc/lburg/*.c"), path.join(IOQ3_CODE_PATH, "tools/lcc/lburg/*.h") }

project "lcc"
	kind "ConsoleApp"
	characterset "MBCS"
	defines { "WIN32" }
	files
	{
		path.join(IOQ3_CODE_PATH, "tools/lcc/etc/bytecode.c"),
		path.join(path.getabsolute("."), "lcc/lcc.c")
	}
	buildoptions
	{
		"/wd\"4273\"", -- "inconsistent dll linkage" getpid
		"/wd\"4996\"" -- "The POSIX name for this item is deprecated. Instead, use the ISO C++ conformant name"
	}

project "q3asm"
	kind "ConsoleApp"
	characterset "MBCS"
	defines { "WIN32" }
	files { path.join(IOQ3_CODE_PATH, "tools/asm/*.c"), path.join(IOQ3_CODE_PATH, "tools/asm/*.h") }
	buildoptions { "/wd\"4273\""} -- "inconsistent dll linkage" strupr
	
project "q3cpp"
	kind "ConsoleApp"
	characterset "MBCS"
	defines { "WIN32" }
	files { path.join(IOQ3_CODE_PATH, "tools/lcc/cpp/*.c"), path.join(IOQ3_CODE_PATH, "tools/lcc/cpp/*.h") }
	buildoptions
	{
		"/wd\"4018\"", -- "signed/unsigned mismatch"
		"/wd\"4996\"" -- "The POSIX name for this item is deprecated. Instead, use the ISO C++ conformant name"
	}

project "q3rcc"
	kind "ConsoleApp"
	characterset "MBCS"
	defines { "WIN32" }
	files
	{
		path.join(IOQ3_CODE_PATH, "tools/lcc/src/*.c"),
		path.join(IOQ3_CODE_PATH, "tools/lcc/src/*.h"),
		path.join(IOQ3_CODE_PATH, "tools/lcc/src/dagcheck.md"),
		path.join(BUILD_PATH, "dynamic/dagcheck.c")
	}
	vpaths
	{
		["dynamic"] = path.join(BUILD_PATH, "dynamic/*.c"),
		["*"] = path.join(IOQ3_CODE_PATH, "tools/lcc/src")
	}
	includedirs { path.join(IOQ3_CODE_PATH, "tools/lcc/src") } -- for dagcheck.c
	links { "lburg" } -- build dependency
	buildoptions
	{
		"/wd\"4018\"", -- "signed/unsigned mismatch"
		"/wd\"4244\"" -- "conversion from 'x' to 'y', possible loss of data"
	}
	configuration "**.md"
		buildmessage "lburg %{file.basename}"
		buildcommands("\"" .. path.join("%{cfg.targetdir}", "lburg.exe") .. "\" \"%{file.relpath}\" > \"dynamic\\%{file.basename}.c\"")
		buildoutputs(path.join(BUILD_PATH, "dynamic\\%{file.basename}.c"))
end
