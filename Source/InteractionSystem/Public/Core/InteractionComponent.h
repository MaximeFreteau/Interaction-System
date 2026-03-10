// Copyright Maxime Freteau. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/InteractionTypes.h"
#include "Interface/InteractableInterface.h"
#include "InteractionComponent.generated.h"

UCLASS(ClassGroup = (Interaction), meta = (BlueprintSpawnableComponent, DisplayName = "Interaction Component"))
class INTERACTIONSYSTEM_API UInteractionComponent : public UActorComponent, public IInteractableInterface
{
    GENERATED_BODY()

public:
    UInteractionComponent();

protected:
    virtual void BeginPlay() override;

public:

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    FInteractionData InteractionData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    bool bEnabledByDefault = true;

    UPROPERTY(BlueprintReadOnly, Category = "Interaction")
    bool bIsEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Visual")
    EInteractionFeedback FeedbackType = EInteractionFeedback::Outline;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Visual")
    FLinearColor HighlightColor = FLinearColor(0.0f, 1.0f, 0.5f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Visual")
    bool bUseCustomHighlightColor = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Visual")
    TArray<TObjectPtr<UPrimitiveComponent>> ComponentsToHighlight;

    UPROPERTY(BlueprintAssignable, Category = "Interaction|Events")
    FOnInteractionStarted OnInteractionStarted;

    UPROPERTY(BlueprintAssignable, Category = "Interaction|Events")
    FOnInteractionCompleted OnInteractionCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Interaction|Events")
    FOnInteractionCancelled OnInteractionCancelled;

    UPROPERTY(BlueprintAssignable, Category = "Interaction|Events")
    FOnInteractionProgress OnInteractionProgress;

    UPROPERTY(BlueprintAssignable, Category = "Interaction|Events")
    FOnFocusGained OnFocusGained;

    UPROPERTY(BlueprintAssignable, Category = "Interaction|Events")
    FOnFocusLost OnFocusLost;

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void SetEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    bool CanInteractWith(AActor* Interactor) const;

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void StartInteraction(AActor* Interactor);

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void CompleteInteraction(AActor* Interactor);

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void CancelInteraction(AActor* Interactor);

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void UpdateInteractionProgress(float Progress);

    UFUNCTION(BlueprintPure, Category = "Interaction")
    AActor* GetCurrentInteractor() const { return CurrentInteractor; }

    UFUNCTION(BlueprintPure, Category = "Interaction")
    bool IsInteracting() const { return bIsInteracting; }

    UFUNCTION(BlueprintPure, Category = "Interaction")
    FInteractionData GetInteractionInfo() const { return InteractionData; }

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void ApplyHighlight();

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void RemoveHighlight();

    virtual bool CanInteract_Implementation(AActor* Interactor) const override;
    virtual FInteractionData GetInteractionData_Implementation() const override;
    virtual void Interact_Implementation(AActor* Interactor) override;
    virtual void OnFocused_Implementation(AActor* Interactor) override;
    virtual void OnUnfocused_Implementation(AActor* Interactor) override;

protected:
    bool CheckRequiredTags(AActor* Interactor) const;

    void AutoDetectHighlightComponents();

    void ApplyOutlineHighlight();

    void RemoveOutlineHighlight();

private:
    UPROPERTY()
    TObjectPtr<AActor> CurrentInteractor;

    bool bIsInteracting = false;

    bool bIsHighlighted = false;

    float CurrentProgress = 0.0f;
};