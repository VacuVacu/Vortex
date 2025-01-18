// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "Flag.generated.h"

class UStaticMeshComponent;

UCLASS()
class VORTEX_API AFlag : public AWeapon
{
	GENERATED_BODY()
public:
	AFlag();
	virtual void Dropped() override;
	void ResetFlag();

protected:
	virtual void BeginPlay() override;
	virtual void OnEquipped() override;
	virtual void OnDropped() override;
private:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* FlagMesh;

	FTransform InitialTranform;

public:
	FORCEINLINE FTransform GetInitialTranform() const { return InitialTranform; }
};
