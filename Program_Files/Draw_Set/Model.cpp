/*==============================================================================

	Manage Model Logic [Model.cpp]

	Author : Choi HyungJoon

==============================================================================*/
#include <assert.h>
#include "direct3d.h"
#include "Texture_Manager.h"   
#include "Model.h"
#include <DirectXMath.h>
#include "WICTextureLoader11.h"
#include "Shader_Manager.h"    
#include <string>
#include "debug_ostream.h"
#include "Game_Model.h"
#include "Debug_Collision.h"
#include "Palette.h"
using namespace DirectX;

// ===== Root =====
float g_Model_Root_Y = MODEL_ROOT_Y_DEFAULT;

// ===== Eyes =====
float g_L_Eye_X = 0.0f, g_L_Eye_Y = 0.0f;
float g_R_Eye_X = 0.0f, g_R_Eye_Y = 0.0f;

// ===== Clavicle =====
float g_L_Clavicle_X = L_CLAV_X_DEF, g_L_Clavicle_Y = L_CLAV_Y_DEF, g_L_Clavicle_Z = L_CLAV_Z_DEF;
float g_R_Clavicle_X = R_CLAV_X_DEF, g_R_Clavicle_Y = R_CLAV_Y_DEF, g_R_Clavicle_Z = R_CLAV_Z_DEF;

// ===== Arms =====
// Left Arm
bool g_L_Arm_Invert_X = false, g_L_Arm_Invert_Y = false, g_L_Arm_Invert_Z = false;
float g_L_Arm_Upper_X = L_ARM_UPPER_X_DEF, g_L_Arm_Upper_Y = L_ARM_UPPER_Y_DEF, g_L_Arm_Upper_Z = L_ARM_UPPER_Z_DEF;
float g_L_Arm_Fore_X = 0.0f, g_L_Arm_Fore_Y = 0.0f, g_L_Arm_Fore_Z = L_ARM_FORE_Z_DEF;

// Right Arm
bool g_R_Arm_Invert_X = false, g_R_Arm_Invert_Y = false, g_R_Arm_Invert_Z = false;
float g_R_Arm_Upper_X = R_ARM_UPPER_X_DEF, g_R_Arm_Upper_Y = R_ARM_UPPER_Y_DEF, g_R_Arm_Upper_Z = R_ARM_UPPER_Z_DEF;
float g_R_Arm_Fore_X = 0.0f, g_R_Arm_Fore_Y = 0.0f, g_R_Arm_Fore_Z = R_ARM_FORE_Z_DEF;

// ===== Hand =====
// Left Hand
float g_L_Hand_X = HAND_DEF_X, g_L_Hand_Y = HAND_DEF_Y, g_L_Hand_Z = HAND_DEF_Z;
float g_L_Finger[5][3][3] = { 0 };

// Right Hand
float g_R_Hand_X = HAND_DEF_X, g_R_Hand_Y = HAND_DEF_Y, g_R_Hand_Z = HAND_DEF_Z;
float g_R_Finger[5][3][3] = { 0 };

// ===== Legs =====
// Left Leg
bool g_L_Leg_Invert_X = false, g_L_Leg_Invert_Y = false, g_L_Leg_Invert_Z = false;
float g_L_Leg_Upper_X = 0.0f, g_L_Leg_Upper_Y = 0.0f, g_L_Leg_Upper_Z = LEG_UPPER_Z_DEF; // 180
float g_L_Leg_Lower_X = 0.0f, g_L_Leg_Lower_Y = 0.0f, g_L_Leg_Lower_Z = LEG_LOWER_Z_DEF; // 0
float g_L_Foot_X = FOOT_X_DEF, g_L_Foot_Y = 0.0f, g_L_Foot_Z = FOOT_Z_DEF; // X 45, Z 0
float g_L_Toe_X = FOOT_X_DEF, g_L_Toe_Y = 0.0f, g_L_Toe_Z = FOOT_Z_DEF;

// Right Leg
bool g_R_Leg_Invert_X = false, g_R_Leg_Invert_Y = false, g_R_Leg_Invert_Z = false;
float g_R_Leg_Upper_X = 0.0f, g_R_Leg_Upper_Y = 0.0f, g_R_Leg_Upper_Z = LEG_UPPER_Z_DEF; // 180
float g_R_Leg_Lower_X = 0.0f, g_R_Leg_Lower_Y = 0.0f, g_R_Leg_Lower_Z = LEG_LOWER_Z_DEF; // 0
float g_R_Foot_X = FOOT_X_DEF, g_R_Foot_Y = 0.0f, g_R_Foot_Z = FOOT_Z_DEF; // X 45, Z 0
float g_R_Toe_X = FOOT_X_DEF, g_R_Toe_Y = 0.0f, g_R_Toe_Z = FOOT_Z_DEF;

// Debug Bone Draw
bool Is_Bone_AABB_Draw = true;;
float Bone_AABB = Bone_AABB_Defalut;

static int g_TextureWhite = -1; // "TextSample" ID

//-------------------Healper Logic--------------------//
static void AddBoneData(Vertex3D& v, int boneIndex, float weight);
static const aiNodeAnim* FindNodeAnim(const aiAnimation* anim, const std::string& name);
static void ReadNodeHierarchy(MODEL* model, float animTime, const aiNode* node, const DirectX::XMMATRIX& parent);
static bool Parse_Finger_Bone(const std::string& NodeName, bool isLeft, int& outFingerIdx, int& outJointIdx);

