// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "JumpPickup.generated.h"


UCLASS()
class VORTEX_API AJumpPickup : public APickup
{
	GENERATED_BODY()
public:

protected:
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
	
private:
	UPROPERTY(EditAnywhere)
	float JumpZVelocityBuff = 4000.f;
	UPROPERTY(EditAnywhere)
	float JumpBuffTime = 10.f;
	
};
