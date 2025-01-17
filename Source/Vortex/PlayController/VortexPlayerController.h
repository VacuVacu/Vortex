// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "VortexPlayerController.generated.h"

class UReturnToMainMenu;
class UInputAction;
class UInputMappingContext;
class AVortexGameMode;
class UCharacterOverlay;
class AVortexHUD;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHighPingDelegate, bool, bPingTooHigh);

UCLASS()
class VORTEX_API AVortexPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDShield(float Shield, float MaxShield);
	void SetHUDScore(float Score);
	void SetHUDDefeats(int32 Defeats);
	void SetHUDWeaponAmmo(int32 Ammo);
	void SetHUDCarriedAmmo(int32 Ammo);
	void SetHUDMatchCountDown(float CountDownTime);
	void SetHUDAnnouncementCountDown(float CountDownTime);
	void SetHUDGrenades(int32 Grenades);
	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaSeconds) override;
	virtual float GetServerTime();
	virtual void ReceivedPlayer() override;
	void OnMatchStateSet(FName State);
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	void HandleMatchHasStarted();
	void HandleCooldown();

	float SingleTripTime = 0.0f;

	FHighPingDelegate HighPingDelegate;

	void BroadcastElim(APlayerState* Attacker, APlayerState* Victim);
	
protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	void SetHUDTime();

	void PollInit();
	/*
	 * Sync time between client and server
	 */
	UFUNCTION(Server, Reliable)
	void ServerRequestServerTime(float TimeOfClientRequest);

	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(float TimeOfClientRequest, float TimeServerReceivedClientRequest);

	float ClientServerDelta = 0.f;

	UPROPERTY(EditAnywhere, Category = Time)
	float TimeSyncFrequency = 5.f;

	float TimeSyncRunningTime = 0.f;

	void CheckTimeSync(float DeltaTime);

	UFUNCTION(Server, Reliable)
	void ServerCheckMatchState();

	UFUNCTION(Client, Reliable)
	void ClientJoinMidgame(FName StateOfMatch, float Warmup, float Match, float Cooldown, float StartingTime);

	void HighPingWarning();
	void StopHighPingWarning();
	void CheckPing(float DeltaTime);

	void ShowReturnToMainMenu();

	UFUNCTION(Client, Reliable)
	void ClientElimAnnouncement(APlayerState* Attacker, APlayerState* Victim);

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* ControllerMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* BackToMainMenuAction;
	
	UPROPERTY()
	AVortexHUD* VortexHUD;

	UPROPERTY(EditAnywhere, Category=HUD)
	TSubclassOf<UUserWidget> ReturnToMainMenuWidget;
	UPROPERTY()
	UReturnToMainMenu* ReturnToMainMenu;
	
	bool bReturnToMainMenuOpen = false;
	
	UPROPERTY()
	AVortexGameMode* VortexGameMode;
	
	float LevelStartingTime = 0.f;
	float MatchTime = 0.f;
	float WarmupTime = 0.f;
	float CooldownTime = 0.f;
	uint32 CountDownInt=0;

	UPROPERTY(ReplicatedUsing=OnRep_MatchState)
	FName MatchState;

	UFUNCTION()
	void OnRep_MatchState();

	UPROPERTY()
	UCharacterOverlay* CharacterOverlay;
	
	bool bInitializeHealth = false;
	float HUDHealth = 0.f;
	float HUDMaxHealth = 0.f;
	bool bInitializeScore = false;
	float HUDScore = 0.f;
	bool bInitializeDefeats = false;
	int32 HUDDefeats = 0;
	bool bInitializeGrenades = false;
	int32 HUDGrenades = 0;
	bool bInitializeShield = false;
	float HUDShield = 0.f;
	float HUDMaxShield = 0.f;
	bool bInitializeCarriedAmmo = false;
	int32 HUDCarriedAmmo = 0;
	bool bInitializeWeaponAmmo = false;
	int32 HUDWeaponAmmo = 0;

	float HighPingRunningTime = 0.f;
	UPROPERTY(EditAnywhere)
	float HighPingDuration = 5.f;
	float PingAnimationRunningTime = 0.f;
	UPROPERTY(EditAnywhere)
	float CheckPingFrequency = 20.f;

	UFUNCTION(Server, Reliable)
	void ServerReportPingStatus(bool bHighPing);
	
	UPROPERTY(EditAnywhere)
	float HighPingThreshold = 50.f;
};
