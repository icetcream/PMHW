// MHWCharacter.h
#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "GameplayTagContainer.h" // 需要包含 GameplayTags 模块
#include "Interface/MHWCharacterInterface.h"
#include "MHWCharacter.generated.h"

class UStateTreeComponent;
class UMHWEquipmentManagerComponent;
class UMHWPawnExtensionComponent;
class UMHWHeroComponent;
class UInputMappingContext;
class UMHWInputConfig;
struct FInputActionValue;

UCLASS()
class PMHW_API AMHWCharacter : public ACharacter, public IAbilitySystemInterface, public IMHWCharacterInterface
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
	virtual const UMHWEquipmentManagerComponent* GetEquipmentManagerComponent_Implementation() override;
	virtual UStateTreeComponent* GetStateTreeComponent_Implementation() override;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MHW|Character", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMHWPawnExtensionComponent> MHWPawnExtensionComponent;

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly, Category = "MHW|Character", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMHWEquipmentManagerComponent> MHWEquipmentManagerComponent;
	
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly, Category = "MHW|Character", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStateTreeComponent> MHWStateTreeComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "MHW|Character", Meta = (AllowPrivateAccess = "true"))
	FGameplayTagContainer CurrentComboState;
};


