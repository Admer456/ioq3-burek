#include "ui_local.hpp"

#define ID_BACK 125
#define ID_PLAY_TUTORIAL 126
#define ID_PLAY_CAMPAIGN 127
#define ID_VIOLENCE 128

#define ART_CONFIRM_FRAME	"menu/art/cut_frame"

struct campaign_t
{
	menuframework_s menu;

	menubitmap_s frame; // 142, 117, 359, 256, ART_CONFIRM_FRAME

	menutext_s campaign;
	menutext_s tutorial;
	menutext_s back;

	menuradiobutton_s violence;
};

static campaign_t s;

void UI_CampaignEvent( void* ptr, int eventId )
{
	if ( eventId != QM_ACTIVATED )
	{
		return;
	}

	switch ( ((menucommon_s*)ptr)->id )
	{
	case ID_BACK:
		UI_PopMenu();
		break;

	case ID_PLAY_CAMPAIGN:
		UI_PopMenu();
		trap_Cmd_ExecuteText( EXEC_APPEND, "spmap sewers;" );
		break;

	case ID_PLAY_TUTORIAL:
		UI_PopMenu();
		trap_Cmd_ExecuteText( EXEC_APPEND, "spmap tutorial;" );
		break;

	case ID_VIOLENCE:
		menuradiobutton_s* v = (menuradiobutton_s*)ptr;
		trap_Cvar_Set( "g_violence", v->curvalue ? "1" : "0" );
		break;
	}
}

void UI_CampaignMenuInit()
{
	trap_R_RegisterShaderNoMip( ART_CONFIRM_FRAME );

	memset( &s, 0, sizeof( campaign_t ) );
	s.menu.wrapAround = true;
	s.menu.fullscreen = true;

	constexpr int labelX = 640 / 2;
	constexpr int labelY = 166;
	constexpr int style = UI_CENTER;

	// Window frame
	s.frame.generic.type = MTYPE_BITMAP;
	s.frame.generic.name = ART_CONFIRM_FRAME;
	s.frame.generic.flags = QMF_INACTIVE;
	s.frame.generic.x = 142;
	s.frame.generic.y = 117;
	s.frame.width = 359;
	s.frame.height = 256;

	// Campaign
	s.campaign.generic.type = MTYPE_PTEXT;
	s.campaign.generic.id = ID_PLAY_CAMPAIGN;
	s.campaign.generic.flags = QMF_CENTER_JUSTIFY | QMF_PULSEIFFOCUS;
	s.campaign.generic.x = labelX;
	s.campaign.generic.y = labelY;
	s.campaign.generic.callback = UI_CampaignEvent;
	s.campaign.string = "Campaign";
	s.campaign.style = style;
	s.campaign.color = color_yellow;

	// Tutorial
	s.tutorial.generic.type = MTYPE_PTEXT;
	s.tutorial.generic.id = ID_PLAY_TUTORIAL;
	s.tutorial.generic.flags = QMF_CENTER_JUSTIFY | QMF_PULSEIFFOCUS;
	s.tutorial.generic.x = labelX;
	s.tutorial.generic.y = labelY + 24;
	s.tutorial.generic.callback = UI_CampaignEvent;
	s.tutorial.string = "Tutorial";
	s.tutorial.style = style;
	s.tutorial.color = color_yellow;

	// Back
	s.back.generic.type = MTYPE_PTEXT;
	s.back.generic.id = ID_BACK;
	s.back.generic.flags = QMF_CENTER_JUSTIFY | QMF_PULSEIFFOCUS;
	s.back.generic.x = labelX;
	s.back.generic.y = labelY + 24*4;
	s.back.generic.callback = UI_CampaignEvent;
	s.back.string = "Back";
	s.back.style = style;
	s.back.color = color_white;

	// Violence option
	s.violence.generic.type = MTYPE_RADIOBUTTON;
	s.violence.generic.id = ID_VIOLENCE;
	s.violence.generic.flags = QMF_CENTER_JUSTIFY;
	s.violence.generic.x = labelX + 8;
	s.violence.generic.y = labelY + 24*2 + 8;
	s.violence.generic.name = "Violence";
	s.violence.generic.callback = UI_CampaignEvent;
	s.violence.curvalue = qfalse;

	Menu_AddItem( &s.menu, &s.frame );
	Menu_AddItem( &s.menu, &s.campaign );
	Menu_AddItem( &s.menu, &s.tutorial );
	Menu_AddItem( &s.menu, &s.back );
	Menu_AddItem( &s.menu, &s.violence );

	// What an unfriendly move, MTYPE_TEXT gets
	// the QMF_INACTIVE flag, time to reset it
	s.campaign.generic.flags &= ~QMF_INACTIVE;
	s.tutorial.generic.flags &= ~QMF_INACTIVE;
	s.back.generic.flags &= ~QMF_INACTIVE;
}

void UI_CampaignMenu()
{
	UI_CampaignMenuInit();
	UI_PushMenu( &s.menu );
}
