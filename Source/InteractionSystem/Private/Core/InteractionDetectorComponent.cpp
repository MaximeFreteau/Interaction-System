// Copyright Maxime Freteau. All Rights Reserved.

#include "Core/InteractionDetectorComponent.h"
#include "Core/InteractionComponent.h"
#include "Core/InteractionSettings.h"
#include "Interface/InteractableInterface.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"
#include "Engine/OverlapResult.h"
#include "CollisionQueryParams.h"
#include "WorldCollision.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"

UInteractionDetectorComponent::UInteractionDetectorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UInteractionDetectorComponent::BeginPlay()
{
    Super::BeginPlay();

    const UInteractionSettings* Settings = GetDefault<UInteractionSettings>();
    if (Settings)
    {
        if (DetectionMethod == EInteractionDetectionMethod::Raycast)
        {
            DetectionMethod = Settings->DefaultDetectionMethod;
        }
        if (DetectionDistance <= 0.0f)
        {
            DetectionDistance = Settings->DefaultInteractionDistance;
        }
        if (SphereRadius <= 0.0f)
        {
            SphereRadius = Settings->SphereOverlapRadius;
        }
        if (DetectionFrequency <= 0.0f)
        {
            DetectionFrequency = Settings->DetectionFrequency;
        }
        TraceChannel = Settings->InteractionTraceChannel;
        bAllowHoldToInteract = Settings->bAllowHoldToInteract;
        bShowDebug = Settings->bShowDebugTraces;

        if (!InteractInputAction && Settings->DefaultInteractInputAction.IsValid())
        {
            InteractInputAction = Settings->DefaultInteractInputAction.LoadSynchronous();
        }

        if (!InputMappingContext && Settings->DefaultInputMappingContext.IsValid())
        {
            InputMappingContext = Settings->DefaultInputMappingContext.LoadSynchronous();
        }

        if (!PromptWidgetClass && Settings->DefaultPromptWidgetClass.IsValid())
        {
            PromptWidgetClass = Settings->DefaultPromptWidgetClass.LoadSynchronous();
        }
    }

    if (bAutoSetupInput)
    {
        SetupInputBinding();
    }

    if (bAutoCreateWidget && PromptWidgetClass)
    {
        CreatePromptWidget();
    }

    if (DetectionFrequency > 0.0f)
    {
        float TimerInterval = 1.0f / DetectionFrequency;
        GetWorld()->GetTimerManager().SetTimer(
            DetectionTimerHandle,
            this,
            &UInteractionDetectorComponent::DetectInteractables,
            TimerInterval,
            true
        );
    }
}

void UInteractionDetectorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bIsHoldingInteraction)
    {
        UpdateHoldInteractionProgress(DeltaTime);
    }
}

void UInteractionDetectorComponent::DetectInteractables()
{
    AActor* NewFocusedActor = nullptr;

    switch (DetectionMethod)
    {
        case EInteractionDetectionMethod::Raycast:
            NewFocusedActor = DetectViaRaycast();
            break;

        case EInteractionDetectionMethod::Sphere:
            NewFocusedActor = DetectViaSphereOverlap();
            break;

        case EInteractionDetectionMethod::Box:
            NewFocusedActor = DetectViaBoxOverlap();
            break;

        default:
            break;
    }

    if (NewFocusedActor != FocusedActor)
    {
        SetFocusedActor(NewFocusedActor);
    }
}

