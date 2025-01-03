// Fill out your copyright notice in the Description page of Project Settings.


#include "VortexPlayerController.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Vortex/HUD/VortexHUD.h"
#include "Vortex/HUD/CharacterOverlay.h"
#include "Vortex/Character/VortexCharacter.h"

void AVortexPlayerController::BeginPlay() {
	Super::BeginPlay();
	VortexHUD = Cast<AVortexHUD>(GetHUD());
}

void AVortexPlayerController::OnPossess(APawn* InPawn) {
	Super::OnPossess(InPawn);
	AVortexCharacter* VortexCharacter = Cast<AVortexCharacter>(InPawn);
	if (VortexCharacter) {
		SetHUDHealth(VortexCharacter->GetHealth(), VortexCharacter->GetMaxHealth());
	}
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
}

void AVortexPlayerController::SetHUDScore(float Score) {
	VortexHUD = VortexHUD == nullptr ? Cast<AVortexHUD>(GetHUD()) : VortexHUD;
	bool bHUDValid = VortexHUD && VortexHUD->CharacterOverlay &&
		VortexHUD->CharacterOverlay->ScoreAmount;
	if (bHUDValid) {
		FString ScoreText = FString::Printf(TEXT(" %d"), FMath::FloorToInt(Score));
		VortexHUD->CharacterOverlay->ScoreAmount->SetText(FText::FromString(ScoreText));
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
}

void AVortexPlayerController::SetHUDWeaponAmmo(int32 Ammo) {
	VortexHUD = VortexHUD == nullptr ? Cast<AVortexHUD>(GetHUD()) : VortexHUD;
	bool bHUDValid = VortexHUD && VortexHUD->CharacterOverlay &&
		VortexHUD->CharacterOverlay->WeaponAmmoAmount;
	if (bHUDValid) {
		FString AmmoText = FString::Printf(TEXT(" %d"), Ammo);
		VortexHUD->CharacterOverlay->WeaponAmmoAmount->SetText(FText::FromString(AmmoText));
	}
}

void AVortexPlayerController::SetHUDCarriedAmmo(int32 Ammo) {
	VortexHUD = VortexHUD == nullptr ? Cast<AVortexHUD>(GetHUD()) : VortexHUD;
	bool bHUDValid = VortexHUD && VortexHUD->CharacterOverlay &&
		VortexHUD->CharacterOverlay->CarriedAmmoAmount;
	if (bHUDValid) {
		FString AmmoText = FString::Printf(TEXT(" %d"), Ammo);
		VortexHUD->CharacterOverlay->CarriedAmmoAmount->SetText(FText::FromString(AmmoText));
	}
}

