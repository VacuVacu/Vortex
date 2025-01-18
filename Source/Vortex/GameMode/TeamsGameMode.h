// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "VortexGameMode.h"
#include "TeamsGameMode.generated.h"

/**
 * 
 */
UCLASS()
class VORTEX_API ATeamsGameMode : public AVortexGameMode
{
	GENERATED_BODY()
public:
	ATeamsGameMode();
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
	virtual float CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage) override;
	virtual void PlayerEliminated(AVortexCharacter* ElimmedCharacter, AVortexPlayerController* VictimController, AVortexPlayerController* AttackerController) override;
protected:
	virtual void HandleMatchHasStarted() override;
	
};