MODEL* ModelLoad(const char* FileName, bool bBlender)
{
	if (g_TextureWhite == -1)
	{
		g_TextureWhite = Texture_Manager::GetInstance()->GetID("TextSample");
	}

	// Make ModelPointer
	MODEL* model = new MODEL;

	// Make Model Scene
	model->AiScene = aiImportFile(FileName,
		aiProcessPreset_TargetRealtime_MaxQuality |
		aiProcess_ConvertToLeftHanded |
		aiProcess_LimitBoneWeights);

	if (model->AiScene == nullptr)
	{
		Debug::D_Out << "Error: aiImportFile failed to load file: " << FileName << std::endl;
		delete model;
		return nullptr;
	}

	model->Local_AABB.Min = { FLT_MAX, FLT_MAX, FLT_MAX };
	model->Local_AABB.Max = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

	model->VertexBuffer = new ID3D11Buffer * [model->AiScene->mNumMeshes]();
	model->IndexBuffer = new ID3D11Buffer * [model->AiScene->mNumMeshes]();

	// ===================================================================
	// Loop for Index Mesh
	// ===================================================================
	for (unsigned int m = 0; m < model->AiScene->mNumMeshes; m++)
	{
		aiMesh* mesh = model->AiScene->mMeshes[m];

		aiMaterial* aimaterial = model->AiScene->mMaterials[mesh->mMaterialIndex];

		aiString textureName;
		aimaterial->GetTexture(aiTextureType_DIFFUSE, 0, &textureName);

		// Get Diffuse Color
		XMFLOAT4 meshColor = { 1.0f, 1.0f, 1.0f, 1.0f };
		aiColor3D diffuse(1.0f, 1.0f, 1.0f);

		if (aimaterial->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse) == AI_SUCCESS)
		{
			// [Safety Fix] If Diffuse is Black(0,0,0), Force White
			if (diffuse.r <= 0.01f && diffuse.g <= 0.01f && diffuse.b <= 0.01f)
			{
				meshColor = { 1.0f, 1.0f, 1.0f, 1.0f };
			}
			else
			{
				meshColor = { diffuse.r, diffuse.g, diffuse.b, 1.0f };
			}
		}

		// 頂点バッファ生成
		Vertex3D* vertex = new Vertex3D[mesh->mNumVertices];

		for (unsigned int v = 0; v < mesh->mNumVertices; v++)
		{
			// POS
			if (bBlender) {
				vertex[v].position = {
					mesh->mVertices[v].x,
					mesh->mVertices[v].z,
					mesh->mVertices[v].y,
				};
			}
			else {
				vertex[v].position = {
					mesh->mVertices[v].x,
					mesh->mVertices[v].y,
					mesh->mVertices[v].z,
				};
			}

			// 法線
			if (mesh->HasNormals()) {
				if (bBlender) {
					vertex[v].normal = {
						mesh->mNormals[v].x,
						mesh->mNormals[v].z, // Y and Z
						mesh->mNormals[v].y, // 
					};
				}
				else {
					vertex[v].normal = {
						mesh->mNormals[v].x,
						mesh->mNormals[v].y,
						mesh->mNormals[v].z,
					};
				}
			}
			else {
				vertex[v].normal = { 0.0f,1.0f,0.0f };
			}


			// 色
			if (mesh->HasVertexColors(0))
			{
				vertex[v].color = {
					mesh->mColors[0][v].r,
					mesh->mColors[0][v].g,
					mesh->mColors[0][v].b,
					mesh->mColors[0][v].a,
				};
			}
			else
			{
				vertex[v].color = meshColor;
			}

			// UV
			if (mesh->HasTextureCoords(0)) {
				vertex[v].texcoord = {
					mesh->mTextureCoords[0][v].x,
					mesh->mTextureCoords[0][v].y
				};
			}
			else {
				vertex[v].texcoord = { 0.0f,0.0f };
			}

			// Make AABB
			model->Local_AABB.Min.x = std::min(model->Local_AABB.Min.x, vertex[v].position.x);
			model->Local_AABB.Min.y = std::min(model->Local_AABB.Min.y, vertex[v].position.y);
			model->Local_AABB.Min.z = std::min(model->Local_AABB.Min.z, vertex[v].position.z);

			model->Local_AABB.Max.x = std::max(model->Local_AABB.Max.x, vertex[v].position.x);
			model->Local_AABB.Max.y = std::max(model->Local_AABB.Max.y, vertex[v].position.y);
			model->Local_AABB.Max.z = std::max(model->Local_AABB.Max.z, vertex[v].position.z);
		}

		// ===== Bone Load =====
		if (mesh->HasBones())
		{
			for (unsigned int b = 0; b < mesh->mNumBones; b++)
			{
				aiBone* bone = mesh->mBones[b];
				std::string boneName = bone->mName.C_Str();

				int boneIndex = 0;

				if (model->BoneMapping.find(boneName) == model->BoneMapping.end())
				{
					boneIndex = (int)model->BoneInfos.size();

					model->BoneMapping[boneName] = boneIndex;

					// Save Offset Matrix
					BoneInfo bi{};
					bi.Offset = XMMatrixTranspose(XMMATRIX(&bone->mOffsetMatrix.a1));
					model->BoneInfos.push_back(bi);
				}
				else
				{
					boneIndex = model->BoneMapping[boneName];
				}

				for (unsigned int w = 0; w < bone->mNumWeights; w++)
				{
					unsigned int vertexID = bone->mWeights[w].mVertexId;
					float weight = bone->mWeights[w].mWeight;

					AddBoneData(vertex[vertexID], boneIndex, weight);
				}
			}

			// ===== BoneWeight Normalize =====
			for (unsigned int v = 0; v < mesh->mNumVertices; v++)
			{
				float sum = 0.0f;
				for (int i = 0; i < 4; i++)
					sum += vertex[v].BoneWeight[i];

				if (sum > 0.0f)
				{
					for (int i = 0; i < 4; i++)
						vertex[v].BoneWeight[i] /= sum;
				}
			}
		}

		// 頂点バッファ生成
		D3D11_BUFFER_DESC buffer_desc{};
		buffer_desc.ByteWidth = sizeof(Vertex3D) * mesh->mNumVertices;
		buffer_desc.Usage = D3D11_USAGE_DEFAULT;
		buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA subresource_data{};
		subresource_data.pSysMem = vertex;

		Direct3D_GetDevice()->CreateBuffer(&buffer_desc, &subresource_data, &model->VertexBuffer[m]);

		delete[] vertex;

		// インデックスバッファ生成
		unsigned int* index = new unsigned int[mesh->mNumFaces * 3];

		for (unsigned int f = 0; f < mesh->mNumFaces; f++)
		{
			index[f * 3 + 0] = mesh->mFaces[f].mIndices[0];
			index[f * 3 + 1] = mesh->mFaces[f].mIndices[1];
			index[f * 3 + 2] = mesh->mFaces[f].mIndices[2];
		}

		buffer_desc.ByteWidth = sizeof(unsigned int) * mesh->mNumFaces * 3;
		buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		subresource_data.pSysMem = index;
		Direct3D_GetDevice()->CreateBuffer(&buffer_desc, &subresource_data, &model->IndexBuffer[m]);

		delete[] index;
	}
	// ===================================================================
	// loop done
	// ===================================================================


	// ===================================================================
	// 2. Texture Load
	// ===================================================================
	for (unsigned int i = 0; i < model->AiScene->mNumTextures; i++)
	{
		aiTexture* AI_Tex = model->AiScene->mTextures[i];

		ID3D11ShaderResourceView* texture;
		ID3D11Resource* resource;

		CreateWICTextureFromMemory(
			Direct3D_GetDevice(),
			Direct3D_GetContext(),
			(const uint8_t*)AI_Tex->pcData,
			((size_t)AI_Tex->mWidth == 0) ? (size_t)AI_Tex->mHeight : (size_t)AI_Tex->mWidth,
			&resource,
			&texture);

		assert(texture);

		resource->Release();
		model->Texture[AI_Tex->mFilename.data] = texture;
	}

	const std::string modelPath(FileName);
	size_t pos = modelPath.find_last_of("/\\");
	std::string directory = (pos != std::string::npos) ? modelPath.substr(0, pos + 1) : "";

	for (unsigned int i = 0; i < model->AiScene->mNumMaterials; i++)
	{
		aiString texPath;
		aiMaterial* aimaterial = model->AiScene->mMaterials[i];

		if (aimaterial->GetTexture(aiTextureType_DIFFUSE, 0, &texPath) == AI_SUCCESS)
		{
			if (model->Texture.find(texPath.C_Str()) == model->Texture.end())
			{
				ID3D11ShaderResourceView* texture = nullptr;
				ID3D11Resource* resource = nullptr;

				std::string sTexPath = texPath.C_Str();
				size_t namePos = sTexPath.find_last_of("/\\");
				if (namePos != std::string::npos)
				{
					sTexPath = sTexPath.substr(namePos + 1);
				}

				// Combine Directory + Filename
				std::string texfilename = directory + sTexPath;
				std::replace(texfilename.begin(), texfilename.end(), '\\', '/');
				Debug::D_Out << "Loading Texture: " << texfilename << std::endl;

				// WideChar Conversion
				int len = MultiByteToWideChar(CP_UTF8, 0, texfilename.c_str(), -1, nullptr, 0);
				wchar_t* pWideFilename = new wchar_t[len];
				MultiByteToWideChar(CP_UTF8, 0, texfilename.c_str(), -1, pWideFilename, len);

				HRESULT hr = CreateWICTextureFromFile(
					Direct3D_GetDevice(),
					pWideFilename,
					&resource,
					&texture);

				delete[] pWideFilename;

				if (SUCCEEDED(hr))
				{
					if (resource) resource->Release();
					model->Texture[texPath.C_Str()] = texture;
				}
				else
				{
					Debug::D_Out << "Failed to load external texture: " << texfilename.c_str() << std::endl;
					// Keep nullptr, fallback handled in Draw
					model->Texture[texPath.C_Str()] = nullptr;
				}
			}
		}
	}

	model->RootNode = model->AiScene->mRootNode;
	
	/*
	// IF Need Scaleling In Load Model, Use This Code
	// Model Scale Set For Animation RootNode Transform (Defance Code)
	model->AiScene->mRootNode->mTransformation.a4 *= scale;
	model->AiScene->mRootNode->mTransformation.b4 *= scale;
	model->AiScene->mRootNode->mTransformation.c4 *= scale;
	*/

	model->GlobalInverse = XMMatrixInverse(nullptr, XMMatrixTranspose(XMMATRIX(&model->AiScene->mRootNode->mTransformation.a1)));

	return model;
}

