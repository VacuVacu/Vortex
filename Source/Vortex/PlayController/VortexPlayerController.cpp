// Fill out your copyright notice in the Description page of Project Settings.


#include "VortexPlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "GameFramework/GameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Vortex/HUD/VortexHUD.h"
#include "Vortex/HUD/CharacterOverlay.h"
#include "Vortex/Character/VortexCharacter.h"
#include "Vortex/GameMode/VortexGameMode.h"
#include "Vortex/GameState/VortexGameState.h"
#include "Vortex/HUD/Announcement.h"
#include "Vortex/HUD/ReturnToMainMenu.h"
#include "Vortex/PlayerState/VortexPlayerState.h"
#include "Vortex/VortexComponents/CombatComponent.h"


void AVortexPlayerController::BeginPlay() {
	Super::BeginPlay();
	VortexHUD = Cast<AVortexHUD>(GetHUD());
	ServerCheckMatchState();
}

void AVortexPlayerController::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AVortexPlayerController, MatchState);
}

void AVortexPlayerController::ServerCheckMatchState_Implementation() {
	AVortexGameMode* GameMode = Cast<AVortexGameMode>(UGameplayStatics::GetGameMode(this));
	if (GameMode) {
		WarmupTime = GameMode->WarmupTime;
		MatchTime = GameMode->MatchTime;
		LevelStartingTime = GameMode->LevelStartingTime;
		MatchState = GameMode->GetMatchState();
		CooldownTime = GameMode->CooldownTime;
		ClientJoinMidgame(MatchState, WarmupTime, MatchTime, CooldownTime, LevelStartingTime);
	}
}

void AVortexPlayerController::ClientJoinMidgame_Implementation(FName StateOfMatch, float Warmup, float Match, float Cooldown, float StartingTime) {
	WarmupTime = Warmup;
	MatchTime = Match;
	LevelStartingTime = StartingTime;
	MatchState = StateOfMatch;
	CooldownTime = Cooldown;
	OnMatchStateSet(MatchState);
	if (VortexHUD && MatchState == MatchState::WaitingToStart) {
		VortexHUD->AddAnnouncement();
	}
}


void AVortexPlayerController::OnPossess(APawn* InPawn) {
	Super::OnPossess(InPawn);
	AVortexCharacter* VortexCharacter = Cast<AVortexCharacter>(InPawn);
	if (VortexCharacter) {
		SetHUDHealth(VortexCharacter->GetHealth(), VortexCharacter->GetMaxHealth());
	}
}

void AVortexPlayerController::Tick(float DeltaSeconds) {
	Super::Tick(DeltaSeconds);
	SetHUDTime();
	CheckTimeSync(DeltaSeconds);
	PollInit();
	CheckPing(DeltaSeconds);
}

void AVortexPlayerController::CheckTimeSync(float DeltaTime) {
	TimeSyncRunningTime += DeltaTime;
	if (IsLocalController() && TimeSyncRunningTime > TimeSyncFrequency) {
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		TimeSyncRunningTime = 0.f;
	}
}

void AVortexPlayerController::HighPingWarning() {
	VortexHUD = VortexHUD == nullptr ? Cast<AVortexHUD>(GetHUD()) : VortexHUD;
	bool bHUDValid = VortexHUD && VortexHUD->CharacterOverlay &&
		VortexHUD->CharacterOverlay->HighPingImage && VortexHUD->CharacterOverlay->HighPingAnimation;
	if (bHUDValid) {
		VortexHUD->CharacterOverlay->HighPingImage->SetOpacity(1.f);
		VortexHUD->CharacterOverlay->PlayAnimation(VortexHUD->CharacterOverlay->HighPingAnimation, 0.f, 5);
	}
}

void AVortexPlayerController::StopHighPingWarning() {
	VortexHUD = VortexHUD == nullptr ? Cast<AVortexHUD>(GetHUD()) : VortexHUD;
	bool bHUDValid = VortexHUD && VortexHUD->CharacterOverlay &&
		VortexHUD->CharacterOverlay->HighPingImage && VortexHUD->CharacterOverlay->HighPingAnimation;
	if (bHUDValid) {
		VortexHUD->CharacterOverlay->HighPingImage->SetOpacity(0.f);
		if (VortexHUD->CharacterOverlay->IsAnimationPlaying(VortexHUD->CharacterOverlay->HighPingAnimation)) {
			VortexHUD->CharacterOverlay->StopAnimation(VortexHUD->CharacterOverlay->HighPingAnimation);
		}
	}
}

