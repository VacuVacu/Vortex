// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "VortexGameState.generated.h"


class AVortexPlayerState;

UCLASS()
class VORTEX_API AVortexGameState : public AGameState
{
	GENERATED_BODY()
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void UpdateTopScore(AVortexPlayerState* ScoringPlayer);
	
	UPROPERTY(Replicated)
	TArray<AVortexPlayerState*> TopScoringPlayers;

	/*
	 *  Teams
	 */
	void RedTeamScores();
	void BlueTeamScores();
	
	TArray<AVortexPlayerState*> RedTeam;
	TArray<AVortexPlayerState*> BlueTeam;

	UPROPERTY(ReplicatedUsing=OnRep_RedTeamScore)
	float RedTeamScore = 0.0f;
	UFUNCTION()
	void OnRep_RedTeamScore();

	UPROPERTY(ReplicatedUsing=OnRep_BlueTeamScore)
	float BlueTeamScore = 0.0f;
	UFUNCTION()
	void OnRep_BlueTeamScore();
	
private:
	float TopScore = 0.f;	
};