void ModelRelease(MODEL* model)
{
	if (!model) return;

	if (model->AiScene)
	{
		for (unsigned int m = 0; m < model->AiScene->mNumMeshes; m++)
		{
			SAFE_RELEASE(model->VertexBuffer[m]);
			SAFE_RELEASE(model->IndexBuffer[m]);
		}
	}

	delete[] model->VertexBuffer;
	delete[] model->IndexBuffer;

	if (model->AiScene)
	{
		aiReleaseImport(model->AiScene);
		model->AiScene = nullptr;
	}

	for (auto scene : model->AnimScenes)
	{
		aiReleaseImport(scene);
	}
	// Reset Animation Info
	model->AnimScenes.clear();

	delete model;
}

void ModelDraw(MODEL* model, const DirectX::XMMATRIX& mtxWorld)
{
	if (!model) return;

	float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	Direct3D_GetContext()->OMSetBlendState(nullptr, blendFactor, 0xffffffff);

	// -----------------------------------------------------------
	// 1. Check Bone, Set Shader
	// -----------------------------------------------------------

	bool hasBones = !model->BoneInfos.empty();

	if (hasBones)
	{
		// Animation Model -> Set Skinning Shader
		Shader_Manager::GetInstance()->Begin3D_Skinning();

		// Trans BoneInfo XMMATRIX For XMFLOAT4X4
		std::vector<XMFLOAT4X4> boneTransforms;
		boneTransforms.reserve(model->BoneInfos.size());

		for (const auto& boneInfo : model->BoneInfos)
		{
			XMFLOAT4X4 m;
			// FinalTransform Aleady Set In ModelUpdateAnimation
			XMStoreFloat4x4(&m, boneInfo.FinalTransform);
			boneTransforms.push_back(m);
		}

		// Send Current Frame Bone Dara To Shader Manager
		Shader_Manager::GetInstance()->SetBones(boneTransforms.data(), (int)boneTransforms.size());
	}
	else
	{
		// Normal Model -> Set 3D Shader
		Shader_Manager::GetInstance()->Begin3D();
	}

	// World Matrix Set Is Same Work
	Shader_Manager::GetInstance()->SetWorldMatrix3D(mtxWorld);

	// -----------------------------------------------------------
	// 2. Draw Mash Loop
	// -----------------------------------------------------------
	// プリミティブトポロジ設定
	Direct3D_GetContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	for (unsigned int m = 0; m < model->AiScene->mNumMeshes; m++)
	{
		aiMaterial* aimaterial = model->AiScene->mMaterials[model->AiScene->mMeshes[m]->mMaterialIndex];

		// Get Diffuse Color And Opacity
		aiColor4D diffuseColor(1.0f, 1.0f, 1.0f, 1.0f);
		if (aimaterial->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColor) == AI_SUCCESS)
		{
			if (diffuseColor.r < 0.01f && diffuseColor.g < 0.01f && diffuseColor.b < 0.01f)
			{
				diffuseColor = { 1.0f, 1.0f, 1.0f, 1.0f };
			}
		}

		float opacity = 1.0f;
		aimaterial->Get(AI_MATKEY_OPACITY, opacity);
		diffuseColor.a *= opacity;

		// Get Texture
		aiString texturePath;
		aimaterial->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath);

		// Set Texture
		ID3D11ShaderResourceView* pTexture = nullptr;
		bool textureFound = false;

		if (texturePath.length != 0)
		{
			auto it = model->Texture.find(texturePath.C_Str());
			if (it != model->Texture.end() && it->second != nullptr) // If Find Texture And Load Success
			{
				pTexture = it->second;
				textureFound = true;
			}
		}

		if (!textureFound) // If Cant`t Find Texture And Load Failed
		{
			pTexture = Texture_Manager::GetInstance()->Get_Shader_Resource_View(g_TextureWhite);

			// If No Texture DiffuseColor = White
			diffuseColor.r = 1.0f;
			diffuseColor.g = 1.0f;
			diffuseColor.b = 1.0f;
			diffuseColor.a = 1.0f;
		}

		// Input Diffuse Color And Opacity To Shader
		Shader_Manager::GetInstance()->SetDiffuseColor(XMFLOAT4(diffuseColor.r, diffuseColor.g, diffuseColor.b, diffuseColor.a));
		Shader_Manager::GetInstance()->SetTexture3D(pTexture);

		// 頂点バッファを描画パイプラインに設定
		UINT stride = sizeof(Vertex3D);
		UINT offset = 0;
		Direct3D_GetContext()->IASetVertexBuffers(0, 1, &model->VertexBuffer[m], &stride, &offset);

		// インデックスバッファを描画パイプラインに設定
		Direct3D_GetContext()->IASetIndexBuffer(model->IndexBuffer[m], DXGI_FORMAT_R32_UINT, 0);

		// 描画
		Direct3D_GetContext()->DrawIndexed(model->AiScene->mMeshes[m]->mNumFaces * 3, 0, 0);
	}
}

