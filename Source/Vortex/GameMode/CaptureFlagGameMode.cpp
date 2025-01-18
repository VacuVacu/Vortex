// Fill out your copyright notice in the Description page of Project Settings.


#include "CaptureFlagGameMode.h"

#include "Kismet/GameplayStatics.h"
#include "Vortex/CaptureFlag/FlagZone.h"
#include "Vortex/GameState/VortexGameState.h"
#include "Vortex/Weapon/Flag.h"

void ACaptureFlagGameMode::PlayerEliminated(AVortexCharacter* ElimmedCharacter,
                                            AVortexPlayerController* VictimController, AVortexPlayerController* AttackerController) {
	AVortexGameMode::PlayerEliminated(ElimmedCharacter, VictimController, AttackerController);
	
}

void ACaptureFlagGameMode::FlagCaptured(AFlag* Flag, AFlagZone* FlagZone) {
	bool bValidCapture = Flag->GetTeam() != FlagZone->Team;
	AVortexGameState* VortexGameState = Cast<AVortexGameState>(GameState);
	if (VortexGameState) {
		if (FlagZone->Team == ETeam::ET_BlueTeam) {
			VortexGameState->BlueTeamScores();	
		}
		if (FlagZone->Team == ETeam::ET_RedTeam) {
			VortexGameState->RedTeamScores();
		}
	}
}
