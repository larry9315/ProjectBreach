// Copyright Epic Games, Inc. All Rights Reserved.

#include "Framework/ProjectBreachGameMode.h"
#include "Framework/ProjectBreachGameState.h"

AProjectBreachGameMode::AProjectBreachGameMode()
{
	GameStateClass = AProjectBreachGameState::StaticClass();
}