void ModelDraw_Bone(MODEL* model, const DirectX::XMMATRIX& worldMtx)
{
	if (!model || !Is_Bone_AABB_Draw) return;

	for (const auto& bone : model->BoneInfos)
	{
		// 1. Get Bone Matrix Info
		XMMATRIX boneMtx = XMLoadFloat4x4(&bone.ModelSpaceTransform);

		// 2. Get Real World Matrix In Model World Matrix
		XMMATRIX finalMtx = boneMtx * worldMtx;

		// 3. Get Translation In Matrix
		XMFLOAT3 bonePos;
		XMStoreFloat3(&bonePos, finalMtx.r[3]);

		// 4. Set Box Scale
		float boxSize = Bone_AABB;
		AABB boneBox = {};
		boneBox.Min = { bonePos.x - boxSize, bonePos.y - boxSize, bonePos.z - boxSize };
		boneBox.Max = { bonePos.x + boxSize, bonePos.y + boxSize, bonePos.z + boxSize };

		// 5. Draw
		Debug_Collision_Draw(boneBox, PALETTE::Red);
	}
}

AABB Model_Get_AABB(MODEL* model, const DirectX::XMFLOAT3& position)
{
	return
	{
		{position.x + model->Local_AABB.Min.x, position.y + model->Local_AABB.Min.y, position.z + model->Local_AABB.Min.z},
		{position.x + model->Local_AABB.Max.x, position.y + model->Local_AABB.Max.y, position.z + model->Local_AABB.Max.z}
	};
}

