#include "Animation/AnimState/ANS_MeleeAttackWindow.h"

#include "Character/MHWAttackComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"
#include "Interface/MHWCharacterInterface.h"

void UANS_MeleeAttackWindow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (!MeshComp)
	{
		return;
	}

	AActor* OwnerActor = MeshComp->GetOwner();
	if (!OwnerActor || !OwnerActor->Implements<UMHWCharacterInterface>())
	{
		return;
	}

	if (UMHWAttackComponent* AttackComponent = IMHWCharacterInterface::Execute_GetAttackComponent(OwnerActor))
	{
		AttackComponent->BeginAttackWindow(AttackWindowSpec);
	}
}

void UANS_MeleeAttackWindow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (!MeshComp)
	{
		return;
	}

	AActor* OwnerActor = MeshComp->GetOwner();
	if (!OwnerActor || !OwnerActor->Implements<UMHWCharacterInterface>())
	{
		return;
	}

	if (UMHWAttackComponent* AttackComponent = IMHWCharacterInterface::Execute_GetAttackComponent(OwnerActor))
	{
		AttackComponent->EndAttackWindow();
	}
}

FString UANS_MeleeAttackWindow::GetNotifyName_Implementation() const
{
	const FString AttackName = AttackWindowSpec.AttackId.IsNone()
		? TEXT("None")
		: AttackWindowSpec.AttackId.ToString();
	return FString::Printf(TEXT("MeleeWindow[%s:%d]"), *AttackName, AttackWindowSpec.WindowIndex);
}
