// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "HealthPickup.generated.h"


class UNiagaraSystem;
class UNiagaraComponent;

UCLASS()
class VORTEX_API AHealthPickup : public APickup
{
	GENERATED_BODY()
public:
	AHealthPickup();
protected:
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
							 UPrimitiveComponent* OtherComp,
							 int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
	
private:
	UPROPERTY(EditAnywhere)
	float HealAmount = 100.f;

	UPROPERTY(EditAnywhere)
	float HealTime = 5.f;
	
};
