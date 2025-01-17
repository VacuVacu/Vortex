// Fill out your copyright notice in the Description page of Project Settings.


#include "VortexGameMode.h"

#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "Vortex/Character/VortexCharacter.h"
#include "Vortex/GameState/VortexGameState.h"
#include "Vortex/PlayController/VortexPlayerController.h"
#include "Vortex/PlayerState/VortexPlayerState.h"

namespace MatchState
{
	const FName Cooldown = FName("Cooldown");
}

AVortexGameMode::AVortexGameMode() {
	bDelayedStart = true;
}

void AVortexGameMode::BeginPlay() {
	Super::BeginPlay();
	LevelStartingTime = GetWorld()->GetTimeSeconds();
}

void AVortexGameMode::Tick(float DeltaSeconds) {
	Super::Tick(DeltaSeconds);
	if (MatchState == MatchState::WaitingToStart) {
		CountDownTime = WarmupTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountDownTime <= 0.f) {
			StartMatch();
		}
	}else if (MatchState == MatchState::InProgress) {
		CountDownTime = WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountDownTime <= 0.f) {
			SetMatchState(MatchState::Cooldown);
		}
	}else if (MatchState == MatchState::Cooldown) {
		CountDownTime = WarmupTime + MatchTime + CooldownTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountDownTime <= 0.f) {
			RestartGame();
		}
	}
}

void AVortexGameMode::OnMatchStateSet() {
	Super::OnMatchStateSet();
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It) {
		AVortexPlayerController* VortexPlayer = Cast<AVortexPlayerController>(*It);
		if (VortexPlayer) {
			VortexPlayer->OnMatchStateSet(MatchState);
		}
	}
}

void AVortexGameMode::PlayerEliminated(AVortexCharacter* ElimmedCharacter, AVortexPlayerController* VictimController,
                                       AVortexPlayerController* AttackerController) {
	AVortexPlayerState* AttackerPlayerState = AttackerController ? Cast<AVortexPlayerState>(AttackerController->PlayerState) : nullptr;
	AVortexPlayerState* VictimPlayerState = VictimController ? Cast<AVortexPlayerState>(VictimController->PlayerState) : nullptr;
	AVortexGameState* VortexGameState = GetGameState<AVortexGameState>();
	if (AttackerPlayerState && AttackerPlayerState != VictimPlayerState && VortexGameState) {
		AttackerPlayerState->AddToScore(10.f);
		VortexGameState->UpdateTopScore(AttackerPlayerState);
	}
	if (VictimPlayerState) {
		VictimPlayerState->AddToDefeats(1);
	}
	if (ElimmedCharacter) {
		ElimmedCharacter->Elim(false);
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

void AVortexGameMode::PlayerLeftGame(AVortexPlayerState* PlayerLeaving) {
	if (PlayerLeaving == nullptr) return;
	AVortexGameState* VortexGameState = GetGameState<AVortexGameState>();
	if (VortexGameState && VortexGameState->TopScoringPlayers.Contains(PlayerLeaving)) {
		VortexGameState->TopScoringPlayers.Remove(PlayerLeaving);
	}
	AVortexCharacter* CharacterLeaving = Cast<AVortexCharacter>(PlayerLeaving->GetPawn());
	if (CharacterLeaving) {
		CharacterLeaving->Elim(true);
	}
}

