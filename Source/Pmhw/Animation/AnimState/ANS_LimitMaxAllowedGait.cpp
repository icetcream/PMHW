#include "Animation/AnimState/ANS_LimitMaxAllowedGait.h"

#include "Character/MHWCharacter.h"
#include "Components/SkeletalMeshComponent.h"

void UANS_LimitMaxAllowedGait::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (!MeshComp || !MaxAllowedGait.IsValid())
	{
		return;
	}

	if (AMHWCharacter* Character = Cast<AMHWCharacter>(MeshComp->GetOwner()))
	{
		Character->SetMaxAllowedGaitOverride(MaxAllowedGait);
	}
}

void UANS_LimitMaxAllowedGait::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (!bClearOverrideOnEnd || !MeshComp)
	{
		return;
	}

	if (AMHWCharacter* Character = Cast<AMHWCharacter>(MeshComp->GetOwner()))
	{
		Character->ClearMaxAllowedGaitOverride();
	}
}

FString UANS_LimitMaxAllowedGait::GetNotifyName_Implementation() const
{
	return FString::Printf(TEXT("LimitGait[%s]"), *MaxAllowedGait.ToString());
}
