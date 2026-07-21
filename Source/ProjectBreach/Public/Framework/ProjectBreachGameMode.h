// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ProjectBreachGameMode.generated.h"

/**
 * Authoritative gameplay rules and framework-class configuration for
 * Project Breach.
 */
UCLASS(abstract)
class PROJECTBREACH_API AProjectBreachGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	
	/** Constructor */
	AProjectBreachGameMode();
};