void Model_Update_Animation(MODEL* model, float deltaTime)
{
	if (!model || !model->CurrentAnim) return;

	// If Animation Finished And Not Loop, Do Not Update
	if (model->IsAnimFinished && !model->CurrentAnim_Loop) return;

	float TicksPerSecond = static_cast<float>(model->CurrentAnim->mTicksPerSecond !=
		0 ? model->CurrentAnim->mTicksPerSecond : 25.0f);
	float TimeInTicks = deltaTime * TicksPerSecond;

	model->AnimationTime += TimeInTicks;

	float Duration = static_cast<float>(model->CurrentAnim->mDuration);

	if (model->CurrentAnim_Loop)
	{
		model->AnimationTime = fmod(model->AnimationTime, Duration);
	}
	else
	{
		if (model->AnimationTime >= Duration)
		{
			model->AnimationTime = Duration - 0.001f;
			model->IsAnimFinished = true;
		}
	}

	ReadNodeHierarchy(model, model->AnimationTime, model->RootNode, XMMatrixIdentity());
}

void Model_Add_Animation(MODEL* model, const std::string& animName, const char* fileName)
{
	if (!model) return;

	// Load Animation File, Denind Mash Info, Only Animation Data
	const aiScene* animScene = aiImportFile(fileName, aiProcess_LimitBoneWeights | aiProcess_ConvertToLeftHanded);

	// Check Animation Load Success
	if (!animScene || !animScene->HasAnimations())
	{
		Debug::D_Out << "Failed to load animation: " << fileName << std::endl;
		return;
	}

	// Save Pointer For Memory
	model->AnimScenes.push_back(animScene);

	// Set Animation Track In Map
	model->Animations[animName] = animScene->mAnimations[0];
	
	// IF Map Is Clear, This Animation Will Be Default
	if (model->CurrentAnim == nullptr)
	{
		model->CurrentAnim = animScene->mAnimations[0];
	}
}

void Model_Play_Animation(MODEL* model, const std::string& animName, bool loop)
{
	if (!model) return;

	auto it = model->Animations.find(animName);
	if (it != model->Animations.end())
	{
		// It Already Played, Reset Or Do Notihing (In Now, Just Change)
		if (model->CurrentAnim == it->second)
		{
			model->CurrentAnim_Loop = loop; // Only Update Loop Flag
			return;
		}

		model->CurrentAnim = it->second;
		model->AnimationTime = 0.0f;     // Is New Animation, Reset Time
		model->CurrentAnim_Loop = loop;  // Set Loop
		model->IsAnimFinished = false;   // Reset Finish Flag

		Debug::D_Out << "Play Animation : " << animName << std::endl;
	}
}

void Model_Get_BoneTransforms(MODEL* model, std::vector<DirectX::XMFLOAT4X4>& outTransforms)
{
	if (!model) return;

	outTransforms.clear();
	outTransforms.resize(model->BoneInfos.size());

	for (size_t i = 0; i < model->BoneInfos.size(); ++i)
	{
		XMMATRIX finalMtx = model->BoneInfos[i].FinalTransform;
		XMStoreFloat4x4(&outTransforms[i], finalMtx);
	}
}

//-------------------Healper Logic--------------------//
static void AddBoneData(Vertex3D& v, int boneIndex, float weight)
{
	for (int i = 0; i < 4; i++)
	{
		if (v.BoneWeight[i] == 0.0f)
		{
			v.BoneIndex[i] = boneIndex;
			v.BoneWeight[i] = weight;
			return;
		}
	}

}

static const aiNodeAnim* FindNodeAnim(const aiAnimation* anim, const std::string& name)
{
	for (unsigned int i = 0; i < anim->mNumChannels; i++)
	{
		if (anim->mChannels[i]->mNodeName.C_Str() == name)
			return anim->mChannels[i];
	}
	return nullptr;
}

