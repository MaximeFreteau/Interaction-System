// Copyright Maxime Freteau. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/InteractionTypes.h"
#include "InputActionValue.h"
#include "InteractionDetectorComponent.generated.h"

class UInteractionComponent;
class IInteractableInterface;
class UInputAction;
class UInputMappingContext;

UCLASS(ClassGroup = (Interaction), meta = (BlueprintSpawnableComponent, DisplayName = "Interaction Detector"))
class INTERACTIONSYSTEM_API UInteractionDetectorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UInteractionDetectorComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
    EInteractionDetectionMethod DetectionMethod = EInteractionDetectionMethod::Raycast;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection", meta = (ClampMin = "0.0"))
    float DetectionDistance = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection", meta = (ClampMin = "0.0", EditCondition = "DetectionMethod == EInteractionDetectionMethod::Sphere", EditConditionHides))
    float SphereRadius = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection", meta = (EditCondition = "DetectionMethod == EInteractionDetectionMethod::Box", EditConditionHides))
    FVector BoxExtent = FVector(100.0f, 100.0f, 100.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
    TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection", meta = (ClampMin = "0.1", ClampMax = "60.0"))
    float DetectionFrequency = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection|Debug")
    bool bShowDebug = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection|Debug", meta = (EditCondition = "bShowDebug"))
    float DebugDuration = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
    TObjectPtr<UInputAction> InteractInputAction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
    TObjectPtr<UInputMappingContext> InputMappingContext;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
    int32 MappingPriority = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
    bool bAllowHoldToInteract = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
    bool bAutoSetupInput = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSubclassOf<UUserWidget> PromptWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    bool bAutoCreateWidget = true;

    UPROPERTY(BlueprintReadOnly, Category = "UI")
    TObjectPtr<UUserWidget> PromptWidget;

    UPROPERTY(BlueprintAssignable, Category = "Interaction|Events")
    FOnInteractableFound OnInteractableFound;

    UPROPERTY(BlueprintAssignable, Category = "Interaction|Events")
    FOnFocusLost OnFocusLost;

    UPROPERTY(BlueprintAssignable, Category = "Interaction|Events")
    FOnFocusGained OnFocusGained;

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void DetectInteractables();

    UFUNCTION(BlueprintPure, Category = "Interaction")
    AActor* GetFocusedActor() const { return FocusedActor; }

    UFUNCTION(BlueprintPure, Category = "Interaction")
    UInteractionComponent* GetFocusedInteractionComponent() const { return FocusedInteractionComponent; }

    UFUNCTION(BlueprintPure, Category = "Interaction")
    FInteractionData GetFocusedInteractionData() const;

    UFUNCTION(BlueprintPure, Category = "Interaction")
    bool IsFocusingInteractable() const { return FocusedActor != nullptr; }

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void InteractWithFocused();

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void StartHoldInteraction();

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void StopHoldInteraction();

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void SetupInputBinding();

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void CreatePromptWidget();

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void ShowPrompt();

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void HidePrompt();

protected:
    AActor* DetectViaRaycast();

    AActor* DetectViaSphereOverlap();

    AActor* DetectViaBoxOverlap();

    AActor* FilterByPriority(const TArray<AActor*>& Actors);

    bool IsActorInteractable(AActor* Actor) const;

    UInteractionComponent* GetInteractionComponent(AActor* Actor) const;

    void SetFocusedActor(AActor* NewFocusedActor);

    void UpdateHoldInteractionProgress(float DeltaTime);

    void OnInteractStarted(const FInputActionValue& Value);

    void OnInteractCompleted(const FInputActionValue& Value);

private:
    FTimerHandle DetectionTimerHandle;

    UPROPERTY()
    TObjectPtr<AActor> FocusedActor;

    UPROPERTY()
    TObjectPtr<UInteractionComponent> FocusedInteractionComponent;

    bool bIsHoldingInteraction = false;

    float HoldProgress = 0.0f;

    float CurrentInteractionDuration = 0.0f;
};