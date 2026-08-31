/*==============================================================================

	Manage Model Logic [Model.h]

	Author : Choi HyungJoon

==============================================================================*/
#ifndef MODEL_H
#define MODEL_H
#pragma comment (lib, "assimp-vc143-mt.lib")
#include <unordered_map>
#include <vector>                        
#include <string>                        
#include <d3d11.h>
#include <DirectXMath.h>
#include "cimport.h"
#include "scene.h"
#include "postprocess.h"
#include "matrix4x4.h"
#include "Collision.h"

// -----------------//
//   Model Default  //
// -----------------//
// Default Model Root Y Position
constexpr float MODEL_ROOT_Y_DEFAULT = -90.0f;

// Clavicle Default Rotation
constexpr float L_CLAV_X_DEF = -15.0f, L_CLAV_Y_DEF = 90.0f, L_CLAV_Z_DEF = 90.0f;
constexpr float R_CLAV_X_DEF = 15.0f, R_CLAV_Y_DEF = 270.0f, R_CLAV_Z_DEF = 270.0f;

// Left Arm Default Rotation
constexpr float L_ARM_UPPER_X_DEF = 0.0f, L_ARM_UPPER_Y_DEF = 0.0f, L_ARM_UPPER_Z_DEF = 0.0f; // 45?
constexpr float L_ARM_FORE_Z_DEF = 0.0f; // 15?

// Right Arm Default Rotation
constexpr float R_ARM_UPPER_X_DEF = 0.0f, R_ARM_UPPER_Y_DEF = 0.0f, R_ARM_UPPER_Z_DEF = 0.0f;
constexpr float R_ARM_FORE_Z_DEF = 0.0f;

// Hand Default Rotation
constexpr float HAND_DEF_X = 0.0f;
constexpr float HAND_DEF_Y = 0.0f;
constexpr float HAND_DEF_Z = 0.0f;

// Leg Default Rotation
constexpr float LEG_UPPER_Z_DEF = 180.0f;
constexpr float LEG_LOWER_Z_DEF = 0.0f;

// Foot Default Rotation
constexpr float FOOT_X_DEF = 45.0f;
constexpr float FOOT_Z_DEF = 0.0f;

// -----------------//
//       Root	    //
// -----------------//
// Model Root Position Adjust
extern float g_Model_Root_Y;

// -----------------//
//       Eyes       //
// -----------------//
extern float g_L_Eye_X, g_L_Eye_Y;
extern float g_R_Eye_X, g_R_Eye_Y;

// -----------------//
//     Clavicle     //
// -----------------//
extern float g_L_Clavicle_X, g_L_Clavicle_Y, g_L_Clavicle_Z;
extern float g_R_Clavicle_X, g_R_Clavicle_Y, g_R_Clavicle_Z;

// -----------------//
//       Arms       //
// -----------------//
// Left Arm
extern bool g_L_Arm_Invert_X, g_L_Arm_Invert_Y, g_L_Arm_Invert_Z;
extern float g_L_Arm_Upper_X, g_L_Arm_Upper_Y, g_L_Arm_Upper_Z;
extern float g_L_Arm_Fore_X, g_L_Arm_Fore_Y, g_L_Arm_Fore_Z;

// Right Arm
extern bool g_R_Arm_Invert_X, g_R_Arm_Invert_Y, g_R_Arm_Invert_Z;
extern float g_R_Arm_Upper_X, g_R_Arm_Upper_Y, g_R_Arm_Upper_Z;
extern float g_R_Arm_Fore_X, g_R_Arm_Fore_Y, g_R_Arm_Fore_Z;

// -----------------//
//   Hand (Wrist)   //
// -----------------//
// Left Hand
extern float g_L_Hand_X, g_L_Hand_Y, g_L_Hand_Z;
extern float g_L_Finger[5][3][3];

// Right Hand
extern float g_R_Hand_X, g_R_Hand_Y, g_R_Hand_Z;
extern float g_R_Finger[5][3][3];

