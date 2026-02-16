# ⚡ HYPER TRIGGER

- **A high-speed 3D Roguelike TPS featuring a dynamic BGM system that reacts to weapon changes.**

I plan to continue updating the project to expand the depth of the Roguelike gameplay.\
Feedback is always welcome!

## 🔑 Key Features

- **Efficient Object Pooling System**
  + Implemented **Object Pooling** to manage **numerous active objects**. (e.g. : Bullets, Enemies, Particles, etc)
  + Ensures **stable framerates** by preventing performance drops caused by frequent memory allocation and deallocation.

- **Real-time Animation Debugging Tools**
  + Integrated **ImGui** to build custom tools for real-time bone control and animation blending, significantly boosting development efficiency.

- **Dynamic Audio Module (XAudio2)**
  + Developed a custom audio module using **DirectX XAudio2** and **std::chrono** for high-precision timing.
  + Accurately calculates loop points to enable seamless BGM layering that reacts instantly to gameplay changes.

## 🛠️ Development Environment

* **Language** : C/C++, HLSL
* **Library** : DirectX 11, ImGui, Assimp, DirectXTK, XAudio2
* **IDE & Tools** : Visual Studio, Adobe Photoshop
* **Target Platform:** PC (Windows)
* **Development start date** : 2025.12.11

## 🎮 How To Play

***(Only code is being distributed because the currently used sound source resources are not redistributable.)***

---

## ✨ Patch And Update History
### Prototype (2026.01.13)
- Prototype Completed!
   + Implemented core loop : Battle -> Upgrade -> Loop -> TimeOut.
 
### v1.0 (2026.02.13)
- **Initial Release!**
   + Implemented core Roguelike gameplay mechanics.
   + Enhanced Real-time Animation Tools for better bone control and debugging.

## 📝 Roadmap & Dev Notes
- **This section outlines known issues and future plans for the project.**

---

> ### ⁉️ Known Issues
>   + **Non-implementation of grenade physical logic** : Need More Time..!
>
> ### 🗃️ Planned Improvements
>   + **Level Design & AI Pathfinding** : Designing complex map layouts with strategic cover and implementing the **A* (A-Star) algorithm** for enemies to intelligently navigate obstacles and pursue the player.
>   + **Game Design Depth** : Adding weapon-specific unique upgrades and expanding the card pool to deepen the Roguelike experience.
>
> ### 📬 Received Feedback
>  + ~~***(I`m Waiting!)***~~
>
> ### ✔️ Completed
>  + ***(Nothing Yet)***

---

# ©️ Credits

## ■ Texture ■

### ◆ [ChatGPT](https://chatgpt.com/)
- Texture

### ◆ [OpenGameArt](https://opengameart.org/)
- Explosion Sprite
  
### ◆ [Xbox One Controller Textures](https://forums.dolphin-emu.org/Thread-xbox-one-controller-textures?highlight=Xbox+One+Controller+Textures)
- Controller Button Textures

### ◆ Photoshop
- Textures Editing

## ■ BGM / SFX ■

### ◆ [P.Ray](https://www.youtube.com/@prayofficial)
- All BGM

### ◆ [効果音ラボ (Sound Effect Lab)](https://soundeffect-lab.info/)
- SFX ***(Producers List Editing...)***

## ■ Editor Tools Site ■

### ◆ [123APPS](https://123apps.com/)
- Convert MP3 to WAV
- Edit Volume

## ■ Reference Games ■

- [PUBG: Battlegrounds](https://en.wikipedia.org/wiki/PUBG:_Battlegrounds)
- [Vampire Survivors](https://en.wikipedia.org/wiki/Vampire_Survivors)
- [Slay the Spire](https://en.wikipedia.org/wiki/Slay_the_Spire)
- [HoloCure – Save the Fans!](https://en.wikipedia.org/wiki/HoloCure_%E2%80%93_Save_the_Fans!)

---

## 💻 Code Snippet

```cpp
// Animation Bone Node Code Snippet
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
//......
```
