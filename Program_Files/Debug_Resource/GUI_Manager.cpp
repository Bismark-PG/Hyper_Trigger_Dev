/*==============================================================================

	Manage GUI Resource [GUI_Manager.h]

	Author : Choi HyungJoon

==============================================================================*/
#include "GUI_Manager.h"
#include "Light_Manager.h"
#include "Player.h"
#include "Debug_Camera.h"
#include "Player_Camera.h"
#include "Model.h"
#include "Debug_Collision.h"
#include "Shader_Manager.h"

using namespace DirectX;

void GUI_Init(HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* context)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // keyboard control
    //io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;      // docking..?

    ImGui::StyleColorsDark();

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(device, context);
}

void GUI_Final()
{
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

void GUI_Model_Editor(double FPS)
{
	ImGui::Begin("Debug Menu");

	ImGui::Text("FPS: %.1f", FPS);
	// ==========================================
	//			   [Player Control]
	// ==========================================
	if (ImGui::CollapsingHeader("Player Control", ImGuiTreeNodeFlags_DefaultOpen))
	{
		// 1. Position Control
		XMFLOAT3 currentPos = Player_Get_POS();
		float posArr[3] = { currentPos.x, currentPos.y, currentPos.z };

		if (ImGui::DragFloat3("Position", posArr, 0.1f, -250.0f, 250.0f))
		{
			currentPos = { posArr[0], posArr[1], posArr[2] };
			Player_Set_POS(currentPos);
		}

		if (ImGui::Button("Reset Position"))
		{
			XMFLOAT3 resetPos = { 0.0f, 5.0f, 0.0f };
			Player_Set_POS(resetPos);
		}

		ImGui::Separator();

		ImGui::Text("Camera Sights:");

		Player_Sights currentSight = Player_Camera_Get_Now_Sights();
		int sightMode = (int)currentSight;

		if (ImGui::RadioButton("Left (L)", sightMode == 0))
			Player_Camera_Set_Sights(Player_Sights::Left);
		ImGui::SameLine();

		if (ImGui::RadioButton("Middle (M)", sightMode == 1))
			Player_Camera_Set_Sights(Player_Sights::Middle);
		ImGui::SameLine();

		if (ImGui::RadioButton("Right (R)", sightMode == 2))
			Player_Camera_Set_Sights(Player_Sights::Right);

		ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.5f, 1.0f), "Current: %s",
			(sightMode == 0 ? "Left" : (sightMode == 1 ? "Middle" : "Right")));
	}
	// ==========================================
	//		    [Debug Render Settings]
	// ==========================================
	if (ImGui::CollapsingHeader("Debug Render Settings", ImGuiTreeNodeFlags_DefaultOpen))
	{
		bool Is_Debug_Draw = Debug_Draw_Get_State();

		if (ImGui::Checkbox("Show Collision Boxes", &Is_Debug_Draw))
		{
			Debug_Draw_Get_State(Is_Debug_Draw);
		}

		ImGui::SameLine();
		ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "[Hitbox Visible]");

		if (ImGui::Checkbox("Show Bone Nodes", &Is_Bone_AABB_Draw))
		{
			// Will Be Check In Model Bone Draw 
		}
		ImGui::SameLine();
		ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "[Bone Visible]");
	}
	// ==========================================
	//			  [Debug Camera Info]
	// ==========================================
	if (ImGui::CollapsingHeader("Debug Camera Info", ImGuiTreeNodeFlags_DefaultOpen))
	{
		// (1) Debug Camera Position
		XMFLOAT3 dcPos = Debug_Camera_Get_POS();
		float dcPosArr[3] = { dcPos.x, dcPos.y, dcPos.z };

		ImGui::TextColored(ImVec4(0.5f, 0.8f, 1.0f, 1.0f), "[ Debug Camera Transform ]");

		if (ImGui::DragFloat3("Cam Position", dcPosArr, 0.1f))
		{
			Debug_Camera_Set_Position({ dcPosArr[0], dcPosArr[1], dcPosArr[2] });
		}

		// (2) FOV Control
		float fovRad = Debug_Camera_Get_FOV();
		float fovDeg = XMConvertToDegrees(fovRad);

		// 1 ~ 179 Degree
		if (ImGui::DragFloat("FOV (Degree)", &fovDeg, 1.0f, 1.0f, 179.0f))
		{
			// Degree >> Radian 
			Debug_Camera_Set_FOV(XMConvertToRadians(fovDeg));
		}

		// (3) Vectors
		if (ImGui::TreeNode("Direction Vectors (Edit)"))
		{
			// Front
			XMFLOAT3 F = Debug_Camera_Get_Front();
			float FrontArr[3] = { F.x, F.y, F.z };
			if (ImGui::DragFloat3("Front", FrontArr, 0.01f, -1.0f, 1.0f))
			{
				Debug_Camera_Set_Front({ FrontArr[0], FrontArr[1], FrontArr[2] });
			}

			// Vertical
			XMFLOAT3 V = Debug_Camera_Get_Vertical();
			float VerticalArr[3] = { V.x, V.y, V.z };
			if (ImGui::DragFloat3("Vertical", VerticalArr, 0.01f, -1.0f, 1.0f))
			{
				Debug_Camera_Set_Vertical({ VerticalArr[0], VerticalArr[1], VerticalArr[2] });
			}

			// Horizon
			XMFLOAT3 H = Debug_Camera_Get_Horizon();
			float HorizonArr[3] = { H.x, H.y, H.z };
			if (ImGui::DragFloat3("Horizon", HorizonArr, 0.01f, -1.0f, 1.0f))
			{
				Debug_Camera_Set_Horizon({ HorizonArr[0], HorizonArr[1], HorizonArr[2] });
			}

			ImGui::TreePop();
		}
	}
	// ==========================================
	//			  [Player Animation]
	// ==========================================
	if (ImGui::CollapsingHeader("- Root Position Fix -", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Root Adjust");
		ImGui::DragFloat("Root Y Offset##Root", &g_Model_Root_Y, 0.1f, -200.0f, 200.0f);
	}

	// ===== Eyes =====
	if (ImGui::CollapsingHeader("- Eyes -", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Text("Left Eye");
		ImGui::SliderFloat("L Eye X (Pitch)", &g_L_Eye_X, -45.0f, 45.0f);
		ImGui::SliderFloat("L Eye Y (Yaw)", &g_L_Eye_Y, -45.0f, 45.0f);

		ImGui::Text("Right Eye");
		ImGui::SliderFloat("R Eye X (Pitch)", &g_R_Eye_X, -45.0f, 45.0f);
		ImGui::SliderFloat("R Eye Y (Yaw)", &g_R_Eye_Y, -45.0f, 45.0f);
		ImGui::Separator();
	}

	// ===== Left Arm =====
	if (ImGui::CollapsingHeader("- Left Arm -", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "[ Left Arm Control ]");

		ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Clavicle (Shoulder Bone)");
		ImGui::DragFloat("L-Clav X##ClavL", &g_L_Clavicle_X, 1.0f, -360.0f, 360.0f);
		ImGui::DragFloat("L-Clav Y##ClavL", &g_L_Clavicle_Y, 1.0f, -360.0f, 360.0f);
		ImGui::DragFloat("L-Clav Z##ClavL", &g_L_Clavicle_Z, 1.0f, -360.0f, 360.0f);
		ImGui::Separator();

		ImGui::Text("Axis Invert:");
		ImGui::SameLine(); ImGui::Checkbox("Inv X##ArmL", &g_L_Arm_Invert_X);
		ImGui::SameLine(); ImGui::Checkbox("Inv Y##ArmL", &g_L_Arm_Invert_Y);
		ImGui::SameLine(); ImGui::Checkbox("Inv Z##ArmL", &g_L_Arm_Invert_Z);

		ImGui::Separator();
		ImGui::Text("Upper Arm (Shoulder)");
		ImGui::DragFloat("L-Up X##ArmL", &g_L_Arm_Upper_X, 1.0f, -360.0f, 360.0f);
		ImGui::DragFloat("L-Up Y##ArmL", &g_L_Arm_Upper_Y, 1.0f, -360.0f, 360.0f);
		ImGui::DragFloat("L-Up Z##ArmL", &g_L_Arm_Upper_Z, 1.0f, -360.0f, 360.0f);

		ImGui::Text("Fore Arm (Elbow)");
		ImGui::DragFloat("L-Fore X##ArmL", &g_L_Arm_Fore_X, 1.0f, -360.0f, 360.0f);
		ImGui::DragFloat("L-Fore Y##ArmL", &g_L_Arm_Fore_Y, 1.0f, -360.0f, 360.0f);
		ImGui::DragFloat("L-Fore Z##ArmL", &g_L_Arm_Fore_Z, 1.0f, -360.0f, 360.0f);

		ImGui::Separator();
		ImGui::Text("Hand (Wrist)");
		ImGui::DragFloat("L-Hand X##HandL", &g_L_Hand_X, 1.0f, -360.0f, 360.0f);
		ImGui::DragFloat("L-Hand Y##HandL", &g_L_Hand_Y, 1.0f, -360.0f, 360.0f);
		ImGui::DragFloat("L-Hand Z##HandL", &g_L_Hand_Z, 1.0f, -360.0f, 360.0f);
	}

	// ===== Right Arm =====
	if (ImGui::CollapsingHeader("- Right Arm -", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "[ Right Arm Control ]");

		ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Clavicle (Shoulder Bone)");
		ImGui::DragFloat("R-Clav X##ClavR", &g_R_Clavicle_X, 1.0f, -360.0f, 360.0f);
		ImGui::DragFloat("R-Clav Y##ClavR", &g_R_Clavicle_Y, 1.0f, -360.0f, 360.0f);
		ImGui::DragFloat("R-Clav Z##ClavR", &g_R_Clavicle_Z, 1.0f, -360.0f, 360.0f);
		ImGui::Separator();

		ImGui::Text("Axis Invert:");
		ImGui::SameLine(); ImGui::Checkbox("Inv X##ArmR", &g_R_Arm_Invert_X);
		ImGui::SameLine(); ImGui::Checkbox("Inv Y##ArmR", &g_R_Arm_Invert_Y);
		ImGui::SameLine(); ImGui::Checkbox("Inv Z##ArmR", &g_R_Arm_Invert_Z);

		ImGui::Separator();
		ImGui::Text("Upper Arm (Shoulder)");
		ImGui::DragFloat("R-Up X##ArmR", &g_R_Arm_Upper_X, 1.0f, -360.0f, 360.0f);
		ImGui::DragFloat("R-Up Y##ArmR", &g_R_Arm_Upper_Y, 1.0f, -360.0f, 360.0f);
		ImGui::DragFloat("R-Up Z##ArmR", &g_R_Arm_Upper_Z, 1.0f, -360.0f, 360.0f);

		ImGui::Text("Fore Arm (Elbow)");
		ImGui::DragFloat("R-Fore X##ArmR", &g_R_Arm_Fore_X, 1.0f, -360.0f, 360.0f);
		ImGui::DragFloat("R-Fore Y##ArmR", &g_R_Arm_Fore_Y, 1.0f, -360.0f, 360.0f);
		ImGui::DragFloat("R-Fore Z##ArmR", &g_R_Arm_Fore_Z, 1.0f, -360.0f, 360.0f);

		ImGui::Separator();
		ImGui::Text("Hand (Wrist)");
		ImGui::DragFloat("R-Hand X##HandR", &g_R_Hand_X, 1.0f, -360.0f, 360.0f);
		ImGui::DragFloat("R-Hand Y##HandR", &g_R_Hand_Y, 1.0f, -360.0f, 360.0f);
		ImGui::DragFloat("R-Hand Z##HandR", &g_R_Hand_Z, 1.0f, -360.0f, 360.0f);
	}

	// ===== Fingers Control =====
	if (ImGui::CollapsingHeader("- Hands (Fingers) -", ImGuiTreeNodeFlags_DefaultOpen))
	{
		const char* FingerNames[] = { "Thumb", "Index", "Middle", "Ring", "Pinky" };

		// --- Left Hand Fingers ---
		if (ImGui::TreeNode("Left Hand Fingers"))
		{
			for (int f = 0; f < 5; ++f) // 5 Fingers
			{
				if (ImGui::TreeNode(FingerNames[f]))
				{
					for (int j = 0; j < 3; ++j) // 3 Joints
					{
						// Label: Joint 1, Joint 2, Joint 3
						std::string label = "Joint " + std::to_string(j + 1);
						ImGui::Text("%s", label.c_str());

						// Use PushID For ID Crash
						ImGui::PushID(f * 10 + j);

						ImGui::PushItemWidth(60);
						ImGui::DragFloat("X", &g_L_Finger[f][j][0], 1.0f, -90.0f, 90.0f); ImGui::SameLine();
						ImGui::DragFloat("Y", &g_L_Finger[f][j][1], 1.0f, -90.0f, 90.0f); ImGui::SameLine();
						ImGui::DragFloat("Z", &g_L_Finger[f][j][2], 1.0f, -90.0f, 90.0f);
						ImGui::PopItemWidth();

						ImGui::PopID(); // ID Pop
					}
					ImGui::TreePop();
				}
			}
			ImGui::TreePop();
		}

		ImGui::Separator();

		// --- Right Hand Fingers ---
		if (ImGui::TreeNode("Right Hand Fingers"))
		{
			for (int f = 0; f < 5; ++f)
			{
				if (ImGui::TreeNode(FingerNames[f]))
				{
					for (int j = 0; j < 3; ++j)
					{
						std::string label = "Joint " + std::to_string(j + 1);
						ImGui::Text("%s", label.c_str());

						// Right Fingers Use PushID Start 100 For Defense Crash Left Fingers 
						ImGui::PushID(100 + f * 10 + j);

						ImGui::PushItemWidth(60);
						ImGui::DragFloat("X", &g_R_Finger[f][j][0], 1.0f, -90.0f, 90.0f); ImGui::SameLine();
						ImGui::DragFloat("Y", &g_R_Finger[f][j][1], 1.0f, -90.0f, 90.0f); ImGui::SameLine();
						ImGui::DragFloat("Z", &g_R_Finger[f][j][2], 1.0f, -90.0f, 90.0f);
						ImGui::PopItemWidth();

						ImGui::PopID();
					}
					ImGui::TreePop();
				}
			}
			ImGui::TreePop();
		}
	}

	// ===== Left Leg =====
	if (ImGui::CollapsingHeader("- Left Leg -", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "[ Left Leg Control ]");

		ImGui::Text("Axis Invert:");
		ImGui::SameLine(); ImGui::Checkbox("Inv X##LegL", &g_L_Leg_Invert_X);
		ImGui::SameLine(); ImGui::Checkbox("Inv Y##LegL", &g_L_Leg_Invert_Y);
		ImGui::SameLine(); ImGui::Checkbox("Inv Z##LegL", &g_L_Leg_Invert_Z);

		ImGui::Separator();
		ImGui::Text("Upper Leg (Thigh)");
		ImGui::DragFloat("L-Up X##LegL", &g_L_Leg_Upper_X, 1.0f, -360.0f, 360.0f);
		ImGui::DragFloat("L-Up Y##LegL", &g_L_Leg_Upper_Y, 1.0f, -360.0f, 360.0f);
		ImGui::DragFloat("L-Up Z##LegL", &g_L_Leg_Upper_Z, 1.0f, -360.0f, 360.0f);

		ImGui::Text("Lower Leg (Calf)");
		ImGui::DragFloat("L-Low X##LegL", &g_L_Leg_Lower_X, 1.0f, -360.0f, 360.0f);
		ImGui::DragFloat("L-Low Y##LegL", &g_L_Leg_Lower_Y, 1.0f, -360.0f, 360.0f);
		ImGui::DragFloat("L-Low Z##LegL", &g_L_Leg_Lower_Z, 1.0f, -360.0f, 360.0f);

		ImGui::Text("Foot (Ankle)");
		ImGui::DragFloat("L-Foot X##LegL", &g_L_Foot_X, 1.0f, -360.0f, 360.0f);
		ImGui::DragFloat("L-Foot Y##LegL", &g_L_Foot_Y, 1.0f, -360.0f, 360.0f);
		ImGui::DragFloat("L-Foot Z##LegL", &g_L_Foot_Z, 1.0f, -360.0f, 360.0f);

		ImGui::Text("Toe");
		ImGui::DragFloat("L-Toe X##LegL", &g_L_Toe_X, 1.0f, -360.0f, 360.0f);
		ImGui::DragFloat("L-Toe Y##LegL", &g_L_Toe_Y, 1.0f, -360.0f, 360.0f);
		ImGui::DragFloat("L-Toe Z##LegL", &g_L_Toe_Z, 1.0f, -360.0f, 360.0f);
	}

	// ===== Right Leg =====
	if (ImGui::CollapsingHeader("- Right Leg -", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "[ Right Leg Control ]");

		ImGui::Text("Axis Invert:");
		ImGui::SameLine(); ImGui::Checkbox("Inv X##LegR", &g_R_Leg_Invert_X);
		ImGui::SameLine(); ImGui::Checkbox("Inv Y##LegR", &g_R_Leg_Invert_Y);
		ImGui::SameLine(); ImGui::Checkbox("Inv Z##LegR", &g_R_Leg_Invert_Z);

		ImGui::Separator();
		ImGui::Text("Upper Leg (Thigh)");
		ImGui::DragFloat("R-Up X##LegR", &g_R_Leg_Upper_X, 1.0f, -360.0f, 360.0f);
		ImGui::DragFloat("R-Up Y##LegR", &g_R_Leg_Upper_Y, 1.0f, -360.0f, 360.0f);
		ImGui::DragFloat("R-Up Z##LegR", &g_R_Leg_Upper_Z, 1.0f, -360.0f, 360.0f);

		ImGui::Text("Lower Leg (Calf)");
		ImGui::DragFloat("R-Low X##LegR", &g_R_Leg_Lower_X, 1.0f, -360.0f, 360.0f);
		ImGui::DragFloat("R-Low Y##LegR", &g_R_Leg_Lower_Y, 1.0f, -360.0f, 360.0f);
		ImGui::DragFloat("R-Low Z##LegR", &g_R_Leg_Lower_Z, 1.0f, -360.0f, 360.0f);

		ImGui::Text("Foot (Ankle)");
		ImGui::DragFloat("R-Foot X##LegR", &g_R_Foot_X, 1.0f, -360.0f, 360.0f);
		ImGui::DragFloat("R-Foot Y##LegR", &g_R_Foot_Y, 1.0f, -360.0f, 360.0f);
		ImGui::DragFloat("R-Foot Z##LegR", &g_R_Foot_Z, 1.0f, -360.0f, 360.0f);

		ImGui::Text("Toe");
		ImGui::DragFloat("R-Toe X##LegR", &g_R_Toe_X, 1.0f, -360.0f, 360.0f);
		ImGui::DragFloat("R-Toe Y##LegR", &g_R_Toe_Y, 1.0f, -360.0f, 360.0f);
		ImGui::DragFloat("R-Toe Z##LegR", &g_R_Toe_Z, 1.0f, -360.0f, 360.0f);
	}

	if (ImGui::CollapsingHeader("- Bone AABB BoX Scale -", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "AABB Scale");
		ImGui::DragFloat("AABB Scale", &Bone_AABB, 0.001f, -1.0f, 1.0f);
	}

	ImGui::Separator();
	if (ImGui::Button("Reset All Rotations (To Default)", ImVec2(-1, 30)))
	{
		Model_Node_Reset();
	}

	ImGui::End();
}

void GUI_World_Editor()
{
    ImGui::Begin("World System");

    if (ImGui::CollapsingHeader("Global Light Control", ImGuiTreeNodeFlags_DefaultOpen))
    {
		ImGui::Text("Sun Settings");
		
		// Manage Shadow Distance
		ImGui::SliderFloat("Sun Distance", &g_Sun_Dist, 10.0f, 500.0f);

		// Sun Auto Rotation
		ImGui::Separator();
		ImGui::Checkbox("Auto Rotation", &g_IsSunRotation);

		if (g_IsSunRotation)
		{
			// If Auto, Just Manage Speed, Tilt
			ImGui::SliderFloat("Rotation Speed", &g_Sun_Speed, 0.0f, 2.0f);
			ImGui::SliderFloat("Tilt (Z-Axis)", &g_Sun_Tilt, -1.0f, 1.0f);

			float Dir[3] = { g_Sun_Dir.x, g_Sun_Dir.y, g_Sun_Dir.z };
			ImGui::InputFloat3("Sun Dir (Auto)", Dir, "%.2f", ImGuiInputTextFlags_ReadOnly);
		}
		else
		{
			// If Not Auto, Can Manage Sun POS
			ImGui::Text("Manual Direction Control");
			ImGui::DragFloat3("Sun Direction (XYZ)", &g_Sun_Dir.x, 0.01f, -1.0f, 1.0f);
		}

		ImGui::Separator();

        // Manage Sun Light Color
		ImGui::Text("Light Color");
		ImGui::ColorEdit4("Sun Color", &g_Sun_Color.x);
		ImGui::ColorEdit4("Ambient Color", &g_Ambient_Color.x);
    }

	ImGui::Separator();
	if (ImGui::Button("Reset Global Light (Default)", ImVec2(-1, 30)))
	{
		Light_Manager::GetInstance().Global_Light_Reset();
	}

    // Map System
    if (ImGui::CollapsingHeader("Shadow Settings"))
    {
		if (ImGui::SliderFloat("Shadow Spread (Blur)", &PCF_Spread, 0.1f, 5.0f))
		{
			Shader_Manager::GetInstance()->SetShadowQuality(PCF_Spread, PCF_Loop);
		}

		if (ImGui::SliderInt("Sample Range (Quality)", &PCF_Loop, 1, 4))
		{
			Shader_Manager::GetInstance()->SetShadowQuality(PCF_Spread, PCF_Loop);
		}

		ImGui::Text("Samples: %d x %d = %d", (PCF_Loop * 2 + 1), (PCF_Loop * 2 + 1), (PCF_Loop * 2 + 1) * (PCF_Loop * 2 + 1));
    }

	ImGui::Separator();
	if (ImGui::Button("Reset PCF Parameters (Default)", ImVec2(-1, 30)))
	{
		Shader_Manager::GetInstance()->ResetShadowQuality();
		Shader_Manager::GetInstance()->SetShadowQuality(PCF_Spread, PCF_Loop);
	}

    ImGui::End();
}