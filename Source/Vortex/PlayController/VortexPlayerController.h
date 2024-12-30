// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "VortexPlayerController.generated.h"

class AVortexHUD;

UCLASS()
class VORTEX_API AVortexPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	void SetHUDHealth(float Health, float MaxHealth);
protected:
	virtual void BeginPlay() override;
	
private:
	AVortexHUD* VortexHUD;
};