void AVortexPlayerController::CheckPing(float DeltaTime) {
	HighPingRunningTime += DeltaTime;
	if (HighPingRunningTime > CheckPingFrequency) {
		PlayerState = PlayerState == nullptr ? GetPlayerState<AVortexPlayerState>() : PlayerState;
		if (PlayerState) {
			if (PlayerState->GetCompressedPing() * 4 > HighPingThreshold) {
				HighPingWarning();
				PingAnimationRunningTime = 0.f;
				ServerReportPingStatus(true);
			}else {
				ServerReportPingStatus(false);
			}
		}
		HighPingRunningTime = 0.f;
	}
	bool bHighPingAnimationPlaying = VortexHUD && VortexHUD->CharacterOverlay && VortexHUD->CharacterOverlay->HighPingAnimation &&
		VortexHUD->CharacterOverlay->IsAnimationPlaying(VortexHUD->CharacterOverlay->HighPingAnimation);
	if (bHighPingAnimationPlaying) {
		PingAnimationRunningTime += DeltaTime;
		if (PingAnimationRunningTime > HighPingDuration) {
			StopHighPingWarning();
		}
	}
}

void AVortexPlayerController::ShowReturnToMainMenu() {
	if (ReturnToMainMenuWidget == nullptr) return;
	if (ReturnToMainMenu == nullptr) {
		ReturnToMainMenu = CreateWidget<UReturnToMainMenu>(this, ReturnToMainMenuWidget);
	}
	if (ReturnToMainMenu) {
		bReturnToMainMenuOpen = !bReturnToMainMenuOpen;
		if (bReturnToMainMenuOpen) {
			ReturnToMainMenu->MenuSetup();
		}else {
			ReturnToMainMenu->MenuTearDown();
		}
	}
}

void AVortexPlayerController::BroadcastElim(APlayerState* Attacker, APlayerState* Victim) {
	ClientElimAnnouncement(Attacker, Victim);
}

void AVortexPlayerController::ClientElimAnnouncement_Implementation(APlayerState* Attacker, APlayerState* Victim) {
	APlayerState* Self = GetPlayerState<APlayerState>();
	if (Attacker && Victim && Self) {
		VortexHUD = VortexHUD == nullptr ? Cast<AVortexHUD>(GetHUD()) : VortexHUD;
		if (VortexHUD) {
			if (Attacker == Self && Victim != Self) {
				VortexHUD->AddElimAnnouncement("You", Victim->GetPlayerName());
				return;
			}
			if (Victim == Self && Attacker != Self) {
				VortexHUD->AddElimAnnouncement(Attacker->GetPlayerName(), "You");
				return;
			}
			if (Attacker == Victim && Attacker == Self) {
				VortexHUD->AddElimAnnouncement("You", "Yourself");
				return;
			}
			if (Attacker == Victim && Attacker != Self) {
				VortexHUD->AddElimAnnouncement(Attacker->GetPlayerName(), "Themselves");
				return;
			}
			VortexHUD->AddElimAnnouncement(Attacker->GetPlayerName(), Victim->GetPlayerName());
		}
	}
}

void AVortexPlayerController::ServerReportPingStatus_Implementation(bool bHighPing) {
	HighPingDelegate.Broadcast(bHighPing);
}

void AVortexPlayerController::SetupInputComponent() {
	Super::SetupInputComponent();
	if (InputComponent == nullptr) return;
	InputComponent->BindAction("Quit", IE_Pressed, this, &AVortexPlayerController::ShowReturnToMainMenu);
}

void AVortexPlayerController::SetHUDHealth(float Health, float MaxHealth) {
	VortexHUD = VortexHUD == nullptr ? Cast<AVortexHUD>(GetHUD()) : VortexHUD;
	bool bHUDValid = VortexHUD && VortexHUD->CharacterOverlay &&
		VortexHUD->CharacterOverlay->HealthBar && VortexHUD->CharacterOverlay->HealthText;
	if (bHUDValid) {
		const float HealthPercent = Health / MaxHealth;
		VortexHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);
		FString HealthText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
		VortexHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
	}
	else {
		bInitializeHealth = true;
		HUDHealth = Health;
		HUDMaxHealth = MaxHealth;
	}
}

