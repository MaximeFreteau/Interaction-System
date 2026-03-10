// Copyright Maxime Freteau. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "InteractionTypes.generated.h"

UENUM(BlueprintType)
enum class EInteractionType : uint8
{
    Talk         UMETA(DisplayName = "Talk"),
    PickUp       UMETA(DisplayName = "Pick Up"),
    Open         UMETA(DisplayName = "Open"),
    Close        UMETA(DisplayName = "Close"),
    Use          UMETA(DisplayName = "Use"),
    Examine      UMETA(DisplayName = "Examine"),
    Trade        UMETA(DisplayName = "Trade"),
    Sit          UMETA(DisplayName = "Sit"),
    Activate     UMETA(DisplayName = "Activate"),
    Craft        UMETA(DisplayName = "Craft"),
    Custom       UMETA(DisplayName = "Custom")
};

UENUM(BlueprintType)
enum class EInteractionPriority : uint8
{
    VeryLow   UMETA(DisplayName = "Very Low"),
    Low       UMETA(DisplayName = "Low"),
    Normal    UMETA(DisplayName = "Normal"),
    High      UMETA(DisplayName = "High"),
    VeryHigh  UMETA(DisplayName = "Very High"),
    Critical  UMETA(DisplayName = "Critical")
};

UENUM(BlueprintType)
enum class EInteractionDetectionMethod : uint8
{
    Raycast       UMETA(DisplayName = "Raycast"),
    Sphere        UMETA(DisplayName = "Sphere Overlap"),
    Box           UMETA(DisplayName = "Box Overlap"),
    Custom        UMETA(DisplayName = "Custom")
};

UENUM(BlueprintType)
enum class EInteractionFeedback : uint8
{
    None          UMETA(DisplayName = "None"),
    Outline       UMETA(DisplayName = "Outline"),
    Highlight     UMETA(DisplayName = "Highlight"),
    Widget        UMETA(DisplayName = "Widget"),
    Custom        UMETA(DisplayName = "Custom")
};

USTRUCT(BlueprintType)
struct FInteractionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    EInteractionType InteractionType = EInteractionType::Use;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    FText InteractionPrompt;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    FText InteractionVerb;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    FText InteractableName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction", meta = (ClampMin = "0.0"))
    float InteractionDistance = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction", meta = (ClampMin = "0.0"))
    float InteractionDuration = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    bool bCanInteractMultipleTimes = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction", meta = (EditCondition = "bCanInteractMultipleTimes"))
    int32 RemainingUses = -1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    EInteractionPriority Priority = EInteractionPriority::Normal;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    bool bRequiresLineOfSight = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    FGameplayTagContainer RequiredTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    FGameplayTagContainer BlockedByTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    TObjectPtr<UTexture2D> InteractionIcon;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractionStarted, AActor*, Interactor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractionCompleted, AActor*, Interactor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractionCancelled, AActor*, Interactor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInteractionProgress, AActor*, Interactor, float, Progress);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFocusGained, AActor*, FocusedActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFocusLost, AActor*, LostActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInteractableFound, AActor*, InteractableActor, const FInteractionData&, InteractionData);