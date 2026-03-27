#include "Game/MHWGameModeBase.h"

#include "Character/MHWCombatComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MHWGameModeBase)

bool AMHWGameModeBase::DebugDamagePlayer(float DamageAmount, int32 PlayerIndex)
{
	if (DamageAmount <= 0.0f)
	{
		return false;
	}

	UMHWCombatComponent* CombatComponent = FindPlayerCombatComponent(PlayerIndex);
	if (!CombatComponent || !CombatComponent->ApplyRawDamage(DamageAmount))
	{
		return false;
	}

	LogPlayerVitals(PlayerIndex, TEXT("ApplyDamage"));
	return true;
}

bool AMHWGameModeBase::DebugHealPlayer(float HealAmount, int32 PlayerIndex)
{
	if (HealAmount <= 0.0f)
	{
		return false;
	}

	UMHWCombatComponent* CombatComponent = FindPlayerCombatComponent(PlayerIndex);
	if (!CombatComponent)
	{
		return false;
	}

	CombatComponent->RestoreHealth(HealAmount);
	LogPlayerVitals(PlayerIndex, TEXT("RestoreHealth"));
	return true;
}

bool AMHWGameModeBase::DebugConsumePlayerStamina(float Amount, int32 PlayerIndex)
{
	if (Amount <= 0.0f)
	{
		return false;
	}

	UMHWCombatComponent* CombatComponent = FindPlayerCombatComponent(PlayerIndex);
	if (!CombatComponent || !CombatComponent->ConsumeStamina(Amount))
	{
		return false;
	}

	LogPlayerVitals(PlayerIndex, TEXT("ConsumeStamina"));
	return true;
}

bool AMHWGameModeBase::DebugRestorePlayerStamina(float Amount, int32 PlayerIndex)
{
	if (Amount <= 0.0f)
	{
		return false;
	}

	UMHWCombatComponent* CombatComponent = FindPlayerCombatComponent(PlayerIndex);
	if (!CombatComponent)
	{
		return false;
	}

	CombatComponent->RestoreStamina(Amount);
	LogPlayerVitals(PlayerIndex, TEXT("RestoreStamina"));
	return true;
}

bool AMHWGameModeBase::DebugResetPlayerVitals(int32 PlayerIndex)
{
	UMHWCombatComponent* CombatComponent = FindPlayerCombatComponent(PlayerIndex);
	if (!CombatComponent)
	{
		return false;
	}

	CombatComponent->ResetVitalsToMax();
	LogPlayerVitals(PlayerIndex, TEXT("ResetVitals"));
	return true;
}

bool AMHWGameModeBase::DebugPrintPlayerVitals(int32 PlayerIndex)
{
	if (!FindPlayerCombatComponent(PlayerIndex))
	{
		return false;
	}

	LogPlayerVitals(PlayerIndex, TEXT("PrintVitals"));
	return true;
}

UMHWCombatComponent* AMHWGameModeBase::FindPlayerCombatComponent(int32 PlayerIndex) const
{
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, PlayerIndex);
	APawn* PlayerPawn = PlayerController ? PlayerController->GetPawn() : nullptr;
	return PlayerPawn ? PlayerPawn->FindComponentByClass<UMHWCombatComponent>() : nullptr;
}

void AMHWGameModeBase::LogPlayerVitals(int32 PlayerIndex, const TCHAR* Context) const
{
	const UMHWCombatComponent* CombatComponent = FindPlayerCombatComponent(PlayerIndex);
	if (!CombatComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UI Test] %s failed. No combat component for player index %d."), Context, PlayerIndex);
		return;
	}

	UE_LOG(
		LogTemp,
		Log,
		TEXT("[UI Test] %s | Player=%d | Health=%.1f/%.1f (%.2f) | Stamina=%.1f/%.1f (%.2f)"),
		Context,
		PlayerIndex,
		CombatComponent->GetHealth(),
		CombatComponent->GetMaxHealth(),
		CombatComponent->GetHealthPercent(),
		CombatComponent->GetStamina(),
		CombatComponent->GetMaxStamina(),
		CombatComponent->GetStaminaPercent());
}
