#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h" 

#define DECLARE_INPUT_TAG_SET(BaseName) \
UE_DECLARE_GAMEPLAY_TAG_EXTERN(BaseName); \
UE_DECLARE_GAMEPLAY_TAG_EXTERN(BaseName##_Hold); \
UE_DECLARE_GAMEPLAY_TAG_EXTERN(BaseName##_Completed);

namespace MHWTags
{
	// --- 输入相关标签 (Input Tags) ---
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Move);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Look_Mouse);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Look_Stick);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Crouch);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_AutoRun);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Weapon_ToggleDraw);
	
	// --- 初始化相关 ---
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InitState_Spawned); 
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InitState_DataAvailable);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InitState_DataInitialized);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InitState_GameplayReady);
	
	DECLARE_INPUT_TAG_SET(InputTag_PrimaryAttack);   // 比如三角键
	DECLARE_INPUT_TAG_SET(InputTag_SecondaryAttack); // 比如圆圈键

	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Interact);        // 比如收刀/R1
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Tackle);          // 铁山靠

	// --- 状态相关标签 (State Tags) ---
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Weapon_Sheathed);    // 收刀
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Weapon_Drawn);  // 拔刀
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_CanMove);
	
	// --- Msg locomotion
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Msg_Locomotion_Idle);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Msg_Locomotion_StartMove);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Msg_Locomotion_RunLoop);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Msg_Locomotion_Stop);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Msg_Locomotion_Pivot);
	
	
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Combat_Charging);    // 蓄力中
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Combat_HitPause);    // 卡肉中
	
    
	// --- 技能相关标签 (Ability Tags) ---
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_GreatSword_Charge); // 蓄力技能ID
	
	// 拔剑相关
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_GreatSword_Sheathed);

	
 }