// -----------------//
//       Legs       //
// -----------------//
// Left Leg
extern bool g_L_Leg_Invert_X, g_L_Leg_Invert_Y, g_L_Leg_Invert_Z;
extern float g_L_Leg_Upper_X, g_L_Leg_Upper_Y, g_L_Leg_Upper_Z;
extern float g_L_Leg_Lower_X, g_L_Leg_Lower_Y, g_L_Leg_Lower_Z;
extern float g_L_Foot_X, g_L_Foot_Y, g_L_Foot_Z;                 
extern float g_L_Toe_X, g_L_Toe_Y, g_L_Toe_Z;

// Right Leg
extern bool g_R_Leg_Invert_X, g_R_Leg_Invert_Y, g_R_Leg_Invert_Z;
extern float g_R_Leg_Upper_X, g_R_Leg_Upper_Y, g_R_Leg_Upper_Z;
extern float g_R_Leg_Lower_X, g_R_Leg_Lower_Y, g_R_Leg_Lower_Z;
extern float g_R_Foot_X, g_R_Foot_Y, g_R_Foot_Z;            
extern float g_R_Toe_X, g_R_Toe_Y, g_R_Toe_Z;

// ---------------------//
//  Animation Constants //
// ---------------------//
constexpr int MAX_BONES = 256;
extern bool Is_Bone_AABB_Draw;
constexpr float Bone_AABB_Defalut = 0.05f;
extern float Bone_AABB;

//3D頂点構造体
struct Vertex3D
{
    DirectX::XMFLOAT3 position = {}; // 頂点座標
    DirectX::XMFLOAT3 normal = {}; //法線
    DirectX::XMFLOAT4 color = {}; // 色
    DirectX::XMFLOAT2 texcoord = {}; // UV

    unsigned int BoneIndex[4] = { 0,0,0,0 };
    float BoneWeight[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
};

struct BoneInfo
{
    DirectX::XMMATRIX Offset;
    DirectX::XMMATRIX FinalTransform;
    DirectX::XMFLOAT4X4 ModelSpaceTransform;
};

struct MODEL
{
    const aiScene* AiScene = nullptr;

    ID3D11Buffer** VertexBuffer = nullptr;
    ID3D11Buffer** IndexBuffer = nullptr;

    std::unordered_map<std::string, ID3D11ShaderResourceView*> Texture;

    AABB Local_AABB = {};

    // ===== Animation File Data =====
    std::unordered_map<std::string, int> BoneMapping;
    std::vector<BoneInfo> BoneInfos;                 
    const aiNode* RootNode = nullptr;                
    DirectX::XMMATRIX GlobalInverse = {};
    float AnimationTime = 0.0f;               

    // ===== Animation Management Data =====
    std::vector<const aiScene*> AnimScenes;
    std::unordered_map<std::string, aiAnimation*> Animations;
    aiAnimation* CurrentAnim = nullptr;

	// ==== Animation Control Data =====
    bool CurrentAnim_Loop = true;
    bool IsAnimFinished = false;
};

MODEL* ModelLoad(const char* FileName, bool bBlender = false);
void ModelRelease(MODEL* model);
void ModelDraw(MODEL* model, const DirectX::XMMATRIX& mtxWorld);
void ModelDraw_Bone(MODEL* model, const DirectX::XMMATRIX& worldMtx);

// Logic For Get Collision
AABB Model_Get_AABB(MODEL* model, const DirectX::XMFLOAT3& position);

// Logic Gor Update Animation
void Model_Update_Animation(MODEL* model, float deltaTime);

// Login For Add Animation In Model
void Model_Add_Animation(MODEL* model, const std::string& animName, const char* fileName);

// Logic For Change Animation
void Model_Play_Animation(MODEL* model, const std::string& animName, bool loop = true);

// Logic For Get Bone Transforms For Shader
void Model_Get_BoneTransforms(MODEL* model, std::vector<DirectX::XMFLOAT4X4>& outTransforms);

// Logic For Reset Model Node Transform
void Model_Node_Reset();
#endif