void AVortexPlayerController::SetHUDShield(float Shield, float MaxShield) {
	VortexHUD = VortexHUD == nullptr ? Cast<AVortexHUD>(GetHUD()) : VortexHUD;
	bool bHUDValid = VortexHUD && VortexHUD->CharacterOverlay &&
		VortexHUD->CharacterOverlay->ShieldBar && VortexHUD->CharacterOverlay->ShieldText;
	if (bHUDValid) {
		const float HealthPercent = Shield / MaxShield;
		VortexHUD->CharacterOverlay->ShieldBar->SetPercent(HealthPercent);
		FString ShieldText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Shield), FMath::CeilToInt(MaxShield));
		VortexHUD->CharacterOverlay->ShieldText->SetText(FText::FromString(ShieldText));
	}
	else {
		bInitializeShield = true;
		HUDShield = Shield;
		HUDMaxShield = MaxShield;
	}
}

void AVortexPlayerController::SetHUDScore(float Score) {
	VortexHUD = VortexHUD == nullptr ? Cast<AVortexHUD>(GetHUD()) : VortexHUD;
	bool bHUDValid = VortexHUD && VortexHUD->CharacterOverlay &&
		VortexHUD->CharacterOverlay->ScoreAmount;
	if (bHUDValid) {
		FString ScoreText = FString::Printf(TEXT(" %d"), FMath::FloorToInt(Score));
		VortexHUD->CharacterOverlay->ScoreAmount->SetText(FText::FromString(ScoreText));
	}
	else {
		bInitializeScore = true;
		HUDScore = Score;
	}
}

void AVortexPlayerController::SetHUDDefeats(int32 Defeats) {
	VortexHUD = VortexHUD == nullptr ? Cast<AVortexHUD>(GetHUD()) : VortexHUD;
	bool bHUDValid = VortexHUD && VortexHUD->CharacterOverlay &&
		VortexHUD->CharacterOverlay->DefeatsAmount;
	if (bHUDValid) {
		FString DefeatsText = FString::Printf(TEXT(" %d"), Defeats);
		VortexHUD->CharacterOverlay->DefeatsAmount->SetText(FText::FromString(DefeatsText));
	}
	else {
		bInitializeDefeats = true;
		HUDDefeats = Defeats;
	}
}

void AVortexPlayerController::SetHUDWeaponAmmo(int32 Ammo) {
	VortexHUD = VortexHUD == nullptr ? Cast<AVortexHUD>(GetHUD()) : VortexHUD;
	bool bHUDValid = VortexHUD && VortexHUD->CharacterOverlay &&
		VortexHUD->CharacterOverlay->WeaponAmmoAmount;
	if (bHUDValid) {
		FString AmmoText = FString::Printf(TEXT(" %d"), Ammo);
		VortexHUD->CharacterOverlay->WeaponAmmoAmount->SetText(FText::FromString(AmmoText));
	}else {
		bInitializeWeaponAmmo = true;
		HUDWeaponAmmo = Ammo;
	}
}

void AVortexPlayerController::SetHUDCarriedAmmo(int32 Ammo) {
	VortexHUD = VortexHUD == nullptr ? Cast<AVortexHUD>(GetHUD()) : VortexHUD;
	bool bHUDValid = VortexHUD && VortexHUD->CharacterOverlay &&
		VortexHUD->CharacterOverlay->CarriedAmmoAmount;
	if (bHUDValid) {
		FString AmmoText = FString::Printf(TEXT(" %d"), Ammo);
		VortexHUD->CharacterOverlay->CarriedAmmoAmount->SetText(FText::FromString(AmmoText));
	}else {
		bInitializeCarriedAmmo = true;
		HUDCarriedAmmo = Ammo;
	}
}

