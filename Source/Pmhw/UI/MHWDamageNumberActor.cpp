#include "UI/MHWDamageNumberActor.h"

#include "Components/SceneComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/PlayerController.h"
#include "UI/Widget/MHWDamageNumberWidget.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MHWDamageNumberActor)

AMHWDamageNumberActor::AMHWDamageNumberActor()
{
	PrimaryActorTick.bCanEverTick = true;

	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(RootSceneComponent);

	WidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("WidgetComponent"));
	WidgetComponent->SetupAttachment(RootSceneComponent);
	WidgetComponent->SetWidgetSpace(EWidgetSpace::World);
	WidgetComponent->SetDrawAtDesiredSize(true);
	WidgetComponent->SetPivot(FVector2D(0.5f, 0.5f));
	WidgetComponent->SetTwoSided(true);
	WidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WidgetComponent->SetGenerateOverlapEvents(false);
	WidgetComponent->SetCastShadow(false);
}

void AMHWDamageNumberActor::BeginPlay()
{
	Super::BeginPlay();

	EnsureWidgetReady();
	SetDamageNumberActive(false);
}

void AMHWDamageNumberActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bInUse)
	{
		return;
	}

	ElapsedLifetime += DeltaSeconds;
	if (Lifetime > 0.0f && ElapsedLifetime >= Lifetime)
	{
		DeactivateDamageNumber();
		return;
	}

	SetActorLocation(StartLocation + FVector(0.0f, 0.0f, FloatSpeed * ElapsedLifetime));
	UpdateFacingRotation();
}

void AMHWDamageNumberActor::InitializeDamageNumber(AActor* InTargetActor, float InDamageAmount, EMHWCriticalHitType InCriticalHitType, FString InAttackDisplayName, bool bInHasCustomSpawnLocation, FVector InCustomSpawnLocation)
{
	TargetActor = InTargetActor;
	DamageAmount = InDamageAmount;
	CriticalHitType = InCriticalHitType;
	AttackDisplayName = InAttackDisplayName;
	bHasCustomSpawnLocation = bInHasCustomSpawnLocation;
	CustomSpawnLocation = InCustomSpawnLocation;
	ElapsedLifetime = 0.0f;
	StartLocation = bHasCustomSpawnLocation
		? ResolveSpawnLocationFromImpactPoint(CustomSpawnLocation)
		: ResolveSpawnLocation(InTargetActor);
	SetActorLocation(StartLocation);
	bInitialized = true;
	bInUse = true;

	EnsureWidgetReady();
	SetDamageNumberActive(true);
	PushDamageNumberToWidget();
	UpdateFacingRotation();
}

void AMHWDamageNumberActor::EnsureWidgetReady()
{
	if (DamageNumberWidgetClass && WidgetComponent->GetWidgetClass() != DamageNumberWidgetClass)
	{
		WidgetComponent->SetWidgetClass(DamageNumberWidgetClass);
	}

	WidgetComponent->InitWidget();
}

void AMHWDamageNumberActor::PushDamageNumberToWidget()
{
	if (UMHWDamageNumberWidget* DamageNumberWidget = Cast<UMHWDamageNumberWidget>(WidgetComponent->GetUserWidgetObject()))
	{
		DamageNumberWidget->SetDamageNumber(DamageAmount, CriticalHitType, FText::FromString(AttackDisplayName));
	}
}

void AMHWDamageNumberActor::UpdateFacingRotation()
{
	if (const UWorld* World = GetWorld())
	{
		if (const APlayerController* PlayerController = World->GetFirstPlayerController())
		{
			if (PlayerController->PlayerCameraManager)
			{
				const FVector CameraLocation = PlayerController->PlayerCameraManager->GetCameraLocation();
				const FRotator LookAtRotation = (CameraLocation - GetActorLocation()).Rotation();
				SetActorRotation(LookAtRotation);
			}
		}
	}
}

void AMHWDamageNumberActor::SetDamageNumberActive(bool bInActive)
{
	SetActorHiddenInGame(!bInActive);
	SetActorTickEnabled(bInActive);
	SetActorEnableCollision(false);

	if (WidgetComponent)
	{
		WidgetComponent->SetVisibility(bInActive, true);
		WidgetComponent->SetHiddenInGame(!bInActive);
	}
}

void AMHWDamageNumberActor::DeactivateDamageNumber()
{
	bInUse = false;
	bInitialized = false;
	ElapsedLifetime = 0.0f;
	TargetActor = nullptr;
	bHasCustomSpawnLocation = false;
	CustomSpawnLocation = FVector::ZeroVector;
	DamageAmount = 0.0f;
	CriticalHitType = EMHWCriticalHitType::None;
	AttackDisplayName.Reset();
	SetDamageNumberActive(false);
}

FVector AMHWDamageNumberActor::ResolveSpawnLocation(AActor* InTargetActor) const
{
	if (!InTargetActor)
	{
		return GetActorLocation();
	}

	FVector BoundsOrigin = InTargetActor->GetActorLocation();
	FVector BoundsExtent = FVector::ZeroVector;
	InTargetActor->GetActorBounds(true, BoundsOrigin, BoundsExtent);

	FVector CameraRightVector = FVector::RightVector;
	if (const UWorld* World = GetWorld())
	{
		if (const APlayerController* PlayerController = World->GetFirstPlayerController())
		{
			if (PlayerController->PlayerCameraManager)
			{
				CameraRightVector = PlayerController->PlayerCameraManager->GetCameraRotation().RotateVector(FVector::RightVector);
				CameraRightVector.Z = 0.0f;
				CameraRightVector = CameraRightVector.GetSafeNormal();
			}
		}
	}

	if (CameraRightVector.IsNearlyZero())
	{
		CameraRightVector = FVector::RightVector;
	}

	const float RightOffset = ImpactPointRightOffset + FMath::FRandRange(-RandomHorizontalOffset, RandomHorizontalOffset);
	return BoundsOrigin + CameraRightVector * RightOffset + FVector(0.0f, 0.0f, SpawnHeightOffset);
}

FVector AMHWDamageNumberActor::ResolveSpawnLocationFromImpactPoint(const FVector& ImpactPoint) const
{
	FVector CameraRightVector = FVector::RightVector;

	if (const UWorld* World = GetWorld())
	{
		if (const APlayerController* PlayerController = World->GetFirstPlayerController())
		{
			if (PlayerController->PlayerCameraManager)
			{
				CameraRightVector = PlayerController->PlayerCameraManager->GetCameraRotation().RotateVector(FVector::RightVector);
				CameraRightVector.Z = 0.0f;
				CameraRightVector = CameraRightVector.GetSafeNormal();
			}
		}
	}

	if (CameraRightVector.IsNearlyZero())
	{
		CameraRightVector = FVector::RightVector;
	}

	return ImpactPoint + CameraRightVector * ImpactPointRightOffset + FVector(0.0f, 0.0f, SpawnHeightOffset);
}
