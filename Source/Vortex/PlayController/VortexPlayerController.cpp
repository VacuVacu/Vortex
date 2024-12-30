// Fill out your copyright notice in the Description page of Project Settings.


#include "VortexPlayerController.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Vortex/HUD/VortexHUD.h"
#include "Vortex/HUD/CharacterOverlay.h"

void AVortexPlayerController::BeginPlay() {
	Super::BeginPlay();
	VortexHUD = Cast<AVortexHUD>(GetHUD());
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
