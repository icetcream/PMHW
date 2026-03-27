#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MHWUserWidgetBase.generated.h"

UCLASS()
class PMHW_API UMHWUserWidgetBase : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = "MHW|UI")
	TObjectPtr<UObject> WidgetController;

	UFUNCTION(BlueprintCallable, Category = "MHW|UI")
	void SetWidgetController(UObject* NewWidgetController);

protected:
	virtual void OnWidgetControllerSet();

	UFUNCTION(BlueprintImplementableEvent, Category = "MHW|UI")
	void WidgetControllerSet();
};
