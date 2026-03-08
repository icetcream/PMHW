#include "MHWGameplayTags.h"

namespace MHWTags
{
	// 宏语法：UE_DEFINE_GAMEPLAY_TAG(变量名, "标签.字符串")
    
	// --- Input ---
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Move, "InputTag.Move");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Look_Mouse, "InputTag.Look_Mouse");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Look_Stick, "InputTag.Look_Stick");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Crouch, "InputTag.Crouch");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_AutoRun, "InputTag.AutoRun");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Weapon_ToggleDraw, "InputTag.Weapon.ToggleDraw");
	
	UE_DEFINE_GAMEPLAY_TAG(InputTag_PrimaryAttack, "InputTag.Ability.Primary");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_SecondaryAttack, "InputTag.Ability.Secondary");
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              


	// --- 初始化阶段 ---
	UE_DEFINE_GAMEPLAY_TAG(InitState_Spawned, "InitState.Spawned");
	UE_DEFINE_GAMEPLAY_TAG(InitState_DataAvailable, "InitState.DataAvailable");
	UE_DEFINE_GAMEPLAY_TAG(InitState_DataInitialized, "InitState.DataInitialized");
	UE_DEFINE_GAMEPLAY_TAG(InitState_GameplayReady, "InitState.GameplayReady");
	
	/*
	
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Interact, "InputTag.Interact");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Tackle, "InputTag.Ability.Tackle");
	*/

	// --- State ---
	UE_DEFINE_GAMEPLAY_TAG(State_Weapon_Sheathed, "State.Weapon.Sheathed");
	UE_DEFINE_GAMEPLAY_TAG(State_Weapon_Drawn, "State.Weapon.Drawn");
	UE_DEFINE_GAMEPLAY_TAG(State_CanMove, "State.CanMove");
	
	// --- Msg ---
	UE_DEFINE_GAMEPLAY_TAG(Msg_Locomotion_Idle, "Msg.Locomotion.Idle");
	UE_DEFINE_GAMEPLAY_TAG(Msg_Locomotion_StartMove, "Msg.Locomotion.StartMove");
	UE_DEFINE_GAMEPLAY_TAG(Msg_Locomotion_RunLoop, "Msg.Locomotion.RunLoop");
	UE_DEFINE_GAMEPLAY_TAG(Msg_Locomotion_Stop, "Msg.Locomotion.Stop");
	UE_DEFINE_GAMEPLAY_TAG(Msg_Locomotion_Pivot, "Msg.Locomotion.Pivot");
	/*UE_DEFINE_GAMEPLAY_TAG(State_Combat_Charging, "State.Combat.Charging");
	UE_DEFINE_GAMEPLAY_TAG(State_Combat_HitPause, "State.Combat.HitPause");
	
	
	

	// --- Ability ---
	UE_DEFINE_GAMEPLAY_TAG(Ability_GreatSword_Charge, "Ability.Weapon.GS.Charge");*/
	
}