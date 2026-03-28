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
	if (Lifetime > 0.0f)
	{
		SetLifeSpan(Lifetime);
	}

	if (!bInitialized)
	{
		StartLocation = GetActorLocation();
	}

	UpdateFacingRotation();
	PushDamageNumberToWidget();
}

void AMHWDamageNumberActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	ElapsedLifetime += DeltaSeconds;
	SetActorLocation(StartLocation + FVector(0.0f, 0.0f, FloatSpeed * ElapsedLifetime));
	UpdateFacingRotation();
}

void AMHWDamageNumberActor::InitializeDamageNumber(AActor* InTargetActor, float InDamageAmount, EMHWCriticalHitType InCriticalHitType, bool bInHasCustomSpawnLocation, FVector InCustomSpawnLocation)
{
	TargetActor = InTargetActor;
	DamageAmount = InDamageAmount;
	CriticalHitType = InCriticalHitType;
	bHasCustomSpawnLocation = bInHasCustomSpawnLocation;
	CustomSpawnLocation = InCustomSpawnLocation;
	ElapsedLifetime = 0.0f;
	StartLocation = bHasCustomSpawnLocation
		? ResolveSpawnLocationFromImpactPoint(CustomSpawnLocation)
		: ResolveSpawnLocation(InTargetActor);
	SetActorLocation(StartLocation);
	bInitialized = true;

	EnsureWidgetReady();
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
		DamageNumberWidget->SetDamageNumber(DamageAmount, CriticalHitType);
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
