// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Vortex/VortexTypes/TurningInPlace.h"
#include "VortexAnimInstance.generated.h"

class AVortexCharacter;
class AWeapon;

UCLASS()
class VORTEX_API UVortexAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

private:
	UPROPERTY(BlueprintReadOnly, Category="Character", meta=(AllowPrivateAccess=true))
	AVortexCharacter* VortexCharacter;
	UPROPERTY(BlueprintReadOnly, Category="Movement", meta=(AllowPrivateAccess=true))
	float Speed;
	UPROPERTY(BlueprintReadOnly, Category="Movement", meta=(AllowPrivateAccess=true))
	bool bIsInAir;
	UPROPERTY(BlueprintReadOnly, Category="Movement", meta=(AllowPrivateAccess=true))
	bool bIsAcclerating;
	UPROPERTY(BlueprintReadOnly, Category="Movement", meta=(AllowPrivateAccess=true))
	bool bWeaponEquipped;
	UPROPERTY(BlueprintReadOnly, Category="Movement", meta=(AllowPrivateAccess=true))
	bool bIsCrouched;
	UPROPERTY(BlueprintReadOnly, Category="Movement", meta=(AllowPrivateAccess=true))
	bool bAiming;
	UPROPERTY(BlueprintReadOnly, Category="Movement", meta=(AllowPrivateAccess=true))
	float YawOffset;
	UPROPERTY(BlueprintReadOnly, Category="Movement", meta=(AllowPrivateAccess=true))
	float Lean;
	UPROPERTY(BlueprintReadOnly, Category="Movement", meta=(AllowPrivateAccess=true))
	float AO_Yaw;
	UPROPERTY(BlueprintReadOnly, Category="Movement", meta=(AllowPrivateAccess=true))
	float AO_Pitch;
	UPROPERTY(BlueprintReadOnly, Category="Movement", meta=(AllowPrivateAccess=true))
	FTransform LeftHandTransform;
	UPROPERTY(BlueprintReadOnly, Category="Movement", meta=(AllowPrivateAccess=true))
	ETurningInPlace TurningInPlace;

	AWeapon* EquippedWeapon;
	
	FRotator CharacterRotationLastFrame;
	FRotator CharacterRotation;
	FRotator DeltaRotation;
	
};