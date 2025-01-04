// Fill out your copyright notice in the Description page of Project Settings.


#include "VortexGameState.h"
#include "Net/UnrealNetwork.h"
#include "Vortex/PlayerState/VortexPlayerState.h"

void AVortexGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AVortexGameState, TopScoringPlayers);
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
