// Copyright Maxime Freteau. All Rights Reserved.

#include "Core/InteractionComponent.h"
#include "Core/InteractionSettings.h"
#include "GameplayTagsManager.h"
#include "Components/PrimitiveComponent.h"

UInteractionComponent::UInteractionComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UInteractionComponent::BeginPlay()
{
    Super::BeginPlay();

    bIsEnabled = bEnabledByDefault;

    if (ComponentsToHighlight.Num() == 0)
    {
        AutoDetectHighlightComponents();
    }

    if (InteractionData.InteractionPrompt.IsEmpty())
    {
        FString VerbStr = TEXT("Use");
        
        switch (InteractionData.InteractionType)
        {
            case EInteractionType::Talk: VerbStr = TEXT("Talk to"); break;
            case EInteractionType::PickUp: VerbStr = TEXT("Pick up"); break;
            case EInteractionType::Open: VerbStr = TEXT("Open"); break;
            case EInteractionType::Close: VerbStr = TEXT("Close"); break;
            case EInteractionType::Use: VerbStr = TEXT("Use"); break;
            case EInteractionType::Examine: VerbStr = TEXT("Examine"); break;
            case EInteractionType::Trade: VerbStr = TEXT("Trade with"); break;
            case EInteractionType::Sit: VerbStr = TEXT("Sit on"); break;
            case EInteractionType::Activate: VerbStr = TEXT("Activate"); break;
            case EInteractionType::Craft: VerbStr = TEXT("Craft at"); break;
        }

        const UInteractionSettings* Settings = GetDefault<UInteractionSettings>();
        FString InputName = TEXT("E");
        
        InteractionData.InteractionPrompt = FText::FromString(
            FString::Printf(TEXT("Press %s to %s"), *InputName, *VerbStr)
        );
    }
}

void UInteractionComponent::SetEnabled(bool bEnabled)
{
    bIsEnabled = bEnabled;

    if (!bIsEnabled && bIsHighlighted)
    {
        RemoveHighlight();
    }
}

bool UInteractionComponent::CanInteractWith(AActor* Interactor) const
{
    if (!bIsEnabled)
    {
        return false;
    }

    if (InteractionData.bCanInteractMultipleTimes && InteractionData.RemainingUses == 0)
    {
        return false;
    }

    if (!CheckRequiredTags(Interactor))
    {
        return false;
    }

    return true;
}

void UInteractionComponent::StartInteraction(AActor* Interactor)
{
    if (!CanInteractWith(Interactor))
    {
        return;
    }

    CurrentInteractor = Interactor;
    bIsInteracting = true;
    CurrentProgress = 0.0f;

    OnInteractionStarted.Broadcast(Interactor);

    if (InteractionData.InteractionDuration <= 0.0f)
    {
        CompleteInteraction(Interactor);
    }
}

void UInteractionComponent::CompleteInteraction(AActor* Interactor)
{
    if (!bIsInteracting)
    {
        return;
    }

    if (InteractionData.RemainingUses > 0)
    {
        InteractionData.RemainingUses--;
    }

    OnInteractionCompleted.Broadcast(Interactor);

    bIsInteracting = false;
    CurrentInteractor = nullptr;
    CurrentProgress = 0.0f;
}

void UInteractionComponent::CancelInteraction(AActor* Interactor)
{
    if (!bIsInteracting)
    {
        return;
    }

    OnInteractionCancelled.Broadcast(Interactor);

    bIsInteracting = false;
    CurrentInteractor = nullptr;
    CurrentProgress = 0.0f;
}

void UInteractionComponent::UpdateInteractionProgress(float Progress)
{
    if (!bIsInteracting)
    {
        return;
    }

    CurrentProgress = FMath::Clamp(Progress, 0.0f, 1.0f);
    OnInteractionProgress.Broadcast(CurrentInteractor, CurrentProgress);

    if (CurrentProgress >= 1.0f)
    {
        CompleteInteraction(CurrentInteractor);
    }
}

void UInteractionComponent::ApplyHighlight()
{
    if (bIsHighlighted)
    {
        return;
    }

    switch (FeedbackType)
    {
        case EInteractionFeedback::Outline:
            ApplyOutlineHighlight();
            break;

        case EInteractionFeedback::Highlight:
            break;

        case EInteractionFeedback::Widget:
            break;

        default:
            break;
    }

    bIsHighlighted = true;
}

void UInteractionComponent::RemoveHighlight()
{
    if (!bIsHighlighted)
    {
        return;
    }

    switch (FeedbackType)
    {
        case EInteractionFeedback::Outline:
            RemoveOutlineHighlight();
            break;

        case EInteractionFeedback::Highlight:
            break;

        case EInteractionFeedback::Widget:
            break;

        default:
            break;
    }

    bIsHighlighted = false;
}


bool UInteractionComponent::CanInteract_Implementation(AActor* Interactor) const
{
    return CanInteractWith(Interactor);
}

FInteractionData UInteractionComponent::GetInteractionData_Implementation() const
{
    return InteractionData;
}

void UInteractionComponent::Interact_Implementation(AActor* Interactor)
{
    StartInteraction(Interactor);
}

void UInteractionComponent::OnFocused_Implementation(AActor* Interactor)
{
    ApplyHighlight();
    OnFocusGained.Broadcast(Interactor);
}

void UInteractionComponent::OnUnfocused_Implementation(AActor* Interactor)
{
    RemoveHighlight();
    OnFocusLost.Broadcast(Interactor);
}


bool UInteractionComponent::CheckRequiredTags(AActor* Interactor) const
{
    if (InteractionData.RequiredTags.Num() == 0 && InteractionData.BlockedByTags.Num() == 0)
    {
        return true;
    }
    
    return true;
}

void UInteractionComponent::AutoDetectHighlightComponents()
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    TArray<UPrimitiveComponent*> PrimitiveComponents;
    Owner->GetComponents<UPrimitiveComponent>(PrimitiveComponents);

    for (UPrimitiveComponent* Comp : PrimitiveComponents)
    {
        if (Comp && Comp->IsVisible())
        {
            ComponentsToHighlight.Add(Comp);
        }
    }
}

void UInteractionComponent::ApplyOutlineHighlight()
{
    const UInteractionSettings* Settings = GetDefault<UInteractionSettings>();
    FLinearColor ColorToUse = bUseCustomHighlightColor ? HighlightColor : (Settings ? Settings->HighlightColor : FLinearColor::Green);

    for (UPrimitiveComponent* Comp : ComponentsToHighlight)
    {
        if (Comp)
        {
            Comp->SetRenderCustomDepth(true);
            Comp->SetCustomDepthStencilValue(1);
        }
    }
}

void UInteractionComponent::RemoveOutlineHighlight()
{
    for (UPrimitiveComponent* Comp : ComponentsToHighlight)
    {
        if (Comp)
        {
            Comp->SetRenderCustomDepth(false);
            Comp->SetCustomDepthStencilValue(0);
        }
    }
}