void AVortexPlayerController::SetHUDMatchCountDown(float CountDownTime) {
	VortexHUD = VortexHUD == nullptr ? Cast<AVortexHUD>(GetHUD()) : VortexHUD;
	bool bHUDValid = VortexHUD && VortexHUD->CharacterOverlay &&
		VortexHUD->CharacterOverlay->MatchCountDownText;
	if (bHUDValid) {
		if (CountDownTime < 0.f) {
			VortexHUD->CharacterOverlay->MatchCountDownText->SetText(FText());
			return;
		}
		int32 Minutes = FMath::FloorToInt(CountDownTime / 60.f);
		int32 Seconds = CountDownTime - Minutes * 60;
		FString CountDownText = FString::Printf(TEXT(" %02d:%02d"), Minutes, Seconds);
		VortexHUD->CharacterOverlay->MatchCountDownText->SetText(FText::FromString(CountDownText));
	}
}

void AVortexPlayerController::SetHUDAnnouncementCountDown(float CountDownTime) {
	VortexHUD = VortexHUD == nullptr ? Cast<AVortexHUD>(GetHUD()) : VortexHUD;
	bool bHUDValid = VortexHUD && VortexHUD->Announcement &&
		VortexHUD->Announcement->WarmupTime;
	if (bHUDValid) {
		if (CountDownTime < 0.f) {
			VortexHUD->Announcement->WarmupTime->SetText(FText());
			return;
		}
		int32 Minutes = FMath::FloorToInt(CountDownTime / 60.f);
		int32 Seconds = CountDownTime - Minutes * 60;
		FString CountDownText = FString::Printf(TEXT(" %02d:%02d"), Minutes, Seconds);
		VortexHUD->Announcement->WarmupTime->SetText(FText::FromString(CountDownText));
	}
}

void AVortexPlayerController::SetHUDGrenades(int32 Grenades) {
	VortexHUD = VortexHUD == nullptr ? Cast<AVortexHUD>(GetHUD()) : VortexHUD;
	bool bHUDValid = VortexHUD && VortexHUD->CharacterOverlay &&
		VortexHUD->CharacterOverlay->GrenadesText;
	if (bHUDValid) {
		FString GrenadeText = FString::Printf(TEXT(" %d"), Grenades);
		VortexHUD->CharacterOverlay->GrenadesText->SetText(FText::FromString(GrenadeText));
	}else {
		bInitializeGrenades = true;
		HUDGrenades = Grenades;
	}
}

void AVortexPlayerController::SetHUDTime() {
	float TimeLeft = 0.f;
	if (MatchState == MatchState::WaitingToStart)
		TimeLeft = WarmupTime - GetServerTime() + LevelStartingTime;
	else if (MatchState == MatchState::InProgress)
		TimeLeft = WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;
	else if (MatchState == MatchState::Cooldown)
		TimeLeft =  WarmupTime + MatchTime + CooldownTime - GetServerTime() + LevelStartingTime;
	uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);
	if (HasAuthority()) {
		VortexGameMode = VortexGameMode == nullptr ? Cast<AVortexGameMode>(UGameplayStatics::GetGameMode(this)) : VortexGameMode;
		if (VortexGameMode) {
			SecondsLeft = FMath::CeilToInt(VortexGameMode->GetCountDownTime() + LevelStartingTime);
		}
	}
	if (CountDownInt != SecondsLeft) {
		if (MatchState == MatchState::WaitingToStart || MatchState == MatchState::Cooldown) {
			SetHUDAnnouncementCountDown(TimeLeft);
		}
		else if (MatchState == MatchState::InProgress) {
			SetHUDMatchCountDown(TimeLeft);
		}
	}
	CountDownInt = SecondsLeft;
}

void AVortexPlayerController::PollInit() {
	if (CharacterOverlay == nullptr) {
		if (VortexHUD && VortexHUD->CharacterOverlay) {
			CharacterOverlay = VortexHUD->CharacterOverlay;
			if (CharacterOverlay) {
				if (bInitializeHealth)  SetHUDHealth(HUDHealth, HUDMaxHealth);
				if (bInitializeShield) SetHUDShield(HUDShield, HUDMaxShield);
				if (bInitializeScore) SetHUDScore(HUDScore);
				if (bInitializeDefeats) SetHUDDefeats(HUDDefeats);
				if (bInitializeCarriedAmmo) SetHUDCarriedAmmo(HUDCarriedAmmo);
				if (bInitializeWeaponAmmo) SetHUDWeaponAmmo(HUDWeaponAmmo);
				AVortexCharacter* VortexCharacter = Cast<AVortexCharacter>(GetPawn());
				if (VortexCharacter && VortexCharacter->GetCombat()) {
					if (bInitializeGrenades) SetHUDGrenades(VortexCharacter->GetCombat()->GetGrenades());
				}
			}
		}
	}
}

