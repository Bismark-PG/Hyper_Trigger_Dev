/*==============================================================================

	Make Camera [Player_Camera.cpp]

	Author : Choi HyungJoon

==============================================================================*/
#include "Player_Camera.h"
#include "Player.h"
#include <DirectXMath.h>
#include "Shader_Manager.h"
#include "direct3d.h"
#include "Player.h"       
#include "KeyLogger.h"
#include <algorithm>
#include <debug_ostream.h>
#include "Resource_Manager.h"
#include "Setting.h"

using namespace DirectX;

// Controller Constants
constexpr float CONTROLLER_STICK_MAX = 32767.0f; // Max Value of Short
constexpr float CONTROLLER_STICK_WEIGHT = 800.0f;   // Pad Input Multiplier (Higher = Faster)
constexpr float CAMERA_ROTATION_SCALE = 0.005f;

#define XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE 8689

// Player Camera Parameter
static float Camera_Yaw = 0.0f, Camera_Pitch = 0.0f;
static float Camera_Near_Z = 0.1f, Camera_Far_z = 1000.0f;
constexpr float Change_Lerp_Speed = 15.0f;
constexpr float Jump_Dist = 1.5f, Run_Dist = 1.2f;

// Normal State (Base)
static const float Dist_Normal = 3.5f;
static const float FOV_Normal = XMConvertToRadians(70.0f);
static const float Height_Normal = 2.5f;
static const float Offset_Width_Normal = 1.5f;

// Aim State (ADS)
static const float Dist_Aim = 2.0f;  // Close Up
static const float FOV_Aim = XMConvertToRadians(60.0f); // Zoom In
static const float Sens_Mult_Aim = 0.5f; // 50% Sensitivity
static const float Height_Aim = 1.8f;
static const float Offset_Width_Aim = 1.0f;

// Current Values
static float Camera_Distance = Dist_Normal;
static float Camera_FOV = FOV_Normal;
static float Camera_Height = Height_Normal;

// Target Values For Lerp
static float Target_Dist = Dist_Normal;
static float Target_FOV = FOV_Normal;
static float Target_Sens_Mult = 1.0f; // Multiplier for Sensitivity
static float Target_Height = Height_Normal;
static float Target_Offset = 0.0f;
static bool Is_Aiming_Mode = false;

// Sensitivity
static float Mouse_Sensitivity = 0.01f; // Base Setting (From Option)
static float Apply_Sensitivity = 0.01f; // Real used value (Lerped)

// --- Vectors & Matrices ---
static XMFLOAT4X4 Camera_View_mtx = {};
static XMFLOAT3 Camera_POS = {};
static XMFLOAT3 Camera_Front = {};
static XMFLOAT3 Current_Camera_Pos = {};

// --- Sights (Shoulder View) ---
static Player_Sights Current_Sights = {};
static float Camera_Sights_Offset = {};

// --- Modes ---
static bool Is_Menu_Mode = false;
static XMFLOAT3 Menu_Cam_Pos = { 0.0f, 0.0f, 0.0f };
static XMFLOAT3 Menu_Cam_Target = { 0.0f, 0.0f, 0.0f };

void Player_Camera_Initialize()
{
    Camera_POS = { 0.0f, 0.0f, 0.0f };
    Camera_Front = { 0.0f, 0.0f, 1.0f };

    if (Setting_Get_View_Type())
    {
        Player_Camera_Set_Sights(Player_Sights::Right);
    }
    else
    {
        Player_Camera_Set_Sights(Player_Sights::Left);
    }

    Current_Sights = Player_Sights::Right;
    Camera_Sights_Offset = 1.5f;

    Current_Camera_Pos = { 0.0f, 0.0f, 0.0f };

    // Default Sensitivity Init
    Mouse_Sensitivity = 0.01f;
    Apply_Sensitivity = Mouse_Sensitivity;

    // Default Targets
    Target_Dist = Dist_Normal;
    Target_FOV = FOV_Normal;
    Target_Sens_Mult = 1.0f;
    Target_Offset = (Current_Sights == Player_Sights::Left) ? -Offset_Width_Normal : Offset_Width_Normal;
    Camera_Sights_Offset = Target_Offset;

    Is_Aiming_Mode = false;
}

void Player_Camera_Finalize()
{
}

void Player_Camera_Reset()
{
    Camera_Yaw = 0.0f;
    Camera_Pitch = 0.0f;

    Apply_Sensitivity = Mouse_Sensitivity;

    Target_Dist = Dist_Normal;
    Target_FOV = FOV_Normal;
    Target_Sens_Mult = 1.0f;
    Target_Height = Height_Normal;
    Target_Offset = (Current_Sights == Player_Sights::Left) ? -Offset_Width_Normal : Offset_Width_Normal;

    Camera_Distance = Dist_Normal;
    Camera_FOV = FOV_Normal;
    Camera_Height = Height_Normal;
    Camera_Sights_Offset = Target_Offset;
}

