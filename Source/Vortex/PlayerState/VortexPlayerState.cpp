// Fill out your copyright notice in the Description page of Project Settings.


#include "VortexPlayerState.h"

#include "Net/UnrealNetwork.h"
#include "Vortex/Character/VortexCharacter.h"
#include "Vortex/PlayController/VortexPlayerController.h"

void AVortexPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AVortexPlayerState, Defeats);
	DOREPLIFETIME(AVortexPlayerState, Team);
}

void AVortexPlayerState::AddToScore(float ScoreAmount) {
	SetScore(GetScore() + ScoreAmount);
	Character = Character == nullptr? Cast<AVortexCharacter>(GetPawn()) : Character;
	if (Character) {
		Controller = Controller == nullptr? Cast<AVortexPlayerController>(Character->Controller) : Controller;
		if (Controller) {
			Controller->SetHUDScore(GetScore());
		}
	}
}

void AVortexPlayerState::OnRep_Score() {
	Super::OnRep_Score();
	Character = Character == nullptr? Cast<AVortexCharacter>(GetPawn()) : Character;
	if (Character) {
		Controller = Controller == nullptr? Cast<AVortexPlayerController>(Character->Controller) : Controller;
		if (Controller) {
			Controller->SetHUDScore(GetScore());
		}
	}
}

void AVortexPlayerState::AddToDefeats(int32 DefeatsAmount) {
	Defeats += DefeatsAmount;
	Character = Character == nullptr? Cast<AVortexCharacter>(GetPawn()) : Character;
	if (Character) {
		Controller = Controller == nullptr? Cast<AVortexPlayerController>(Character->Controller) : Controller;
		if (Controller) {
			Controller->SetHUDDefeats(Defeats);
		}
	}
}

void AVortexPlayerState::OnRep_Defeats() {
	Character = Character == nullptr? Cast<AVortexCharacter>(GetPawn()) : Character;
	if (Character) {
		Controller = Controller == nullptr? Cast<AVortexPlayerController>(Character->Controller) : Controller;
		if (Controller) {
			Controller->SetHUDDefeats(Defeats);
		}
	}
}

void AVortexPlayerState::SetTeam(ETeam TeamToSet) {
	Team = TeamToSet;
	AVortexCharacter* VCharacter = Cast<AVortexCharacter>(GetPawn());
	if (VCharacter) {
		VCharacter->SetTeamColor(Team);
	}
}

void AVortexPlayerState::OnRep_Team() {
	AVortexCharacter* VCharacter = Cast<AVortexCharacter>(GetPawn());
	if (VCharacter) {
		VCharacter->SetTeamColor(Team);
	}
}



