// Fill out your copyright notice in the Description page of Project Settings.


#include "InputActionMappingAsset.h"



bool UInputActionMappingAsset::GetTagSetByInput(FGameplayTag InputTag, FInputActionTagSet& OutTagSet) const
{
	for (const FInputActionTagSet& Set : ActionMappings)
	{
		if (Set.InputTag.Tag == InputTag)
		{
			OutTagSet = Set;
			return true;
		}
	}
	return false;
}