void Player_Camera_Update(double elapsed_time)
{
    float dt = static_cast<float>(elapsed_time);

    // --- Menu Mode Logic ---
    if (Is_Menu_Mode)
    {
        XMVECTOR P = XMLoadFloat3(&Menu_Cam_Pos);
        XMVECTOR T = XMLoadFloat3(&Menu_Cam_Target);
        XMVECTOR U = { 0.0f, 1.0f, 0.0f };

        // Make Matrix View
        XMMATRIX View = XMMatrixLookAtLH(P, T, U);
        XMStoreFloat4x4(&Camera_View_mtx, View);

        // Shader Update
        Shader_Manager::GetInstance()->SetViewMatrix3D(View);

        // Camera POS
        Camera_POS = Menu_Cam_Pos;
        XMStoreFloat3(&Camera_Front, XMVector3Normalize(T - P));

        // Set Shader Projection
        float Aspect = static_cast<float>(Direct3D_GetBackBufferWidth()) 
                      / static_cast<float>(Direct3D_GetBackBufferHeight());
        XMMATRIX Proj = XMMatrixPerspectiveFovLH(Camera_FOV, Aspect, Camera_Near_Z, Camera_Far_z);
        Shader_Manager::GetInstance()->SetProjectionMatrix3D(Proj);

        return;
    }

    if (Target_FOV == FOV_Normal)
    {
        bool Is_Right_Set = Setting_Get_View_Type();

        if (Is_Right_Set)
        {
            Player_Camera_Set_Sights(Player_Sights::Right);
        }
        else
        {
            Player_Camera_Set_Sights(Player_Sights::Left);
        }
    }

    // --- ADS (Aim) Interpolation ---
    float Lerp_Speed = Change_Lerp_Speed * dt;

    if (Is_Aiming_Mode)
    {
        Target_Dist = Dist_Aim;
	}
    else
    {
        if (Is_Player_Jump())
        {
            Target_Dist = Dist_Normal * Jump_Dist;
        }
        else if (Is_Plyer_Run())
        {
            Target_Dist = Dist_Normal * Run_Dist;
        }
        else
        {
            Target_Dist = Dist_Normal;
        }
    }
    // Distance Lerp
    Camera_Distance += (Target_Dist - Camera_Distance) * Lerp_Speed;

    // FOV Lerp
    Camera_FOV += (Target_FOV - Camera_FOV) * Lerp_Speed;

    // Height Lerp
    Camera_Height += (Target_Height - Camera_Height) * Lerp_Speed;

	// Sights Offset Lerp
    Camera_Sights_Offset += (Target_Offset - Camera_Sights_Offset) * Lerp_Speed;

    // Sensitivity Lerp
    // Calculate Target Real Sensitivity (Base Setting * Multiplier)
    float Target_Real_Sens = Mouse_Sensitivity * Target_Sens_Mult;
    Apply_Sensitivity += (Target_Real_Sens - Apply_Sensitivity) * Lerp_Speed;

    // --- Rotation Logic ---
    // Get Mouse Movement
    float mouseMoveX = KeyLogger_GetMouse_MoveX() * Apply_Sensitivity;
    float mouseMoveY = KeyLogger_GetMouse_MoveY() * Apply_Sensitivity;

    //Get Controller Input (Right Stick)
    if (XKeyLogger_IsControllerInput())
    {
        XMFLOAT2 RightStick = XKeyLogger_GetRightStick();
        float padMoveX = 0.0f;
        float padMoveY = 0.0f;

        // Check Deadzone
        if (abs(RightStick.x) > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE || abs(RightStick.y) > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
        {
            // Normalize (0.0 ~ 1.0)
            float normX = RightStick.x / CONTROLLER_STICK_MAX;
            float normY = RightStick.y / CONTROLLER_STICK_MAX;

            // Apply Weight & Time & Shared Sensitivity
            padMoveX = normX * CONTROLLER_STICK_WEIGHT * dt * Apply_Sensitivity;
            padMoveY = normY * CONTROLLER_STICK_WEIGHT * dt * Apply_Sensitivity;
        }

        // Add Angle
        Camera_Yaw += padMoveX;
        Camera_Pitch -= padMoveY;
    }
    else
    {
        // Add Angle
        Camera_Yaw += mouseMoveX;
        Camera_Pitch += mouseMoveY;
    }

    // Limited Pitch 
    Camera_Pitch = std::max(-XM_PIDIV2 * 0.8f, std::min(Camera_Pitch, XM_PIDIV2 * 0.8f)); // -80 ~ +80

    // Make Rotation
    XMMATRIX rotation = XMMatrixRotationRollPitchYaw(Camera_Pitch, Camera_Yaw, 0);

    // --- Position Calculation ---
    // Get Player POS
    XMVECTOR playerPos = XMLoadFloat3(&Player_Get_POS());

    // Set Camera FOV, POS
    XMVECTOR baseTargetPos = playerPos + XMVectorSet(0.0f, Camera_Height, 0.0f, 0.0f);
    XMVECTOR offset = XMVectorSet(0.0f, 0.0f, -Camera_Distance, 0.0f);
    XMVECTOR baseCameraPos = XMVector3Transform(offset, rotation) + baseTargetPos;

    // Get Camera Right Vector
    XMMATRIX yawRotation = XMMatrixRotationY(Camera_Yaw);
    XMVECTOR horizontalRightVec = XMVector3Transform(XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), yawRotation);

    // Get Final Camera POS
    XMVECTOR shiftVector = horizontalRightVec * Camera_Sights_Offset;

    XMVECTOR finalCameraPos = baseCameraPos + shiftVector;
    XMVECTOR finalTargetPos = baseTargetPos + shiftVector;

    // --- Matrix Update ---
    // Make View Matrix
    XMMATRIX mtxView = XMMatrixLookAtLH(finalCameraPos, finalTargetPos, { 0.0f,1.0f,0.0f });
    XMStoreFloat4x4(&Camera_View_mtx, mtxView);
    Shader_Manager::GetInstance()->SetViewMatrix3D(mtxView);
    XMStoreFloat3(&Current_Camera_Pos, finalCameraPos);

    // Get Camera Front Vector
    XMVECTOR front = XMVector3Transform(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), rotation);
    XMStoreFloat3(&Camera_Front, front);

    //--- Projection Matrix ---
	float Ratio = static_cast<float>(Direct3D_GetBackBufferWidth()) / Direct3D_GetBackBufferHeight();
	XMMATRIX mtxPerspective = XMMatrixPerspectiveFovLH(Camera_FOV, Ratio, Camera_Near_Z, Camera_Far_z);

    Shader_Manager::GetInstance()->SetProjectionMatrix3D(mtxPerspective);
}

