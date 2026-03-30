#include "Character/MHWCombatPawn.h"

#include "AbilitySystem/Attributes/MHWCombatAttributeSet.h"
#include "AbilitySystem/MHWAbilitySystemComponent.h"
#include "Character/MHWCombatComponent.h"
#include "Character/MHWMonsterCombatComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SceneComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MHWCombatPawn)

AMHWCombatPawn::AMHWCombatPawn(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	RootSceneComponent = ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, TEXT("Root"));
	SetRootComponent(RootSceneComponent);

	AbilitySystemComponent = ObjectInitializer.CreateDefaultSubobject<UMHWAbilitySystemComponent>(this, TEXT("AbilitySystemComponent"));
	CombatAttributeSet = ObjectInitializer.CreateDefaultSubobject<UMHWCombatAttributeSet>(this, TEXT("CombatAttributeSet"));
	CombatComponent = ObjectInitializer.CreateDefaultSubobject<UMHWMonsterCombatComponent>(this, TEXT("CombatComponent"));
	CombatComponent->bEnableStaminaAutoRegen = false;
}

UAbilitySystemComponent* AMHWCombatPawn::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AMHWCombatPawn::BeginPlay()
{
	Super::BeginPlay();

	TArray<UPrimitiveComponent*> PrimitiveComponents;
	GetComponents<UPrimitiveComponent>(PrimitiveComponents);
	for (UPrimitiveComponent* PrimitiveComponent : PrimitiveComponents)
	{
		if (!PrimitiveComponent || PrimitiveComponent->GetCollisionEnabled() == ECollisionEnabled::NoCollision)
		{
			continue;
		}

		PrimitiveComponent->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Block);
	}

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
	}
}

float AMHWCombatPawn::GetHealth() const
{
	return CombatComponent ? CombatComponent->GetHealth() : 0.0f;
}

float AMHWCombatPawn::GetMaxHealth() const
{
	return CombatComponent ? CombatComponent->GetMaxHealth() : 0.0f;
}

float AMHWCombatPawn::GetHealthPercent() const
{
	return CombatComponent ? CombatComponent->GetHealthPercent() : 0.0f;
}

bool AMHWCombatPawn::IsAlive() const
{
	return CombatComponent ? CombatComponent->IsAlive() : false;
}

bool AMHWCombatPawn::ApplyRawDamage(float DamageAmount)
{
	return CombatComponent ? CombatComponent->ApplyRawDamage(DamageAmount) : false;
}
