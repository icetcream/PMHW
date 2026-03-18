#include "MHWGameplayTags.h"

#define DEFINE_INPUT_TAG_SET(BaseVar, TagPath) \
UE_DEFINE_GAMEPLAY_TAG(BaseVar, TagPath); \
UE_DEFINE_GAMEPLAY_TAG(BaseVar##_Hold, TagPath TEXT(".Hold")); \
UE_DEFINE_GAMEPLAY_TAG(BaseVar##_Completed, TagPath TEXT(".Completed"));
namespace MHWTags
{
	// 宏语法：UE_DEFINE_GAMEPLAY_TAG(变量名, "标签.字符串")
    
	// --- Input ---
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Move, "InputTag.Move");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Look_Mouse, "InputTag.Look_Mouse");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Look_Stick, "InputTag.Look_Stick");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Crouch, "InputTag.Crouch");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_AutoRun, "InputTag.AutoRun");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Weapon_ToggleDraw, "InputTag.ToggleDraw");
	
	DEFINE_INPUT_TAG_SET(InputTag_PrimaryAttack, "InputTag.PrimaryAttack");
	DEFINE_INPUT_TAG_SET(InputTag_SecondaryAttack, "InputTag.SecondaryAttack");
	DEFINE_INPUT_TAG_SET(InputTag_SpecialAction, "InputTag.SpecialAction");

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
	UE_DEFINE_GAMEPLAY_TAG(State_ComboWindow, "State.ComboWindow");
	UE_DEFINE_GAMEPLAY_TAG(State_IsMoving, "State.IsMoving");
	
	UE_DEFINE_GAMEPLAY_TAG(State_Combat_Drawn, "State.Combat.Drawn");
	UE_DEFINE_GAMEPLAY_TAG(State_Combat_Charging, "State.Combat.Charging");
	UE_DEFINE_GAMEPLAY_TAG(State_Combat_Charging_XLZ, "State.Combat.Charging.XLZ");
	UE_DEFINE_GAMEPLAY_TAG(State_Combat_Charging_XLZ2, "State.Combat.Charging.XLZ2");
	UE_DEFINE_GAMEPLAY_TAG(State_Combat_Charging_XLZ3, "State.Combat.Charging.XLZ3");
	UE_DEFINE_GAMEPLAY_TAG(State_Rotation_BlockInput, "State.Rotation.BlockInput");
	UE_DEFINE_GAMEPLAY_TAG(State_Movement_BlockInput, "State.Movement.BlockInput");
	UE_DEFINE_GAMEPLAY_TAG(State_CanMove, "State.CanMove");
	
	// --- Msg ---
	UE_DEFINE_GAMEPLAY_TAG(Msg_Locomotion_Idle, "Msg.Locomotion.Idle");
	UE_DEFINE_GAMEPLAY_TAG(Msg_Locomotion_StartMove, "Msg.Locomotion.StartMove");
	UE_DEFINE_GAMEPLAY_TAG(Msg_Locomotion_RunLoop, "Msg.Locomotion.RunLoop");
	UE_DEFINE_GAMEPLAY_TAG(Msg_Locomotion_Stop, "Msg.Locomotion.Stop");
	UE_DEFINE_GAMEPLAY_TAG(Msg_Locomotion_Pivot, "Msg.Locomotion.Pivot");
	UE_DEFINE_GAMEPLAY_TAG(Msg_Animation_Complete, "Msg.Animation.Complete");
	/*UE_DEFINE_GAMEPLAY_TAG(State_Combat_Charging, "State.Combat.Charging");
	UE_DEFINE_GAMEPLAY_TAG(State_Combat_HitPause, "State.Combat.HitPause");
	
	
	

	// --- Ability ---
	UE_DEFINE_GAMEPLAY_TAG(Ability_GreatSword_Charge, "Ability.Weapon.GS.Charge");*/
	
}