void AVortexPlayerController::ServerRequestServerTime_Implementation(float TimeOfClientRequest) {
	float ServerTimeOfReceipt = GetWorld()->GetTimeSeconds();
	ClientReportServerTime(TimeOfClientRequest, ServerTimeOfReceipt);
}

void AVortexPlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest,
                                                                    float TimeServerReceivedClientRequest) {
	float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;
	SingleTripTime = 0.5f * RoundTripTime;
	float CurrentServerTime = TimeServerReceivedClientRequest + (0.5 * RoundTripTime);
	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
}

float AVortexPlayerController::GetServerTime() {
	return GetWorld()->GetTimeSeconds() + ClientServerDelta;
}

void AVortexPlayerController::ReceivedPlayer() {
	Super::ReceivedPlayer();
	if (IsLocalController()) {
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
}

void AVortexPlayerController::OnMatchStateSet(FName State) {
	MatchState = State;
	if (MatchState ==  MatchState::InProgress) {
		HandleMatchHasStarted();
	}
	else if (MatchState == MatchState::Cooldown) {
		HandleCooldown();
	}
}

void AVortexPlayerController::OnRep_MatchState() {
	if (MatchState ==  MatchState::InProgress) {
		HandleMatchHasStarted();
	}
	else if (MatchState == MatchState::Cooldown) {
		HandleCooldown();
	}
}

void AVortexPlayerController::HandleMatchHasStarted() {
	VortexHUD = VortexHUD == nullptr ? Cast<AVortexHUD>(GetHUD()) : VortexHUD;
	if (VortexHUD) {
		if (VortexHUD->CharacterOverlay==nullptr) {
			VortexHUD->AddCharacterToOverlay();
		}
		if (VortexHUD->Announcement) {
			VortexHUD->Announcement->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void AVortexPlayerController::HandleCooldown() {
	VortexHUD = VortexHUD == nullptr ? Cast<AVortexHUD>(GetHUD()) : VortexHUD;
	if (VortexHUD) {
		VortexHUD->CharacterOverlay->RemoveFromParent();
		bool bHUDValid = VortexHUD->Announcement && VortexHUD->Announcement->AnnouncementText
			&& VortexHUD->Announcement->InfoText;
		if (bHUDValid) {
			VortexHUD->Announcement->SetVisibility(ESlateVisibility::Visible);
			FString AnnouncementText("New Match Starts In:");
			VortexHUD->Announcement->AnnouncementText->SetText(FText::FromString(AnnouncementText));

			AVortexGameState* VortexGameState = Cast<AVortexGameState>(UGameplayStatics::GetGameState(this));
			AVortexPlayerState* VortexPlayerState = GetPlayerState<AVortexPlayerState>();
			if (VortexGameState && VortexPlayerState) {
				TArray<AVortexPlayerState*> TopPlayers = VortexGameState->TopScoringPlayers;
				FString InfoTextString;
				if (TopPlayers.IsEmpty()) {
					InfoTextString = FString("There is no winner.");
				}else if (TopPlayers.Num() == 1 && TopPlayers[0] == VortexPlayerState) {
					InfoTextString = FString("You are the winner!");
				}else if (TopPlayers.Num() == 1) {
					InfoTextString = FString::Printf(TEXT("Winner: \n%s"),*TopPlayers[0]->GetPlayerName());
				}else if (TopPlayers.Num() > 1) {
					InfoTextString =  FString("Player tied for the winner:");
					for (auto TiedPlayer : TopPlayers) {
						InfoTextString.Append(FString::Printf(TEXT("\n%s"), *TiedPlayer->GetPlayerName()));
					}
				}
				VortexHUD->Announcement->InfoText->SetText(FText::FromString(InfoTextString));
			}
		}
	}
	AVortexCharacter* VortexCharacter = Cast<AVortexCharacter>(GetPawn());
	if (VortexCharacter && VortexCharacter->GetCombat()) {
		VortexCharacter->bDisableGameplay = true;
		VortexCharacter->GetCombat()->FireButtonPressed(false);
	}
}
