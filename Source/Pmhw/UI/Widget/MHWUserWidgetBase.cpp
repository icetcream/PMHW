#include "UI/Widget/MHWUserWidgetBase.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MHWUserWidgetBase)

void UMHWUserWidgetBase::SetWidgetController(UObject* NewWidgetController)
{
	WidgetController = NewWidgetController;
	OnWidgetControllerSet();
	WidgetControllerSet();
}

void UMHWUserWidgetBase::OnWidgetControllerSet()
{
}
