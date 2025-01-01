// Fill out your copyright notice in the Description page of Project Settings.


#include "VortexGameMode.h"

#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "Vortex/Character/VortexCharacter.h"
#include "Vortex/PlayController/VortexPlayerController.h"
#include "Vortex/PlayerState/VortexPlayerState.h"

void AVortexGameMode::PlayerEliminated(AVortexCharacter* ElimmedCharacter, AVortexPlayerController* VictimController,
	AVortexPlayerController* AttackerController) {
	AVortexPlayerState* AttackerPlayerState = AttackerController ? Cast<AVortexPlayerState>(AttackerController->PlayerState) : nullptr;
	AVortexPlayerState* VictimPlayerState = VictimController ? Cast<AVortexPlayerState>(VictimController->PlayerState) : nullptr;
	if (AttackerPlayerState && AttackerPlayerState != VictimPlayerState) {
		AttackerPlayerState->AddToScore(10.f);
	}
	if (VictimPlayerState) {
		VictimPlayerState->AddToDefeats(1);
	}
	if (ElimmedCharacter) {
		ElimmedCharacter->Elim();
	}
}

void AVortexGameMode::RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController) {
	if (ElimmedCharacter) {
		ElimmedCharacter->Reset();
		ElimmedCharacter->Destroy();
	}
	if (ElimmedController) {
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(),PlayerStarts);
		int32 Selection = FMath::RandRange(0, PlayerStarts.Num() - 1);
		RestartPlayerAtPlayerStart(ElimmedController, PlayerStarts[Selection]);
	}
}
