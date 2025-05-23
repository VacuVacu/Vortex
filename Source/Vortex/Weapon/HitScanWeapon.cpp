// Fill out your copyright notice in the Description page of Project Settings.


#include "HitScanWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "Vortex/Character/VortexCharacter.h"
#include "Vortex/PlayController/VortexPlayerController.h"
#include "Vortex/VortexComponents/LagCompensationComponent.h"

void AHitScanWeapon::Fire(const FVector& HitTarget) {
	Super::Fire(HitTarget);

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;
	AController* InstigatorController = OwnerPawn->GetController();
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket ) {
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector Start = SocketTransform.GetLocation();
		FHitResult FireHit;
		WeaponTraceHit(Start, HitTarget, FireHit);

		AVortexCharacter* VortexCharacter = Cast<AVortexCharacter>(FireHit.GetActor());
		if (VortexCharacter && InstigatorController) {
			bool bCauseAuthDamage = !bUseServerSideRewind || OwnerPawn->IsLocallyControlled();
			if (HasAuthority() && bCauseAuthDamage) {
				const float DamageToCause = FireHit.BoneName.ToString() == FString("head") ? HeadShotDamage : Damage;
				
				UGameplayStatics::ApplyDamage(
				VortexCharacter,
				DamageToCause,
				InstigatorController,
				this,
				UDamageType::StaticClass());
				
			}else if (!HasAuthority() && bUseServerSideRewind) {
				VortexOwnerCharacter = VortexOwnerCharacter == nullptr ? Cast<AVortexCharacter>(OwnerPawn) : VortexOwnerCharacter;
				VortexOwnerController = VortexOwnerController == nullptr ? Cast<AVortexPlayerController>(InstigatorController) : VortexOwnerController;
				if (VortexOwnerCharacter && VortexOwnerController && VortexOwnerCharacter->GetLagCompensation() && VortexOwnerCharacter->IsLocallyControlled()) {
					VortexOwnerCharacter->GetLagCompensation()->ServerScoreRequest(
						VortexCharacter,
						Start,
						HitTarget,
						VortexOwnerController->GetServerTime()-VortexOwnerController->SingleTripTime
						);
				}
			}
		}
		if (ImpactParticles) {
			UGameplayStatics::SpawnEmitterAtLocation(
				GetWorld(),
				ImpactParticles,
				FireHit.ImpactPoint,
				FireHit.ImpactNormal.Rotation());
		}
		if (HitSound) {
			UGameplayStatics::PlaySoundAtLocation(this, HitSound, FireHit.ImpactPoint);
		}
		if (MuzzleFlash) {
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, SocketTransform);
		}
		if (FireSound) {
			UGameplayStatics::PlaySoundAtLocation(this, FireSound,GetActorLocation());
		}
	}
}

void AHitScanWeapon::WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit) {
	UWorld* World = GetWorld();
	if (World) {
		FVector End = TraceStart + (HitTarget - TraceStart) * 1.25f;
		World->LineTraceSingleByChannel(
				OutHit,
				TraceStart,
				End,
				ECC_Visibility);

		FVector BeamEnd = End;
		if (OutHit.bBlockingHit) {
			BeamEnd = OutHit.ImpactPoint;
		}else {
			OutHit.ImpactPoint = End;
		}
		if (BeamParticles) {
			UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(
				World,
				BeamParticles,
				TraceStart,
				FRotator::ZeroRotator,
				true);
			if (Beam) {
				Beam->SetVectorParameter(FName("Target"), BeamEnd);
			}
		}
	}
}



