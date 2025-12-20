// MHWCharacter.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameplayTagContainer.h" // 需要包含 GameplayTags 模块
#include "MHWCharacter.generated.h"

class UMHWPawnExtensionComponent;
class UMHWHeroComponent;
class UInputMappingContext;
class UMHWInputConfig;
struct FInputActionValue;

UCLASS()
class PMHW_API AMHWCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AMHWCharacter(const FObjectInitializer& ObjectInitializer);


	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lyra|Character", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMHWPawnExtensionComponent> MHWPawnExtensionComponent;
};


