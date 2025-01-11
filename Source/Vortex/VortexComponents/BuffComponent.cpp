// Fill out your copyright notice in the Description page of Project Settings.


#include "BuffComponent.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Vortex/Character/VortexCharacter.h"


UBuffComponent::UBuffComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	
}

void UBuffComponent::Heal(float HealAmount, float HealTime) {
	bHealing = true;
	HealingRate = HealAmount / HealTime;
	AmountToHeal += HealAmount;
}

void UBuffComponent::ReplenishShield(float ShieldAmount, float ReplenishTime) {
	bReplenishingShield = true;
	ShieldReplenishRate = ShieldAmount / ReplenishTime;
	ShieldReplenishAmount += ShieldAmount;
}

void UBuffComponent::SetInitialSpeed(float BaseSpeed, float CrouchSpeed) {
	InitalBaseSpeed = BaseSpeed;
	InitalCrouchSpeed = CrouchSpeed;
}

void UBuffComponent::SetInitialJumpVelocity(float Velocity) {
	InitialJumpVelocity = Velocity;
}

void UBuffComponent::HealRampUp(float DeltaTime) {
	if (!bHealing || Character == nullptr || Character->IsElimmed()) return;
	const float HealThisFrame = HealingRate * DeltaTime;
	Character->SetHealth(FMath::Clamp(Character->GetHealth() + HealThisFrame, 0, Character->GetMaxHealth()));
	Character->UpdateHUDHealth();
	AmountToHeal -= HealThisFrame;
	if (AmountToHeal <= 0.f || Character->GetHealth() >= Character->GetMaxHealth()) {
		bHealing = false;
		AmountToHeal = 0.f;
	}
}

void UBuffComponent::ShieldRampUp(float DeltaTime) {
	if (!bReplenishingShield || Character == nullptr || Character->IsElimmed()) return;
	const float ReplenishThisFrame = ShieldReplenishRate * DeltaTime;
	Character->SetShield(FMath::Clamp(Character->GetShield() + ReplenishThisFrame, 0, Character->GetMaxShield()));
	Character->UpdateHUDShield();
	ShieldReplenishAmount -= ReplenishThisFrame;
	if (ShieldReplenishAmount <= 0.f || Character->GetShield() >= Character->GetMaxShield()) {
		bReplenishingShield = false;
		ShieldReplenishAmount = 0.f;
	}
}

void UBuffComponent::BeginPlay()
{
	Super::BeginPlay();
	
}

void UBuffComponent::BuffSpeed(float BuffBaseSpeed, float BuffCrouchSpeed, float BuffTime) {
	if (Character == nullptr) return;

	Character->GetWorldTimerManager().SetTimer(SpeedBuffTimer, this, &UBuffComponent::ResetSpeed, BuffTime);

	if (Character->GetCharacterMovement()) {
		Character->GetCharacterMovement()->MaxWalkSpeed = BuffBaseSpeed;
		Character->GetCharacterMovement()->MaxWalkSpeedCrouched = BuffCrouchSpeed;
	}
	MultiCastSpeedBuff(BuffBaseSpeed, BuffCrouchSpeed);
}

void UBuffComponent::ResetSpeed() {
	if (Character == nullptr || Character->GetCharacterMovement() == nullptr) return;
	Character->GetCharacterMovement()->MaxWalkSpeed = InitalBaseSpeed;
	Character->GetCharacterMovement()->MaxWalkSpeedCrouched = InitalCrouchSpeed;
	MultiCastSpeedBuff(InitalBaseSpeed, InitalCrouchSpeed);
}

void UBuffComponent::MultiCastSpeedBuff_Implementation(float BaseSpeed, float CrouchSpeed) {
	if (Character && Character->GetCharacterMovement()) {
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseSpeed;
		Character->GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchSpeed;
	}
}

void UBuffComponent::BuffJump(float BuffJumpVelocity, float BuffTime) {
	if (Character == nullptr) return;

	Character->GetWorldTimerManager().SetTimer(JumpBuffTimer, this, &UBuffComponent::ResetJump, BuffTime);
	if (Character->GetCharacterMovement()) {
		Character->GetCharacterMovement()->JumpZVelocity = BuffJumpVelocity;
	}
	MultiCastJumpBuff(BuffJumpVelocity);
}

void UBuffComponent::MultiCastJumpBuff_Implementation(float JumpVelocity) {
	if (Character && Character->GetCharacterMovement()) {
		Character->GetCharacterMovement()->JumpZVelocity = JumpVelocity;
	}
}

void UBuffComponent::ResetJump() {
	if (Character == nullptr || Character->GetCharacterMovement() == nullptr) return;
	Character->GetCharacterMovement()->JumpZVelocity = InitialJumpVelocity;
	MultiCastJumpBuff(InitialJumpVelocity);
}

void UBuffComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	HealRampUp(DeltaTime);
	ShieldRampUp(DeltaTime);
}

