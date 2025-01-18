// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "VortexGameMode.generated.h"

namespace MatchState
{
	extern VORTEX_API const FName Cooldown;  
}


class AVortexPlayerController;
class AVortexCharacter;
class AVortexPlayerState;

UCLASS()
class VORTEX_API AVortexGameMode : public AGameMode
{
	GENERATED_BODY()
public:
	AVortexGameMode();
	virtual void Tick(float DeltaSeconds) override;
	virtual void PlayerEliminated(AVortexCharacter* ElimmedCharacter, AVortexPlayerController* VictimController, AVortexPlayerController* AttackerController);
	virtual void RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController);
	void PlayerLeftGame(AVortexPlayerState* PlayerLeaving);

	virtual float CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage);
		
	UPROPERTY(EditDefaultsOnly)
	float WarmupTime = 10.f;

	UPROPERTY(EditDefaultsOnly)
	float MatchTime = 120.f;

	UPROPERTY(EditDefaultsOnly)
	float CooldownTime = 10.f;
	
	float LevelStartingTime = 0.f;

	bool bTeamsMatch = false;

protected:
	virtual void BeginPlay() override;
	virtual void OnMatchStateSet() override;

private:
	float CountDownTime = 0.f;

public:
	FORCEINLINE float GetCountDownTime() const { return CountDownTime; }	
};
