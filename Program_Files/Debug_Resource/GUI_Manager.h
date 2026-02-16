/*==============================================================================

	Manage GUI Resource [GUI_Manager.h]

	Author : Choi HyungJoon

==============================================================================*/
#ifndef GUI_MANAGER_H
#define GUI_MANAGER_H
#include "direct3d.h"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

void GUI_Init(HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* context);
void GUI_Final();

void GUI_Model_Editor(double FPS);
void GUI_World_Editor();

#endif // GUI_MANAGER_H
