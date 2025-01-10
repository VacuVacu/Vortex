// Fill out your copyright notice in the Description page of Project Settings.


#include "VortexAnimInstance.h"
#include "VortexCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Vortex/Weapon/Weapon.h"

void UVortexAnimInstance::NativeInitializeAnimation() {
	Super::NativeInitializeAnimation();
	VortexCharacter = Cast<AVortexCharacter>(TryGetPawnOwner());
	
}

void UVortexAnimInstance::NativeUpdateAnimation(float DeltaSeconds) {
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (VortexCharacter==nullptr) {
		VortexCharacter = Cast<AVortexCharacter>(TryGetPawnOwner());
	}
	if (VortexCharacter==nullptr) 
		return;
	FVector Velocity = VortexCharacter->GetVelocity();
	Velocity.Z = 0.f;
	Speed = Velocity.Size();

	bIsInAir = VortexCharacter->GetCharacterMovement()->IsFalling();
	bIsAcclerating = VortexCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f;
	bWeaponEquipped = VortexCharacter->IsWeaponEquipped();
	EquippedWeapon = VortexCharacter->GetEquippedWeapon();
	bIsCrouched = VortexCharacter->bIsCrouched;
	bAiming = VortexCharacter->IsAiming();
	TurningInPlace = VortexCharacter->GetTurningInPlace();
	bRotateRootBone = VortexCharacter->ShouldRotateRootBone();
	bElimmed = VortexCharacter->IsElimmed();

	FRotator AimRotation = VortexCharacter->GetBaseAimRotation();
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(VortexCharacter->GetVelocity());
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);
	DeltaRotation = FMath::RInterpTo(DeltaRotation,DeltaRot,DeltaSeconds,6.f);
	YawOffset = DeltaRotation.Yaw; 
   
	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = VortexCharacter->GetActorRotation();
	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame);
	const float Target = Delta.Yaw / DeltaSeconds;
	const float Interp = FMath::FInterpTo(Lean,Target,DeltaSeconds,6.f);
	Lean = FMath::Clamp(Interp, -90.f, 90.f);

	AO_Yaw = VortexCharacter->GetAO_Yaw();
	AO_Pitch = VortexCharacter->GetAO_Pitch();

	if (bWeaponEquipped && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && VortexCharacter->GetMesh()) {
		LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), RTS_World);
		FVector OutPosition;
		FRotator OutRotation;
		VortexCharacter->GetMesh()->TransformToBoneSpace(FName("hand_r"), LeftHandTransform.GetLocation(), FRotator::ZeroRotator, OutPosition, OutRotation );
		LeftHandTransform.SetLocation(OutPosition);
		LeftHandTransform.SetRotation(FQuat(OutRotation));

		if (VortexCharacter->IsLocallyControlled()) {
			bLocallyControlled = true;
			FTransform RightHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("hand_R"), RTS_World);
			FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(RightHandTransform.GetLocation(),
				RightHandTransform.GetLocation() + (RightHandTransform.GetLocation() - VortexCharacter->GetHitTarget()));
			RightHandRotation = FMath::RInterpTo(RightHandRotation,LookAtRotation,DeltaSeconds,30.f);
		}
	}
	bUseFABRIC = VortexCharacter->GetCombatState() == ECombatState::ECS_Unoccupied;
	bUseAimOffsets = VortexCharacter->GetCombatState() == ECombatState::ECS_Unoccupied && !VortexCharacter->GetDisableGameplay();
	bTransformRightHand = VortexCharacter->GetCombatState() == ECombatState::ECS_Unoccupied && !VortexCharacter->GetDisableGameplay();
}
