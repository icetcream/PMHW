#include "UI/Widget/MHWDamageNumberWidget.h"

#include "Components/TextBlock.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MHWDamageNumberWidget)

void UMHWDamageNumberWidget::SetDamageNumber(float InDamageAmount, EMHWCriticalHitType InCriticalHitType, const FText& InAttackDisplayName)
{
	DamageAmount = InDamageAmount;
	CriticalHitType = InCriticalHitType;
	AttackDisplayName = InAttackDisplayName;

	if (DamageText)
	{
		DamageText->SetText(FText::AsNumber(FMath::RoundToInt(DamageAmount)));
	}

	if (AttackNameText)
	{
		AttackNameText->SetText(AttackDisplayName);
	}

	OnDamageNumberSet(DamageAmount, CriticalHitType, AttackDisplayName);
}
