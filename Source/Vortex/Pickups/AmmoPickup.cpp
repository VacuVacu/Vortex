// Fill out your copyright notice in the Description page of Project Settings.


#include "AmmoPickup.h"
#include "Vortex/VortexComponents/CombatComponent.h"
#include "Vortex/Character/VortexCharacter.h"

void AAmmoPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                  UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	AVortexCharacter* VortexCharacter = Cast<AVortexCharacter>(OtherActor);
	if (VortexCharacter) {
		UCombatComponent* Combat = VortexCharacter->GetCombat();
		if (Combat) {
			Combat->PickupAmmo(WeaponType, AmmoAmounnt);
		}
	}
	Destroy();
}