AActor* UInteractionDetectorComponent::DetectViaRaycast()
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return nullptr;
    }

    FVector StartLocation;
    FRotator ViewRotation;

    if (APlayerController* PC = Cast<APlayerController>(Owner))
    {
        PC->GetPlayerViewPoint(StartLocation, ViewRotation);
    }
    else if (ACharacter* Character = Cast<ACharacter>(Owner))
    {
        if (UCameraComponent* Camera = Character->FindComponentByClass<UCameraComponent>())
        {
            StartLocation = Camera->GetComponentLocation();
            ViewRotation = Camera->GetComponentRotation();
        }
        else
        {
            StartLocation = Character->GetActorLocation();
            ViewRotation = Character->GetActorRotation();
        }
    }
    else
    {
        StartLocation = Owner->GetActorLocation();
        ViewRotation = Owner->GetActorRotation();
    }

    FVector EndLocation = StartLocation + (ViewRotation.Vector() * DetectionDistance);

    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(Owner);

    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        StartLocation,
        EndLocation,
        TraceChannel,
        QueryParams
    );

    if (bShowDebug)
    {
        DrawDebugLine(
            GetWorld(),
            StartLocation,
            EndLocation,
            bHit ? FColor::Green : FColor::Red,
            false,
            DebugDuration,
            0,
            2.0f
        );

        if (bHit)
        {
            DrawDebugSphere(
                GetWorld(),
                HitResult.ImpactPoint,
                10.0f,
                12,
                FColor::Green,
                false,
                DebugDuration
            );
        }
    }

    if (bHit && HitResult.GetActor())
    {
        AActor* HitActor = HitResult.GetActor();
        if (IsActorInteractable(HitActor))
        {
            return HitActor;
        }
    }

    return nullptr;
}

AActor* UInteractionDetectorComponent::DetectViaSphereOverlap()
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return nullptr;
    }

    FVector OwnerLocation = Owner->GetActorLocation();

    TArray<FOverlapResult> Overlaps;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(Owner);

    GetWorld()->OverlapMultiByChannel(
        Overlaps,
        OwnerLocation,
        FQuat::Identity,
        TraceChannel,
        FCollisionShape::MakeSphere(SphereRadius),
        QueryParams
    );

    if (bShowDebug)
    {
        DrawDebugSphere(
            GetWorld(),
            OwnerLocation,
            SphereRadius,
            16,
            Overlaps.Num() > 0 ? FColor::Green : FColor::Red,
            false,
            DebugDuration
        );
    }

    TArray<AActor*> InteractableActors;
    for (const FOverlapResult& Overlap : Overlaps)
    {
        AActor* OverlappedActor = Overlap.GetActor();
        if (OverlappedActor && IsActorInteractable(OverlappedActor))
        {
            InteractableActors.Add(OverlappedActor);
        }
    }

    return FilterByPriority(InteractableActors);
}

AActor* UInteractionDetectorComponent::DetectViaBoxOverlap()
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return nullptr;
    }

    FVector OwnerLocation = Owner->GetActorLocation();
    FRotator OwnerRotation = Owner->GetActorRotation();

    TArray<FOverlapResult> Overlaps;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(Owner);

    GetWorld()->OverlapMultiByChannel(
        Overlaps,
        OwnerLocation,
        OwnerRotation.Quaternion(),
        TraceChannel,
        FCollisionShape::MakeBox(BoxExtent),
        QueryParams
    );

    if (bShowDebug)
    {
        DrawDebugBox(
            GetWorld(),
            OwnerLocation,
            BoxExtent,
            OwnerRotation.Quaternion(),
            Overlaps.Num() > 0 ? FColor::Green : FColor::Red,
            false,
            DebugDuration
        );
    }

    TArray<AActor*> InteractableActors;
    for (const FOverlapResult& Overlap : Overlaps)
    {
        AActor* OverlappedActor = Overlap.GetActor();
        if (OverlappedActor && IsActorInteractable(OverlappedActor))
        {
            InteractableActors.Add(OverlappedActor);
        }
    }

    return FilterByPriority(InteractableActors);
}

