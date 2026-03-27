#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MHWGameModeBase.generated.h"

class UMHWCombatComponent;

UCLASS()
class PMHW_API AMHWGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Exec, Category = "MHW|Debug|UI")
	bool DebugDamagePlayer(float DamageAmount = 10.0f, int32 PlayerIndex = 0);

	UFUNCTION(BlueprintCallable, Exec, Category = "MHW|Debug|UI")
	bool DebugHealPlayer(float HealAmount = 10.0f, int32 PlayerIndex = 0);

	UFUNCTION(BlueprintCallable, Exec, Category = "MHW|Debug|UI")
	bool DebugConsumePlayerStamina(float Amount = 25.0f, int32 PlayerIndex = 0);

	UFUNCTION(BlueprintCallable, Exec, Category = "MHW|Debug|UI")
	bool DebugRestorePlayerStamina(float Amount = 25.0f, int32 PlayerIndex = 0);

	UFUNCTION(BlueprintCallable, Exec, Category = "MHW|Debug|UI")
	bool DebugResetPlayerVitals(int32 PlayerIndex = 0);

	UFUNCTION(BlueprintCallable, Exec, Category = "MHW|Debug|UI")
	bool DebugPrintPlayerVitals(int32 PlayerIndex = 0);

protected:
	UMHWCombatComponent* FindPlayerCombatComponent(int32 PlayerIndex) const;
	void LogPlayerVitals(int32 PlayerIndex, const TCHAR* Context) const;
};
