// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BuffComponent.generated.h"


class AVortexCharacter;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class VORTEX_API UBuffComponent : public UActorComponent
{
	GENERATED_BODY()
	friend class AVortexCharacter;
public:	
	UBuffComponent();
	void Heal(float HealAmount, float HealTime);
	void ReplenishShield(float ShieldAmount, float ReplenishTime);
	void BuffSpeed(float BuffBaseSpeed, float BuffCrouchSpeed, float BuffTime);
	void BuffJump(float BuffJumpVelocity, float BuffTime);
	void SetInitialSpeed(float BaseSpeed, float CrouchSpeed);
	void SetInitialJumpVelocity(float Velocity);
protected:
	virtual void BeginPlay() override;
	void HealRampUp(float DeltaTime);
	void ShieldRampUp(float DeltaTime);
private:
	UPROPERTY()
	AVortexCharacter* Character;
	
	/*
	 *  Heal Buff
	 */
	bool bHealing = false;
	float HealingRate = 0.f;
	float AmountToHeal = 0.f;

	/*
	 * Shield buff
	 */
	bool bReplenishingShield = false;
	float ShieldReplenishRate = 0.f;
	float ShieldReplenishAmount = 0.f;
	
	/*
	 * Speed Buff
	 */
	FTimerHandle SpeedBuffTimer;
	void ResetSpeed();
	float InitalBaseSpeed = 0.f;
	float InitalCrouchSpeed = 0.f;

	UFUNCTION(NetMulticast, Reliable)
	void MultiCastSpeedBuff(float BaseSpeed, float CrouchSpeed);

	/*
	 * Jump Buff
	 */
	FTimerHandle JumpBuffTimer;
	float InitialJumpVelocity;
	void ResetJump();
	UFUNCTION(NetMulticast, Reliable)
	void MultiCastJumpBuff(float JumpVelocity);
	
public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};