AActor* UInteractionDetectorComponent::FilterByPriority(const TArray<AActor*>& Actors)
{
    if (Actors.Num() == 0)
    {
        return nullptr;
    }

    if (Actors.Num() == 1)
    {
        return Actors[0];
    }

    AActor* Owner = GetOwner();
    FVector OwnerLocation = Owner ? Owner->GetActorLocation() : FVector::ZeroVector;

    AActor* BestActor = nullptr;
    EInteractionPriority HighestPriority = EInteractionPriority::VeryLow;
    float ClosestDistance = MAX_FLT;

    for (AActor* Actor : Actors)
    {
        UInteractionComponent* InteractionComp = GetInteractionComponent(Actor);
        if (!InteractionComp)
        {
            continue;
        }

        FInteractionData Data = InteractionComp->GetInteractionInfo();
        float Distance = FVector::Distance(OwnerLocation, Actor->GetActorLocation());

        if (Data.Priority > HighestPriority)
        {
            BestActor = Actor;
            HighestPriority = Data.Priority;
            ClosestDistance = Distance;
        }
        else if (Data.Priority == HighestPriority && Distance < ClosestDistance)
        {
            BestActor = Actor;
            ClosestDistance = Distance;
        }
    }

    return BestActor;
}

bool UInteractionDetectorComponent::IsActorInteractable(AActor* Actor) const
{
    if (!Actor)
    {
        return false;
    }

    UInteractionComponent* InteractionComp = Actor->FindComponentByClass<UInteractionComponent>();
    if (InteractionComp && InteractionComp->CanInteractWith(GetOwner()))
    {
        return true;
    }

    if (Actor->Implements<UInteractableInterface>())
    {
        return IInteractableInterface::Execute_CanInteract(Actor, GetOwner());
    }

    return false;
}

UInteractionComponent* UInteractionDetectorComponent::GetInteractionComponent(AActor* Actor) const
{
    if (!Actor)
    {
        return nullptr;
    }

    return Actor->FindComponentByClass<UInteractionComponent>();
}

void UInteractionDetectorComponent::SetFocusedActor(AActor* NewFocusedActor)
{
    if (FocusedActor && FocusedActor != NewFocusedActor)
    {
        if (FocusedInteractionComponent)
        {
            FocusedInteractionComponent->OnUnfocused_Implementation(GetOwner());
        }
        else if (FocusedActor->Implements<UInteractableInterface>())
        {
            IInteractableInterface::Execute_OnUnfocused(FocusedActor, GetOwner());
        }

        OnFocusLost.Broadcast(FocusedActor);
        HidePrompt();
    }

    FocusedActor = NewFocusedActor;
    FocusedInteractionComponent = NewFocusedActor ? GetInteractionComponent(NewFocusedActor) : nullptr;

    if (FocusedActor)
    {
        FInteractionData InteractionData = GetFocusedInteractionData();

        if (FocusedInteractionComponent)
        {
            FocusedInteractionComponent->OnFocused_Implementation(GetOwner());
        }
        else if (FocusedActor->Implements<UInteractableInterface>())
        {
            IInteractableInterface::Execute_OnFocused(FocusedActor, GetOwner());
        }

        OnFocusGained.Broadcast(FocusedActor);
        OnInteractableFound.Broadcast(FocusedActor, InteractionData);
        ShowPrompt();
    }
}

FInteractionData UInteractionDetectorComponent::GetFocusedInteractionData() const
{
    if (FocusedInteractionComponent)
    {
        return FocusedInteractionComponent->GetInteractionInfo();
    }

    if (FocusedActor && FocusedActor->Implements<UInteractableInterface>())
    {
        return IInteractableInterface::Execute_GetInteractionData(FocusedActor);
    }

    return FInteractionData();
}

void UInteractionDetectorComponent::InteractWithFocused()
{
    if (!FocusedActor)
    {
        return;
    }

    FInteractionData InteractionData = GetFocusedInteractionData();

    if (InteractionData.InteractionDuration > 0.0f && bAllowHoldToInteract)
    {
        StartHoldInteraction();
        return;
    }

    if (FocusedInteractionComponent)
    {
        FocusedInteractionComponent->Interact_Implementation(GetOwner());
    }
    else if (FocusedActor->Implements<UInteractableInterface>())
    {
        IInteractableInterface::Execute_Interact(FocusedActor, GetOwner());
    }
}

