// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "VortexGameMode.generated.h"


class AVortexPlayerController;
class AVortexCharacter;

UCLASS()
class VORTEX_API AVortexGameMode : public AGameMode
{
	GENERATED_BODY()
public:
	virtual void PlayerEliminated(AVortexCharacter* ElimmedCharacter, AVortexPlayerController* VictimController, AVortexPlayerController* AttackerController);
	virtual void RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController);

};