static void ReadNodeHierarchy(
	MODEL* model,
	float animTime,
	const aiNode* node,
	const XMMATRIX& parent)
{
	std::string name = node->mName.C_Str();

	// Make Transform Matrix (If No Animation, Use Node Transform)
	// Original Transform (T-Pose)
	XMMATRIX nodeTransform = XMMatrixTranspose(XMMATRIX(&node->mTransformation.a1));

	const aiAnimation* anim = model->CurrentAnim;
	const aiNodeAnim* channel = FindNodeAnim(anim, name);

	if (channel)
	{
		unsigned int posIndex = static_cast<unsigned int>(animTime) % channel->mNumPositionKeys;
		unsigned int rotIndex = static_cast<unsigned int>(animTime) % channel->mNumRotationKeys;
		unsigned int scaleIndex = channel->mNumScalingKeys > 0 ? static_cast<unsigned int>(animTime) % channel->mNumScalingKeys : 0;

		aiVector3D POS = channel->mPositionKeys[posIndex].mValue;
		aiQuaternion Rotation = channel->mRotationKeys[rotIndex].mValue;
		aiVector3D Scale(1, 1, 1);

		if (channel->mNumScalingKeys > 0)
			Scale = channel->mScalingKeys[scaleIndex].mValue;

// ---------------------------------------------------------------------------------------------------------------------------
// Function : Model Position, Rotation Convert To DirectX Math
// ---------------------------------------------------------------------------------------------------------------------------
		// -------------------------------------------------------------------------------------------------------------------
		// Step 1 : Detail Bone Detection
		// -------------------------------------------------------------------------------------------------------------------
		// [B]asic Types]
		bool isHips = (name.find("Hips") != std::string::npos) || (name.find("Root") != std::string::npos);

		// Eyes
		bool isLEyes = (name.find("LeftEye") != std::string::npos);
		bool isREyes = (name.find("RightEye") != std::string::npos);

		// [Shoulder]
		bool isClavicle = (name.find("Shoulder") != std::string::npos);

		// [Sides]
		bool isLeft = (name.find("Left") != std::string::npos) || (name.find("_L") != std::string::npos);
		bool isRight = (name.find("Right") != std::string::npos) || (name.find("_R") != std::string::npos);

		// [Arm]
		bool isForeArm = (name.find("ForeArm") != std::string::npos);
		bool isUpperArm = !isForeArm && !isClavicle && (name.find("Arm") != std::string::npos);

		// [Hand]
		bool isFinger = (name.find("Thumb") != std::string::npos) || (name.find("Index") != std::string::npos) ||
			(name.find("Middle") != std::string::npos) || (name.find("Ring") != std::string::npos) ||
			(name.find("Pinky") != std::string::npos);
		bool isHand = !isFinger && (name.find("Hand") != std::string::npos);

		// [Leg]
		bool isUpperLeg = (name.find("UpLeg") != std::string::npos);
		bool isLowerLeg = !isUpperLeg && (name.find("Leg") != std::string::npos);
		bool isFoot = (name.find("Foot") != std::string::npos);
		bool isToe = (name.find("Toe") != std::string::npos);

		// -------------------------------------------------------------------------------------------------------------------
		// Step 2 : Attach To Bone
		// If Not Hips, Ignore Position, Use Model Root Position
		// -------------------------------------------------------------------------------------------------------------------

		if (isHips)
		{
			POS.x = 0.0f;
			POS.z = 0.0f;
			POS.y += g_Model_Root_Y;
		}
		else
		{
			// If Hips Is Not, Use Original Position For Bone Attachment
			POS.x = node->mTransformation.a4;
			POS.y = node->mTransformation.b4;
			POS.z = node->mTransformation.c4;

			// Code For Ground Level Fix (Y Position)
			// Need To Jump Or Move UpDown, Use Hips Y Position
			// POS.y = node->mTransformation.b4; // Keep Y Position For Ground Level
		}

		// -------------------------------------------------------------------------------------------------------------------
		// Step 3 : Model Rotation
		// If Model Root or Hips Was Upward, Apply Correction Rotation
		// -------------------------------------------------------------------------------------------------------------------
	
		XMVECTOR Q_Rot = XMVectorSet(Rotation.x, Rotation.y, Rotation.z, Rotation.w);

		// Detail Adjustment
		float dbgX = 0.0f, dbgY = 0.0f, dbgZ = 0.0f;
		bool bInvX = false, bInvY = false, bInvZ = false;
		bool applyFix = false;

		// --- Eyes ---

		if (isLEyes)
		{
			dbgX = g_L_Eye_X;
			dbgY = g_L_Eye_Y;
			applyFix = true;
		}
		else if (isREyes)
		{
			dbgX = g_R_Eye_X;
			dbgY = g_R_Eye_Y;
			applyFix = true;
		}

		// --- Clavicle ---
		else if (isLeft && isClavicle)
		{
			dbgX = g_L_Clavicle_X; dbgY = g_L_Clavicle_Y; dbgZ = g_L_Clavicle_Z; applyFix = true;
		}
		else if (isRight && isClavicle)
		{
			dbgX = g_R_Clavicle_X; dbgY = g_R_Clavicle_Y; dbgZ = g_R_Clavicle_Z; applyFix = true;
		}

		// --- ARMS ---
		else if (isLeft && isUpperArm)
		{
			dbgX = g_L_Arm_Upper_X; dbgY = g_L_Arm_Upper_Y; dbgZ = g_L_Arm_Upper_Z;
			bInvX = g_L_Arm_Invert_X; bInvY = g_L_Arm_Invert_Y; bInvZ = g_L_Arm_Invert_Z; applyFix = true;
		}
		else if (isRight && isUpperArm) 
		{
			dbgX = g_R_Arm_Upper_X; dbgY = g_R_Arm_Upper_Y; dbgZ = g_R_Arm_Upper_Z;
			bInvX = g_R_Arm_Invert_X; bInvY = g_R_Arm_Invert_Y; bInvZ = g_R_Arm_Invert_Z; applyFix = true;
		}
		else if (isLeft && isForeArm)
		{
			dbgX = g_L_Arm_Fore_X; dbgY = g_L_Arm_Fore_Y; dbgZ = g_L_Arm_Fore_Z;
			bInvX = g_L_Arm_Invert_X; bInvY = g_L_Arm_Invert_Y; bInvZ = g_L_Arm_Invert_Z; applyFix = true;
		}
		else if (isRight && isForeArm) 
		{
			dbgX = g_R_Arm_Fore_X; dbgY = g_R_Arm_Fore_Y; dbgZ = g_R_Arm_Fore_Z;
			bInvX = g_R_Arm_Invert_X; bInvY = g_R_Arm_Invert_Y; bInvZ = g_R_Arm_Invert_Z; applyFix = true;
		}

		// --- HANDS & FINGERS ---
		else if (isFinger)
		{
			int fIdx = -1;
			int jIdx = -1;

			if (Parse_Finger_Bone(name, true, fIdx, jIdx)) // Left Hand Finger
			{
				dbgX = g_L_Finger[fIdx][jIdx][0];
				dbgY = g_L_Finger[fIdx][jIdx][1];
				dbgZ = g_L_Finger[fIdx][jIdx][2];
				applyFix = true;
			}
			else if (Parse_Finger_Bone(name, false, fIdx, jIdx)) // Right Hand Finger
			{
				dbgX = g_R_Finger[fIdx][jIdx][0];
				dbgY = g_R_Finger[fIdx][jIdx][1];
				dbgZ = g_R_Finger[fIdx][jIdx][2];
				applyFix = true;
			}
		}
		else if (isLeft && isHand)
		{
			dbgX = g_L_Hand_X; dbgY = g_L_Hand_Y; dbgZ = g_L_Hand_Z;
			bInvX = g_L_Arm_Invert_X; bInvY = g_L_Arm_Invert_Y; bInvZ = g_L_Arm_Invert_Z; applyFix = true;
		}
		else if (isRight && isHand) 
		{
			dbgX = g_R_Hand_X; dbgY = g_R_Hand_Y; dbgZ = g_R_Hand_Z;
			bInvX = g_R_Arm_Invert_X; bInvY = g_R_Arm_Invert_Y; bInvZ = g_R_Arm_Invert_Z; applyFix = true;
		}

		// --- LEGS ---
		else if (isLeft && isUpperLeg) 
		{
			dbgX = g_L_Leg_Upper_X; dbgY = g_L_Leg_Upper_Y; dbgZ = g_L_Leg_Upper_Z;
			bInvX = g_L_Leg_Invert_X; bInvY = g_L_Leg_Invert_Y; bInvZ = g_L_Leg_Invert_Z; applyFix = true;
		}
		else if (isRight && isUpperLeg) 
		{
			dbgX = g_R_Leg_Upper_X; dbgY = g_R_Leg_Upper_Y; dbgZ = g_R_Leg_Upper_Z;
			bInvX = g_R_Leg_Invert_X; bInvY = g_R_Leg_Invert_Y; bInvZ = g_R_Leg_Invert_Z; applyFix = true;
		}
		else if (isLeft && isLowerLeg) 
		{
			dbgX = g_L_Leg_Lower_X; dbgY = g_L_Leg_Lower_Y; dbgZ = g_L_Leg_Lower_Z;
			bInvX = g_L_Leg_Invert_X; bInvY = g_L_Leg_Invert_Y; bInvZ = g_L_Leg_Invert_Z; applyFix = true;
		}
		else if (isRight && isLowerLeg)
		{
			dbgX = g_R_Leg_Lower_X; dbgY = g_R_Leg_Lower_Y; dbgZ = g_R_Leg_Lower_Z;
			bInvX = g_R_Leg_Invert_X; bInvY = g_R_Leg_Invert_Y; bInvZ = g_R_Leg_Invert_Z; applyFix = true;
		}

		// --- FEET & TOES ---
		else if (isLeft && (isFoot || isToe))
		{
			dbgX = isFoot ? g_L_Foot_X : g_L_Toe_X;
			dbgY = isFoot ? g_L_Foot_Y : g_L_Toe_Y;
			dbgZ = isFoot ? g_L_Foot_Z : g_L_Toe_Z;
			bInvX = g_L_Leg_Invert_X; bInvY = g_L_Leg_Invert_Y; bInvZ = g_L_Leg_Invert_Z; applyFix = true;
		}
		else if (isRight && (isFoot || isToe))
		{
			dbgX = isFoot ? g_R_Foot_X : g_R_Toe_X;
			dbgY = isFoot ? g_R_Foot_Y : g_R_Toe_Y;
			dbgZ = isFoot ? g_R_Foot_Z : g_R_Toe_Z;
			bInvX = g_R_Leg_Invert_X; bInvY = g_R_Leg_Invert_Y; bInvZ = g_R_Leg_Invert_Z; applyFix = true;
		}

		// Apply Debug Rotation
		if (applyFix)
		{
			// Invert Axis Checkbox
			float finalPitch = XMConvertToRadians(dbgX * (bInvX ? -1.0f : 1.0f));
			float finalYaw = XMConvertToRadians(dbgY * (bInvY ? -1.0f : 1.0f));
			float finalRoll = XMConvertToRadians(dbgZ * (bInvZ ? -1.0f : 1.0f));

			XMVECTOR Q_Debug = XMQuaternionRotationRollPitchYaw(finalPitch, finalYaw, finalRoll);
			Q_Rot = XMQuaternionMultiply(Q_Rot, Q_Debug);
		}

		XMMATRIX Rot = XMMatrixRotationQuaternion(Q_Rot);

		// -------------------------------------------------------------------------------------------------------------------
		// Step 4 : Final Transform Combine
		// -------------------------------------------------------------------------------------------------------------------

		nodeTransform = XMMatrixScaling(Scale.x, Scale.y, Scale.z) * Rot * XMMatrixTranslation(POS.x, POS.y, POS.z);
	}

	// Combine Parent Transform
	XMMATRIX globalTransform = nodeTransform * parent;

	// Update Bone Final Transform
	if (model->BoneMapping.find(name) != model->BoneMapping.end())
	{
		unsigned int boneIndex = model->BoneMapping[name];

		// 1. Save Bone Matrix
		XMStoreFloat4x4(&model->BoneInfos[boneIndex].ModelSpaceTransform, globalTransform);

		// 2. Offset Matrix
		XMMATRIX offset = model->BoneInfos[boneIndex].Offset;

		// 3. Global Inverse
		XMMATRIX globalInverse = model->GlobalInverse;

		// 4. Final Set (Offset * Global * Inverse)
		XMMATRIX finalTransform = offset * globalTransform * globalInverse;

		model->BoneInfos[boneIndex].FinalTransform = finalTransform;
	}

	// Return To Child Node
	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		ReadNodeHierarchy(model, animTime, node->mChildren[i], globalTransform);
	}
}

