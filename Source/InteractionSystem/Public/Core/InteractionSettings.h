// Copyright Maxime Freteau. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "InteractionTypes.h"
#include "InteractionSettings.generated.h"

UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Interaction System"))
class INTERACTIONSYSTEM_API UInteractionSettings : public UDeveloperSettings
{
    GENERATED_BODY()

public:
    UInteractionSettings();

    UPROPERTY(Config, EditAnywhere, Category = "Detection")
    EInteractionDetectionMethod DefaultDetectionMethod = EInteractionDetectionMethod::Raycast;

    UPROPERTY(Config, EditAnywhere, Category = "Detection", meta = (ClampMin = "0.0"))
    float DefaultInteractionDistance = 300.0f;

    UPROPERTY(Config, EditAnywhere, Category = "Detection", meta = (ClampMin = "0.0"))
    float SphereOverlapRadius = 200.0f;

    UPROPERTY(Config, EditAnywhere, Category = "Detection", meta = (ClampMin = "0.1", ClampMax = "60.0"))
    float DetectionFrequency = 10.0f;

    UPROPERTY(Config, EditAnywhere, Category = "Detection")
    TEnumAsByte<ECollisionChannel> InteractionTraceChannel = ECC_Visibility;

    UPROPERTY(Config, EditAnywhere, Category = "Detection")
    bool bShowDebugTraces = false;

    UPROPERTY(Config, EditAnywhere, Category = "Visual Feedback")
    EInteractionFeedback DefaultFeedbackType = EInteractionFeedback::Outline;

    UPROPERTY(Config, EditAnywhere, Category = "Visual Feedback")
    FLinearColor HighlightColor = FLinearColor(0.0f, 1.0f, 0.5f, 1.0f);

    UPROPERTY(Config, EditAnywhere, Category = "Visual Feedback", meta = (ClampMin = "0.0", ClampMax = "10.0"))
    float HighlightIntensity = 2.0f;

    UPROPERTY(Config, EditAnywhere, Category = "Input")
    TSoftObjectPtr<class UInputAction> DefaultInteractInputAction;

    UPROPERTY(Config, EditAnywhere, Category = "Input")
    TSoftObjectPtr<class UInputMappingContext> DefaultInputMappingContext;

    UPROPERTY(Config, EditAnywhere, Category = "Input")
    bool bAllowHoldToInteract = true;

    UPROPERTY(Config, EditAnywhere, Category = "UI")
    TSoftClassPtr<UUserWidget> DefaultPromptWidgetClass;

    UPROPERTY(Config, EditAnywhere, Category = "UI")
    bool bShowDistanceInPrompt = false;

    UPROPERTY(Config, EditAnywhere, Category = "UI")
    bool bShowProgressBar = true;

    UPROPERTY(Config, EditAnywhere, Category = "Integration")
    bool bAutoStartNarrativeDialogue = true;

    UPROPERTY(Config, EditAnywhere, Category = "Integration")
    FGameplayTag InteractableTag;
};