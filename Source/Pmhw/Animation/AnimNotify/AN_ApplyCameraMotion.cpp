#include "Animation/AnimNotify/AN_ApplyCameraMotion.h"

#include "GameFramework/Pawn.h"
#include "Player/MHWPlayerController.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AN_ApplyCameraMotion)

namespace ApplyCameraMotionNotify
{
	static AMHWPlayerController* GetLocalPlayerController(USkeletalMeshComponent* MeshComp)
	{
		APawn* OwnerPawn = MeshComp ? Cast<APawn>(MeshComp->GetOwner()) : nullptr;
		AMHWPlayerController* PlayerController = OwnerPawn ? Cast<AMHWPlayerController>(OwnerPawn->GetController()) : nullptr;
		return PlayerController && PlayerController->IsLocalController() ? PlayerController : nullptr;
	}
}

void UAN_ApplyCameraMotion::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	AMHWPlayerController* PlayerController = ApplyCameraMotionNotify::GetLocalPlayerController(MeshComp);
	if (!PlayerController)
	{
		return;
	}

	if (bResetToDefault)
	{
		PlayerController->ClearCombatCameraMotion(ResetBlendDuration);
		return;
	}

	if (CameraMotionTag.IsValid() && PlayerController->ApplyCombatCameraMotionByTag(CameraMotionTag))
	{
		return;
	}

	PlayerController->ApplyCombatCameraMotion(CameraMotion);
}

FString UAN_ApplyCameraMotion::GetNotifyName_Implementation() const
{
	return bResetToDefault ? TEXT("Reset Camera Motion") : TEXT("Apply Camera Motion");
}