static bool Parse_Finger_Bone(const std::string& NodeName, bool isLeft, int& outFingerIdx, int& outJointIdx)
{
	// 1. Check L R
	std::string SidePrefix = isLeft ? "Left" : "Right";
	if (NodeName.find(SidePrefix) == std::string::npos) return false;

	// 2. Check Finger Name
	if (NodeName.find("Thumb") != std::string::npos) outFingerIdx = 0;
	else if (NodeName.find("Index") != std::string::npos) outFingerIdx = 1;
	else if (NodeName.find("Middle") != std::string::npos) outFingerIdx = 2;
	else if (NodeName.find("Ring") != std::string::npos) outFingerIdx = 3;
	else if (NodeName.find("Pinky") != std::string::npos) outFingerIdx = 4;
	else return false; // Not Finger

	// 3. Check Finger Number (e.g: LeftHandIndex1)
	if (NodeName.find("1") != std::string::npos) outJointIdx = 0;
	else if (NodeName.find("2") != std::string::npos) outJointIdx = 1;
	else if (NodeName.find("3") != std::string::npos) outJointIdx = 2;
	else return false; // No Number Or Tip(4)

	return true;
}

void Model_Node_Reset()
{
	// Root
	g_Model_Root_Y = MODEL_ROOT_Y_DEFAULT;

	// Eyes
	g_L_Eye_X = 0.0f, g_L_Eye_Y = 0.0f;
	g_R_Eye_X = 0.0f, g_R_Eye_Y = 0.0f;

	// Clavicle
	g_L_Clavicle_X = L_CLAV_X_DEF; g_L_Clavicle_Y = L_CLAV_Y_DEF; g_L_Clavicle_Z = L_CLAV_Z_DEF;
	g_R_Clavicle_X = R_CLAV_X_DEF; g_R_Clavicle_Y = R_CLAV_Y_DEF; g_R_Clavicle_Z = R_CLAV_Z_DEF;

	// Left Arm
	g_L_Arm_Invert_X = false; g_L_Arm_Invert_Y = false; g_L_Arm_Invert_Z = false;
	g_L_Arm_Upper_X = L_ARM_UPPER_X_DEF; g_L_Arm_Upper_Y = L_ARM_UPPER_Y_DEF; g_L_Arm_Upper_Z = L_ARM_UPPER_Z_DEF;
	g_L_Arm_Fore_X = 0.0f; g_L_Arm_Fore_Y = 0.0f; g_L_Arm_Fore_Z = L_ARM_FORE_Z_DEF;

	// Right Arm
	g_R_Arm_Invert_X = false; g_R_Arm_Invert_Y = false; g_R_Arm_Invert_Z = false;
	g_R_Arm_Upper_X = R_ARM_UPPER_X_DEF; g_R_Arm_Upper_Y = R_ARM_UPPER_Y_DEF; g_R_Arm_Upper_Z = R_ARM_UPPER_Z_DEF;
	g_R_Arm_Fore_X = 0.0f; g_R_Arm_Fore_Y = 0.0f; g_R_Arm_Fore_Z = R_ARM_FORE_Z_DEF;

	// Left Hand
	g_L_Hand_X = HAND_DEF_X; g_L_Hand_Y = HAND_DEF_Y; g_L_Hand_Z = HAND_DEF_Z;
	ZeroMemory(g_L_Finger, sizeof(g_L_Finger));

	// Right Hand
	g_R_Hand_X = HAND_DEF_X; g_R_Hand_Y = HAND_DEF_Y; g_R_Hand_Z = HAND_DEF_Z;
	ZeroMemory(g_R_Finger, sizeof(g_R_Finger));

	// Left Leg
	g_L_Leg_Invert_X = false; g_L_Leg_Invert_Y = false; g_L_Leg_Invert_Z = false;
	g_L_Leg_Upper_X = 0.0f; g_L_Leg_Upper_Y = 0.0f; g_L_Leg_Upper_Z = LEG_UPPER_Z_DEF; // 180
	g_L_Leg_Lower_X = 0.0f; g_L_Leg_Lower_Y = 0.0f; g_L_Leg_Lower_Z = LEG_LOWER_Z_DEF; // 0
	g_L_Foot_X = FOOT_X_DEF; g_L_Foot_Y = 0.0f; g_L_Foot_Z = FOOT_Z_DEF; // X 45, Z 0
	g_L_Toe_X = FOOT_X_DEF; g_L_Toe_Y = 0.0f; g_L_Toe_Z = FOOT_Z_DEF;

	// Right Leg 
	g_R_Leg_Invert_X = false; g_R_Leg_Invert_Y = false; g_R_Leg_Invert_Z = false;
	g_R_Leg_Upper_X = 0.0f; g_R_Leg_Upper_Y = 0.0f; g_R_Leg_Upper_Z = LEG_UPPER_Z_DEF; // 180
	g_R_Leg_Lower_X = 0.0f; g_R_Leg_Lower_Y = 0.0f; g_R_Leg_Lower_Z = LEG_LOWER_Z_DEF; // 0
	g_R_Foot_X = FOOT_X_DEF; g_R_Foot_Y = 0.0f; g_R_Foot_Z = FOOT_Z_DEF; // X 45, Z 0
	g_R_Toe_X = FOOT_X_DEF; g_R_Toe_Y = 0.0f; g_R_Toe_Z = FOOT_Z_DEF;

	// AABB
	Bone_AABB = Bone_AABB_Defalut;
}