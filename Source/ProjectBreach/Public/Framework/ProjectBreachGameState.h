// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "ProjectBreachGameState.generated.h"

/**
 * Mission-wide gameplay state for Project Breach.
 *
 * Replicated mission and objective state will be added here only when required
 * by an implemented gameplay feature.
 */
UCLASS()
class PROJECTBREACH_API AProjectBreachGameState : public AGameStateBase
{
	GENERATED_BODY()
};
