#include "Character/MHWCombatPawn.h"

#include "AbilitySystem/Attributes/MHWCombatAttributeSet.h"
#include "AbilitySystem/MHWAbilitySystemComponent.h"
#include "Character/MHWCombatComponent.h"
#include "Components/SceneComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MHWCombatPawn)

AMHWCombatPawn::AMHWCombatPawn(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	RootSceneComponent = ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, TEXT("Root"));
	SetRootComponent(RootSceneComponent);

	AbilitySystemComponent = ObjectInitializer.CreateDefaultSubobject<UMHWAbilitySystemComponent>(this, TEXT("AbilitySystemComponent"));
	CombatAttributeSet = ObjectInitializer.CreateDefaultSubobject<UMHWCombatAttributeSet>(this, TEXT("CombatAttributeSet"));
	CombatComponent = ObjectInitializer.CreateDefaultSubobject<UMHWCombatComponent>(this, TEXT("CombatComponent"));
	CombatComponent->bEnableStaminaAutoRegen = false;
}

UAbilitySystemComponent* AMHWCombatPawn::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AMHWCombatPawn::BeginPlay()
{
	Super::BeginPlay();

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
