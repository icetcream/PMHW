#include "UI/Widget/MHWDamageNumberWidget.h"

#include "Components/TextBlock.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MHWDamageNumberWidget)

void UMHWDamageNumberWidget::SetDamageNumber(float InDamageAmount, EMHWCriticalHitType InCriticalHitType)
{
	DamageAmount = InDamageAmount;
	CriticalHitType = InCriticalHitType;

	if (DamageText)
	{
		DamageText->SetText(FText::AsNumber(FMath::RoundToInt(DamageAmount)));
	}

	OnDamageNumberSet(DamageAmount, CriticalHitType);
}
