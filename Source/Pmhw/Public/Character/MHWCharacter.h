// MHWCharacter.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameplayTagContainer.h" // 需要包含 GameplayTags 模块
#include "MHWCharacter.generated.h"

class UInputMappingContext;
class UMHWInputConfig;
struct FInputActionValue;

UCLASS()
class PMHW_API AMHWCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AMHWCharacter(const FObjectInitializer& ObjectInitializer);
	
};


