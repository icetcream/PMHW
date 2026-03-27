#include "AbilitySystem/MHWDamageGameplayEffect.h"

#include "AbilitySystem/Executions/MHWDamageExecution.h"
#include "Executions/MHWDamageExecution.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MHWDamageGameplayEffect)

UMHWDamageGameplayEffect::UMHWDamageGameplayEffect()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	FGameplayEffectExecutionDefinition ExecutionDefinition;
	ExecutionDefinition.CalculationClass = UMHWDamageExecution::StaticClass();
	Executions.Add(ExecutionDefinition);
}
