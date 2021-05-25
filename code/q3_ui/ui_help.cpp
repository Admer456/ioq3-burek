#include "ui_local.hpp"

#define ART_BACK0			"menu/art/back_0"
#define ART_BACK1			"menu/art/back_1"
#define ART_HELP_ENG		"menu/art/help_eng"
#define ART_HELP_BOS		"menu/art/help_bos"
#define ART_LANG_ENG		"menu/art/lang_english"
#define ART_LANG_BOS		"menu/art/lang_bosnian"

#define ID_BACK 10
#define ID_LANG 11

struct help_t
{
	menuframework_s menu;
	
	menutext_s banner;

	menubitmap_s helpImage;
	menubitmap_s changeLang;
	menubitmap_s back;
};

static help_t s_help;

void UI_HelpMenu_Cache()
{
	trap_R_RegisterShaderNoMip( ART_BACK0 );
	trap_R_RegisterShaderNoMip( ART_BACK1 );
	trap_R_RegisterShaderNoMip( ART_HELP_ENG );
	trap_R_RegisterShaderNoMip( ART_HELP_BOS );
	trap_R_RegisterShaderNoMip( ART_LANG_ENG );
	trap_R_RegisterShaderNoMip( ART_LANG_BOS );
}

void UI_Help_ToggleLanguage()
{
	static bool isEnglish = true;

	if ( isEnglish )
	{
		// Change to Bosnian
		s_help.helpImage.generic.name = ART_HELP_BOS;
		s_help.changeLang.generic.name = ART_LANG_BOS;
		//s_help.changeLang.focuspic = ART_LANG_ENG;
	}
	else
	{
		// Change to English
		s_help.helpImage.generic.name = ART_HELP_ENG;
		s_help.changeLang.generic.name = ART_LANG_ENG;
		//s_help.changeLang.focuspic = ART_LANG_BOS;
	}

	s_help.helpImage.shader = 0;
	s_help.changeLang.shader = 0;

	isEnglish = !isEnglish;
}

void UI_Help_MenuEvent( void* ptr, int event )
{
	if ( event != QM_ACTIVATED )
	{
		return;
	}

	switch ( ((menucommon_s*)ptr)->id )
	{
	case ID_BACK:
		UI_PopMenu();
		break;

	case ID_LANG:
		UI_Help_ToggleLanguage();
		break;
	}
}

void UI_Help_MenuInit()
{
	UI_HelpMenu_Cache();

	memset( &s_help, 0, sizeof( help_t ) );
	s_help.menu.wrapAround = true;
	s_help.menu.fullscreen = true;

	s_help.banner.generic.type = MTYPE_BTEXT;
	s_help.banner.generic.x = 320;
	s_help.banner.generic.y = 16;
	s_help.banner.string = "HELP";
	s_help.banner.color = color_white;
	s_help.banner.style = UI_CENTER;

	// Help image
	// In no way on Earth am I gonna do this as a bunch of labels LOL
	s_help.helpImage.generic.type = MTYPE_BITMAP;
	s_help.helpImage.generic.name = ART_HELP_ENG;
	s_help.helpImage.generic.flags = QMF_INACTIVE;
	s_help.helpImage.generic.x = 0;
	s_help.helpImage.generic.y = 0;
	s_help.helpImage.width = 640;
	s_help.helpImage.height = 480;

	// Back button
	s_help.back.generic.type = MTYPE_BITMAP;
	s_help.back.generic.name = ART_BACK0;
	s_help.back.generic.flags = QMF_LEFT_JUSTIFY | QMF_PULSEIFFOCUS;
	s_help.back.generic.id = ID_BACK;
	s_help.back.generic.callback = UI_Help_MenuEvent;
	s_help.back.generic.x = 0;
	s_help.back.generic.y = 480 - 64;
	s_help.back.width = 128;
	s_help.back.height = 64;
	s_help.back.focuspic = ART_BACK1;

	// Language toggle button
	s_help.changeLang.generic.type = MTYPE_BITMAP;
	s_help.changeLang.generic.name = ART_LANG_ENG;
	s_help.changeLang.generic.flags = QMF_LEFT_JUSTIFY;
	s_help.changeLang.generic.id = ID_LANG;
	s_help.changeLang.generic.callback = UI_Help_MenuEvent;
	s_help.changeLang.generic.x = 128 + 16;
	s_help.changeLang.generic.y = 480 - 64;
	s_help.changeLang.width = 128;
	s_help.changeLang.height = 64;
	//s_help.changeLang.focuspic = ART_LANG_BOS;

	Menu_AddItem( &s_help.menu, &s_help.helpImage );
	Menu_AddItem( &s_help.menu, &s_help.banner );
	Menu_AddItem( &s_help.menu, &s_help.back );
	Menu_AddItem( &s_help.menu, &s_help.changeLang );
}

void UI_HelpMenu()
{
	UI_Help_MenuInit();
	UI_PushMenu( &s_help.menu );
}
