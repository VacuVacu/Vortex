// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TeamsGameMode.h"
#include "CaptureFlagGameMode.generated.h"


class AFlagZone;
class AFlag;

UCLASS()
class VORTEX_API ACaptureFlagGameMode : public ATeamsGameMode
{
	GENERATED_BODY()
public:
	virtual void PlayerEliminated(AVortexCharacter* ElimmedCharacter, AVortexPlayerController* VictimController, AVortexPlayerController* AttackerController) override;
	void FlagCaptured(AFlag* Flag, AFlagZone* FlagZone);
};