void UInteractionDetectorComponent::StartHoldInteraction()
{
    if (!FocusedActor || bIsHoldingInteraction)
    {
        return;
    }

    FInteractionData InteractionData = GetFocusedInteractionData();
    CurrentInteractionDuration = InteractionData.InteractionDuration;

    if (CurrentInteractionDuration <= 0.0f)
    {
        InteractWithFocused();
        return;
    }

    bIsHoldingInteraction = true;
    HoldProgress = 0.0f;

    if (FocusedInteractionComponent)
    {
        FocusedInteractionComponent->StartInteraction(GetOwner());
    }
}

void UInteractionDetectorComponent::StopHoldInteraction()
{
    if (!bIsHoldingInteraction)
    {
        return;
    }

    if (HoldProgress < 1.0f && FocusedInteractionComponent)
    {
        FocusedInteractionComponent->CancelInteraction(GetOwner());
    }

    bIsHoldingInteraction = false;
    HoldProgress = 0.0f;
}

void UInteractionDetectorComponent::UpdateHoldInteractionProgress(float DeltaTime)
{
    if (!bIsHoldingInteraction || CurrentInteractionDuration <= 0.0f)
    {
        return;
    }

    HoldProgress += DeltaTime / CurrentInteractionDuration;
    HoldProgress = FMath::Clamp(HoldProgress, 0.0f, 1.0f);

    if (FocusedInteractionComponent)
    {
        FocusedInteractionComponent->UpdateInteractionProgress(HoldProgress);
    }

    if (HoldProgress >= 1.0f)
    {
        bIsHoldingInteraction = false;
        HoldProgress = 0.0f;
    }
}

void UInteractionDetectorComponent::SetupInputBinding()
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    APlayerController* PC = Cast<APlayerController>(Owner);
    if (!PC)
    {
        if (APawn* Pawn = Cast<APawn>(Owner))
        {
            PC = Cast<APlayerController>(Pawn->GetController());
        }
    }

    if (!PC)
    {
        return;
    }

    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
    {
        if (InputMappingContext)
        {
            Subsystem->AddMappingContext(InputMappingContext, MappingPriority);
        }
        else
        {
        }
    }

    if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PC->InputComponent))
    {
        if (InteractInputAction)
        {
            EnhancedInput->BindAction(InteractInputAction, ETriggerEvent::Started, this, &UInteractionDetectorComponent::OnInteractStarted);
            
            EnhancedInput->BindAction(InteractInputAction, ETriggerEvent::Completed, this, &UInteractionDetectorComponent::OnInteractCompleted);
        }
    }
}

void UInteractionDetectorComponent::OnInteractStarted(const FInputActionValue& Value)
{
    if (FocusedActor)
    {
        FInteractionData InteractionData = GetFocusedInteractionData();

        if (InteractionData.InteractionDuration > 0.0f && bAllowHoldToInteract)
        {
            StartHoldInteraction();
        }
        else
        {
            InteractWithFocused();
        }
    }
}

void UInteractionDetectorComponent::OnInteractCompleted(const FInputActionValue& Value)
{
    if (bIsHoldingInteraction)
    {
        StopHoldInteraction();
    }
}

void UInteractionDetectorComponent::CreatePromptWidget()
{
    if (!PromptWidgetClass)
    {
        return;
    }

    APlayerController* PC = Cast<APlayerController>(GetOwner());
    if (!PC)
    {
        if (APawn* Pawn = Cast<APawn>(GetOwner()))
        {
            PC = Cast<APlayerController>(Pawn->GetController());
        }
    }

    if (!PC)
    {
        return;
    }

    PromptWidget = CreateWidget<UUserWidget>(PC, PromptWidgetClass);
    if (PromptWidget)
    {
        PromptWidget->AddToViewport();
        PromptWidget->SetVisibility(ESlateVisibility::Hidden);
    }
}

void UInteractionDetectorComponent::ShowPrompt()
{
    if (PromptWidget)
    {
        PromptWidget->SetVisibility(ESlateVisibility::Visible);
    }
}

void UInteractionDetectorComponent::HidePrompt()
{
    if (PromptWidget)
    {
        PromptWidget->SetVisibility(ESlateVisibility::Hidden);
    }
}