void Player_Camera_Set_Menu_Mode(const XMFLOAT3& pos, const XMFLOAT3& target)
{
    Is_Menu_Mode = true;
    Menu_Cam_Pos = pos;
    Menu_Cam_Target = target;

    Camera_POS = Menu_Cam_Pos;
}

void Player_Camera_Set_Game_Mode()
{
    Is_Menu_Mode = false;
}

const XMFLOAT3& Player_Camera_Get_POS()
{
    return Camera_POS;
}

const XMFLOAT3& Player_Camera_Get_Front()
{
    return Camera_Front;
}

const XMFLOAT3& Player_Camera_Get_Current_POS()
{
    return Current_Camera_Pos;
}
float Player_Camera_Get_Yaw()
{
    return Camera_Yaw;
}
float Player_Camera_Get_Pitch()
{
    return Camera_Pitch;
}

float Player_Camera_Get_Far_Z()
{
    return Camera_Far_z;
}

Player_Sights Player_Camera_Get_Now_Sights()
{
    return Current_Sights;
}

void Player_Camera_Set_Sights(Player_Sights sight)
{
    Current_Sights = sight;

    float width = Is_Aiming_Mode ? Offset_Width_Aim : Offset_Width_Normal;

    switch (Current_Sights)
    {
    case Player_Sights::Left:
        Target_Offset = -width;
        break;
    case Player_Sights::Middle:
        Target_Offset = 0.0f;
        break;
    case Player_Sights::Right:
        Target_Offset = width;
        break;
    }
}

void Player_Camera_Set_Aiming_Mode(bool Is_Aiming)
{
    Is_Aiming_Mode = Is_Aiming;

    float dir = 0.0f;
    if (Current_Sights == Player_Sights::Right) 
    {
        dir = 1.0f;
    }
    else if (Current_Sights == Player_Sights::Left) 
    {
        dir = -1.0f;
    }

    if (Is_Aiming)
    {
        Target_Dist = Dist_Aim;
        Target_FOV = FOV_Aim;
        Target_Sens_Mult = Sens_Mult_Aim;
        Target_Height = Height_Aim;
        Target_Offset = dir * Offset_Width_Aim;
    }
    else
    {
        Target_Dist = Dist_Normal;
        Target_FOV = FOV_Normal;
        Target_Sens_Mult = 1.0f;
        Target_Height = Height_Normal;
        Target_Offset = dir * Offset_Width_Normal;
    }
}

DirectX::XMFLOAT4X4& Player_Camera_Get_View_Matrix()
{
    return Camera_View_mtx;
}

void Set_Mouse_Sensitivity(float Sensitivity)
{
    Mouse_Sensitivity = Sensitivity;
}

float Get_Mouse_Sensitivity()
{
    return Mouse_Sensitivity;
}
 