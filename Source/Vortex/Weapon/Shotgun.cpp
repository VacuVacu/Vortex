// Fill out your copyright notice in the Description page of Project Settings.


#include "Shotgun.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Vortex/Character/VortexCharacter.h"
#include "Sound/SoundCue.h"
#include "Vortex/PlayController/VortexPlayerController.h"
#include "Vortex/VortexComponents/LagCompensationComponent.h"

void AShotgun::FireShotgun(const TArray<FVector_NetQuantize>& HitTargets) {
	AWeapon::Fire(FVector());
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;
	AController* InstigatorController = OwnerPawn->GetController();
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket) {
		const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		const FVector Start = SocketTransform.GetLocation();
		
		TMap<AVortexCharacter*, uint32> HitMap;
		for (const auto& HitTarget : HitTargets) {
			FHitResult FireHit;
			WeaponTraceHit(Start, HitTarget, FireHit);

			AVortexCharacter* VortexCharacter = Cast<AVortexCharacter>(FireHit.GetActor());
			if (VortexCharacter) {
				if (HitMap.Contains(VortexCharacter)) {
					HitMap[VortexCharacter]++;
				}
				else {
					HitMap.Emplace(VortexCharacter, 1);
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
				UGameplayStatics::PlaySoundAtLocation(this, HitSound, FireHit.ImpactPoint, .5f,FMath::FRandRange(-0.5f,0.5f));
			}
		}
		TArray<AVortexCharacter*> HitCharacters;
		for (const auto& HitPair : HitMap) {
			if (HitPair.Key && InstigatorController) {
				if (HasAuthority() && !bUseServerSideRewind) {
					UGameplayStatics::ApplyDamage(
			HitPair.Key,
				Damage * HitPair.Value,
				InstigatorController,
				this,
				UDamageType::StaticClass());
				}
				HitCharacters.Add(HitPair.Key);
			}
		}
		if (!HasAuthority() && bUseServerSideRewind) {
			VortexOwnerCharacter = VortexOwnerCharacter == nullptr ? Cast<AVortexCharacter>(OwnerPawn) : VortexOwnerCharacter;
			VortexOwnerController = VortexOwnerController == nullptr ? Cast<AVortexPlayerController>(InstigatorController) : VortexOwnerController;
			if (VortexOwnerCharacter && VortexOwnerController && VortexOwnerCharacter->GetLagCompensation() && VortexOwnerCharacter->IsLocallyControlled()) {
				VortexOwnerCharacter->GetLagCompensation()->SHotgunServerScoreRequest(
					HitCharacters,
					Start,
					HitTargets,
					VortexOwnerController->GetServerTime() - VortexOwnerController->SingleTripTime);
			}
		}
	}
}

void AShotgun::ShotgunTraceEndWithScatter(const FVector& HitTarget, TArray<FVector_NetQuantize>& HitTargets) {
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket == nullptr) return;
	
	const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	const FVector TraceStart = SocketTransform.GetLocation();
	
	const FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
	const FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;

	for (uint32 i=0; i < NumberOfPellets; i++) {
		const FVector RandomVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
		const FVector EndLoc = SphereCenter + RandomVec;
		FVector ToEndLoc = EndLoc - TraceStart;
		ToEndLoc = TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size();
		HitTargets.Add(ToEndLoc);
	}
}
