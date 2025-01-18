// Fill out your copyright notice in the Description page of Project Settings.


#include "VortexGameState.h"
#include "Net/UnrealNetwork.h"
#include "Vortex/PlayController/VortexPlayerController.h"
#include "Vortex/PlayerState/VortexPlayerState.h"

void AVortexGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AVortexGameState, TopScoringPlayers);
	DOREPLIFETIME(AVortexGameState, RedTeamScore);
	DOREPLIFETIME(AVortexGameState, BlueTeamScore);
}

void AVortexGameState::UpdateTopScore(AVortexPlayerState* ScoringPlayer) {
	if (TopScoringPlayers.IsEmpty()) {
		TopScoringPlayers.Add(ScoringPlayer);
		TopScore = ScoringPlayer->GetScore();
	}else if (ScoringPlayer->GetScore() == TopScore){
		TopScoringPlayers.AddUnique(ScoringPlayer);
	}else if (ScoringPlayer->GetScore() > TopScore) {
		TopScoringPlayers.Empty();
		TopScoringPlayers.AddUnique(ScoringPlayer);
		TopScore = ScoringPlayer->GetScore();
	}
}

void AVortexGameState::RedTeamScores() {
	++RedTeamScore;
	AVortexPlayerController* PlayerController = Cast<AVortexPlayerController>(GetWorld()->GetFirstPlayerController());
	if (PlayerController) {
		PlayerController->SetHUDRedTeamScore(RedTeamScore);
	}
}

void AVortexGameState::BlueTeamScores() {
	++BlueTeamScore;
	AVortexPlayerController* PlayerController = Cast<AVortexPlayerController>(GetWorld()->GetFirstPlayerController());
	if (PlayerController) {
		PlayerController->SetHUDBlueTeamScore(BlueTeamScore);
	}
}

void AVortexGameState::OnRep_RedTeamScore() {
	AVortexPlayerController* PlayerController = Cast<AVortexPlayerController>(GetWorld()->GetFirstPlayerController());
	if (PlayerController) {
		PlayerController->SetHUDRedTeamScore(RedTeamScore);
	}
	
}

void AVortexGameState::OnRep_BlueTeamScore() {
	AVortexPlayerController* PlayerController = Cast<AVortexPlayerController>(GetWorld()->GetFirstPlayerController());
	if (PlayerController) {
		PlayerController->SetHUDBlueTeamScore(BlueTeamScore);
	}
}
