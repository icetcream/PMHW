#include "MHWGameplayTags.h"

#define DEFINE_INPUT_TAG_SET(BaseVar, TagPath) \
UE_DEFINE_GAMEPLAY_TAG(BaseVar, TagPath); \
UE_DEFINE_GAMEPLAY_TAG(BaseVar##_Hold, TagPath TEXT(".Hold")); \
UE_DEFINE_GAMEPLAY_TAG(BaseVar##_Completed, TagPath TEXT(".Completed"));
namespace MHWInputTags
{
	// 宏语法：UE_DEFINE_GAMEPLAY_TAG(变量名, "标签.字符串")
    
	// --- Input ---
	UE_DEFINE_GAMEPLAY_TAG(Move, "InputTag.Move");
	UE_DEFINE_GAMEPLAY_TAG(Look_Mouse, "InputTag.Look_Mouse");
	UE_DEFINE_GAMEPLAY_TAG(Weapon_ToggleDraw, "InputTag.ToggleDraw");
	UE_DEFINE_GAMEPLAY_TAG(LeftShift, "InputTag.LeftShift");
	
	DEFINE_INPUT_TAG_SET(PrimaryAttack, "InputTag.PrimaryAttack");
	DEFINE_INPUT_TAG_SET(SecondaryAttack, "InputTag.SecondaryAttack");
	DEFINE_INPUT_TAG_SET(SpecialAction, "InputTag.SpecialAction");
	DEFINE_INPUT_TAG_SET(SpecialAction2, "InputTag.SpecialAction2");
	DEFINE_INPUT_TAG_SET(Roll, "InputTag.Roll");
	DEFINE_INPUT_TAG_SET(Defense, "InputTag.Defense");
	DEFINE_INPUT_TAG_SET(QHSJ, "InputTag.QHSJ");
}

namespace MHWInitStateTags
{
	UE_DEFINE_GAMEPLAY_TAG(Spawned, "InitState.Spawned");
	UE_DEFINE_GAMEPLAY_TAG(DataAvailable, "InitState.DataAvailable");
	UE_DEFINE_GAMEPLAY_TAG(DataInitialized, "InitState.DataInitialized");
	UE_DEFINE_GAMEPLAY_TAG(GameplayReady, "InitState.GameplayReady");
	
}

namespace MHWStateTags
{
	// --- 初始化阶段 ---
	// --- state
	UE_DEFINE_GAMEPLAY_TAG(ComboWindow, "State.ComboWindow");
	UE_DEFINE_GAMEPLAY_TAG(ComboWindow_First, "State.ComboWindow.First");
	UE_DEFINE_GAMEPLAY_TAG(ComboWindow_Second, "State.ComboWindow.Second");
	UE_DEFINE_GAMEPLAY_TAG(IsMoving, "State.IsMoving");
	
	UE_DEFINE_GAMEPLAY_TAG(Combat_Drawn, "State.Combat.Drawn");
	UE_DEFINE_GAMEPLAY_TAG(Combat_Defense, "State.Combat.Defense");
	UE_DEFINE_GAMEPLAY_TAG(Combat_Charging, "State.Combat.Charging");
	UE_DEFINE_GAMEPLAY_TAG(Combat_Charging_XLZ, "State.Combat.Charging.XLZ");
	UE_DEFINE_GAMEPLAY_TAG(Combat_Charging_XLZ2, "State.Combat.Charging.XLZ2");
	UE_DEFINE_GAMEPLAY_TAG(Combat_Charging_XLZ3, "State.Combat.Charging.XLZ3");
	UE_DEFINE_GAMEPLAY_TAG(Rotation_BlockRotation, "State.Rotation.BlockRotation");
	UE_DEFINE_GAMEPLAY_TAG(Movement_BlockMove, "State.Movement.BlockMove");
	
	// --- Msg ---
	UE_DEFINE_GAMEPLAY_TAG(Locomotion_Idle, "State.Locomotion.Idle");
	UE_DEFINE_GAMEPLAY_TAG(Locomotion_StartMove, "State.Locomotion.StartMove");
	UE_DEFINE_GAMEPLAY_TAG(Locomotion_RunLoop, "State.Locomotion.RunLoop");
	UE_DEFINE_GAMEPLAY_TAG(Locomotion_Stop, "State.Locomotion.Stop");
	UE_DEFINE_GAMEPLAY_TAG(Locomotion_Pivot, "State.Locomotion.Pivot");
}

namespace MHWGaitTags
{
	UE_DEFINE_GAMEPLAY_TAG(Walking, FName{TEXTVIEW("Als.Gait.Walking")})
	UE_DEFINE_GAMEPLAY_TAG(Running, FName{TEXTVIEW("Als.Gait.Running")})
	UE_DEFINE_GAMEPLAY_TAG(Sprinting, FName{TEXTVIEW("Als.Gait.Sprinting")})
}
namespace MHWLocomotionModeTags
{
	UE_DEFINE_GAMEPLAY_TAG(Grounded, FName{TEXTVIEW("Als.LocomotionMode.Grounded")})
	UE_DEFINE_GAMEPLAY_TAG(InAir, FName{TEXTVIEW("Als.LocomotionMode.InAir")})
}
namespace MHWStanceTags
{
	UE_DEFINE_GAMEPLAY_TAG(Standing, FName{TEXTVIEW("Als.Stance.Standing")})
	UE_DEFINE_GAMEPLAY_TAG(Crouching, FName{TEXTVIEW("Als.Stance.Crouching")})
}

namespace MHWWeaponTags
{
	UE_DEFINE_GAMEPLAY_TAG(Sheathed, FName{TEXTVIEW("state.Weapon.Sheathed")})
	UE_DEFINE_GAMEPLAY_TAG(GreatSword, FName{TEXTVIEW("state.Weapon.GreatSword")})
}

namespace MHWMessageTags
{
	UE_DEFINE_GAMEPLAY_TAG(Animation_Complete, "Msg.Animation.Complete");
}
namespace MHWRotationModeTags
{
	UE_DEFINE_GAMEPLAY_TAG(VelocityDirection, FName{TEXTVIEW("Als.RotationMode.VelocityDirection")})
	UE_DEFINE_GAMEPLAY_TAG(EnemyDirection, FName{TEXTVIEW("Als.RotationMode.EnemyDirection")})
}
