// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "VortexHUD.generated.h"

class UCharacterOverlay;
class UTexture2D;

USTRUCT(BlueprintType)
struct FHUDPackage {
	GENERATED_BODY()

	UTexture2D* CrosshairCenter;
	UTexture2D* CrosshairLeft;
	UTexture2D* CrosshairRight;
	UTexture2D* CrosshairTop;
	UTexture2D* CrosshairBottom;
	float CrosshairSpread;
	FLinearColor CrosshairsColor;
};

UCLASS()
class VORTEX_API AVortexHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;

	UPROPERTY(EditAnywhere, Category="Player States")
	TSubclassOf<UUserWidget> CharacterOverlayClass;
	
	UCharacterOverlay* CharacterOverlay;

protected:
	virtual void BeginPlay() override;
	void AddCharacterToOverlay();
	
private:
	FHUDPackage HUDPackage;
	UPROPERTY(EditAnywhere)
	float CrosshairSpreadMax = 16.f;
	
	void DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor CrosshairColor);
public:
	FORCEINLINE void SetHUDPackage(const FHUDPackage& Package) { HUDPackage = Package; }
};
