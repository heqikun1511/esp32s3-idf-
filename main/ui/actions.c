#include "actions.h"
#include "screens.h"
#include "ui.h"

void action_to_menu(lv_event_t * e)
{
    loadScreen(SCREEN_ID_MENU);
}

void action_to_ami(lv_event_t * e)
{
    loadScreen(SCREEN_ID_AMI);
}

void action_to_acc(lv_event_t * e)
{
    loadScreen(SCREEN_ID_ACC_PAGE);
}

void action_to_acc_page(lv_event_t * e)
{
    loadScreen(SCREEN_ID_ACC_PAGE);
}

void action_to_foll_page(lv_event_t * e)
{
    loadScreen(SCREEN_ID_FOLL_PAGE);
}

void action_to_eight_page(lv_event_t * e)
{
    loadScreen(SCREEN_ID_EIGHT_PAGE);
}



