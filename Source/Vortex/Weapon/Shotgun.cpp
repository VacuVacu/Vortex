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
		TMap<AVortexCharacter*, uint32> HeadShotHitMap;
		
		for (const auto& HitTarget : HitTargets) {
			FHitResult FireHit;
			WeaponTraceHit(Start, HitTarget, FireHit);

			AVortexCharacter* VortexCharacter = Cast<AVortexCharacter>(FireHit.GetActor());
			if (VortexCharacter) {
				const bool bHeadShot = FireHit.BoneName.ToString() == FName("head");
				if (bHeadShot) {
					if (HeadShotHitMap.Contains(VortexCharacter)) {
						HeadShotHitMap[VortexCharacter]++;
					}else {
						HeadShotHitMap.Emplace(VortexCharacter, 1);
					}
				}else {
					if (HitMap.Contains(VortexCharacter)) {
						HitMap[VortexCharacter]++;
					}
					else {
						HitMap.Emplace(VortexCharacter, 1);
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
				UGameplayStatics::PlaySoundAtLocation(this, HitSound, FireHit.ImpactPoint, .5f,FMath::FRandRange(-0.5f,0.5f));
			}
		}
		TArray<AVortexCharacter*> HitCharacters;
		TMap<AVortexCharacter*, float> DamageMap;
		for (const auto& HitPair : HitMap) {
			if (HitPair.Key && InstigatorController) {
				DamageMap.Emplace(HitPair.Key, HitPair.Value *  Damage);
				HitCharacters.AddUnique(HitPair.Key);
			}
		}
		for (const auto& HitPair : HeadShotHitMap) {
			if (HitPair.Key && InstigatorController) {
				if (DamageMap.Contains(HitPair.Key)) DamageMap[HitPair.Key] += HitPair.Value * HeadShotDamage;
				else DamageMap.Emplace(HitPair.Key, HitPair.Value * HeadShotDamage);
				HitCharacters.AddUnique(HitPair.Key);
			}
		}
		for (auto DamagePair : DamageMap) {
			if (DamagePair.Key && InstigatorController) {
				bool bCauseAuthDamage = !bUseServerSideRewind || OwnerPawn->IsLocallyControlled();
				if (HasAuthority() && bCauseAuthDamage) {
					UGameplayStatics::ApplyDamage(
				DamagePair.Key,
					DamagePair.Value,
					InstigatorController,
					this,
					UDamageType::StaticClass());
				}
			}
		}
		
		if (!HasAuthority() && bUseServerSideRewind) {
			VortexOwnerCharacter = VortexOwnerCharacter == nullptr ? Cast<AVortexCharacter>(OwnerPawn) : VortexOwnerCharacter;
			VortexOwnerController = VortexOwnerController == nullptr ? Cast<AVortexPlayerController>(InstigatorController) : VortexOwnerController;
			if (VortexOwnerCharacter && VortexOwnerController && VortexOwnerCharacter->GetLagCompensation() && VortexOwnerCharacter->IsLocallyControlled()) {
				VortexOwnerCharacter->GetLagCompensation()->ShotgunServerScoreRequest(
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
