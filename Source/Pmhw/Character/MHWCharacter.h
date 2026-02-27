// MHWCharacter.h
#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "GameplayTagContainer.h" // 需要包含 GameplayTags 模块
#include "MHWCharacter.generated.h"

class UMHWEquipmentManagerComponent;
class UMHWPawnExtensionComponent;
class UMHWHeroComponent;
class UInputMappingContext;
class UMHWInputConfig;
struct FInputActionValue;

UCLASS()
class PMHW_API AMHWCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AMHWCharacter(const FObjectInitializer& ObjectInitializer);
	
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MHW|Character", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMHWPawnExtensionComponent> MHWPawnExtensionComponent;

	UPROPERTY(VisibleAnywhere, Category = "MHW|Character")
	TObjectPtr<UMHWEquipmentManagerComponent> MHWEquipmentManagerComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "MHW|Character", Meta = (AllowPrivateAccess = "true"))
	FGameplayTagContainer CurrentComboState;
};


