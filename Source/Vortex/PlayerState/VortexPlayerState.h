// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "VortexPlayerState.generated.h"


class AVortexPlayerController;
class AVortexCharacter;

UCLASS()
class VORTEX_API AVortexPlayerState : public APlayerState
{
	GENERATED_BODY()
public:
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;
	virtual void OnRep_Score() override;
	UFUNCTION()
	virtual void OnRep_Defeats();
	void AddToScore(float ScoreAmount);
	void AddToDefeats(int32 DefeatsAmount);
	
private:
	UPROPERTY()
	AVortexCharacter* Character = nullptr;
	UPROPERTY()
	AVortexPlayerController* Controller = nullptr;

	UPROPERTY(ReplicatedUsing=OnRep_Defeats)
	int32 Defeats;
};