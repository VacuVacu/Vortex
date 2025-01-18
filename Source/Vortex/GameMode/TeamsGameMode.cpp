// Fill out your copyright notice in the Description page of Project Settings.


#include "TeamsGameMode.h"

#include "Kismet/GameplayStatics.h"
#include "Vortex/GameState/VortexGameState.h"
#include "Vortex/PlayController/VortexPlayerController.h"
#include "Vortex/PlayerState/VortexPlayerState.h"

ATeamsGameMode::ATeamsGameMode() {
	bTeamsMatch = true;
}

void ATeamsGameMode::PostLogin(APlayerController* NewPlayer) {
	Super::PostLogin(NewPlayer);

	AVortexGameState* VGameState = Cast<AVortexGameState>(UGameplayStatics::GetGameState(this));
	if (VGameState) {
		AVortexPlayerState* VTState = NewPlayer->GetPlayerState<AVortexPlayerState>();
		if (VTState && VTState->GetTeam() == ETeam::ET_NoTeam) {
			if (VGameState->BlueTeam.Num() >= VGameState->RedTeam.Num()) {
				VGameState->RedTeam.AddUnique(VTState);
				VTState->SetTeam(ETeam::ET_RedTeam);
			}else {
				VGameState->BlueTeam.AddUnique(VTState);
				VTState->SetTeam(ETeam::ET_BlueTeam);
			}
		}
	}
}

void ATeamsGameMode::Logout(AController* Exiting) {
	Super::Logout(Exiting);
	AVortexGameState* VGameState = Cast<AVortexGameState>(UGameplayStatics::GetGameState(this));
	AVortexPlayerState* VTState = Exiting->GetPlayerState<AVortexPlayerState>();
	if (VGameState && VTState) {
		if (VGameState->RedTeam.Contains(VTState)) {
			VGameState->RedTeam.Remove(VTState);
		}
		if (VGameState->BlueTeam.Contains(VTState)) {
			VGameState->BlueTeam.Remove(VTState);
		}
	}
}

void ATeamsGameMode::HandleMatchHasStarted() {
	Super::HandleMatchHasStarted();

	AVortexGameState* VGameState = Cast<AVortexGameState>(UGameplayStatics::GetGameState(this));
	if (VGameState) {
		for (auto PState: VGameState->PlayerArray) {
			AVortexPlayerState* VTState = Cast<AVortexPlayerState>(PState.Get());
			if (VTState && VTState->GetTeam() == ETeam::ET_NoTeam) {
				if (VGameState->BlueTeam.Num() >= VGameState->RedTeam.Num()) {
					VGameState->RedTeam.AddUnique(VTState);
					VTState->SetTeam(ETeam::ET_RedTeam);
				}else {
					VGameState->BlueTeam.AddUnique(VTState);
					VTState->SetTeam(ETeam::ET_BlueTeam);
				}
			}
		}
	}
}

float ATeamsGameMode::CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage) {
	AVortexPlayerState* AttackerPState = Attacker->GetPlayerState<AVortexPlayerState>();
	AVortexPlayerState* VictimPState = Victim->GetPlayerState<AVortexPlayerState>();
	if (AttackerPState == nullptr || VictimPState == nullptr) return BaseDamage;
	if (AttackerPState == VictimPState) return BaseDamage;
	if (AttackerPState->GetTeam() == VictimPState->GetTeam()) {
		return 0.f;	
	}
	return BaseDamage;
}

void ATeamsGameMode::PlayerEliminated(AVortexCharacter* ElimmedCharacter, AVortexPlayerController* VictimController,
	AVortexPlayerController* AttackerController) {
	Super::PlayerEliminated(ElimmedCharacter, VictimController, AttackerController);

	AVortexGameState* VGameState = Cast<AVortexGameState>(UGameplayStatics::GetGameState(this));
	AVortexPlayerState* AttackerPlayerState = AttackerController ? Cast<AVortexPlayerState>(AttackerController->PlayerState) : nullptr;
	if (VGameState && AttackerPlayerState) {
		if (AttackerPlayerState->GetTeam() == ETeam::ET_BlueTeam) {
			VGameState->BlueTeamScores();
		}
		if (AttackerPlayerState->GetTeam() == ETeam::ET_RedTeam) {
			VGameState->RedTeamScores();
		}
	}
}
