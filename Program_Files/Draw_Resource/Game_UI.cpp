/*==============================================================================

	Manage Draw Resource For Game UI [Game_UI.h]

	Author : Choi HyungJoon

==============================================================================*/
#include "Game_UI.h"
#include "Sprite.h"
#include "Texture_Manager.h"

int WIDTH = 1920;
int HEIGHT = 1080;

float centerX = 0;
float centerY = 0;
float HPX = 0;
float HPY = 0;

static float Size = 50.0f;

static int UI_Aim = -1;

static int UI_HP = -1;

static int UI_Gum = -1;
static int UI_Ammo = -1;

void Game_UI_Initialize()
{
	UI_Aim = Texture_M->GetID("Aim");

	UI_HP = Texture_M->GetID("HP");

	UI_Gum = -1;
	UI_Ammo = -1;

	centerX = (WIDTH * 0.5f)  - (Size * 0.5f);
	centerY = (HEIGHT * 0.5f) - (Size * 0.5f);

	HPX = WIDTH  - (Size * 10.0f);
	HPY = HEIGHT - (Size * 1.5f);
}

void Game_UI_Finalize()
{
}

void Game_UI_Draw()
{
	Sprite_Draw(UI_Aim, centerX, centerY, Size, Size, {}, { 1.0f, 1.0f, 1.0f, 1.0f });
	Sprite_Draw(UI_HP, HPX, HPY, Size * 10.0f, Size * 1.5f, {}, { 1.0f, 1.0f, 1.0f, 1.0f });
}
