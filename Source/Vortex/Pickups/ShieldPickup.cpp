// Fill out your copyright notice in the Description page of Project Settings.


#include "ShieldPickup.h"
#include "Vortex/Character/VortexCharacter.h"
#include "Vortex/VortexComponents/BuffComponent.h"

void AShieldPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
	
	AVortexCharacter* VortexCharacter = Cast<AVortexCharacter>(OtherActor);
	if (VortexCharacter) {
		UBuffComponent* Buff = VortexCharacter->GetBuff();
		if (Buff) {
			Buff->ReplenishShield(ShieldReplenishAmount, ShieldReplenishTime);
		}
	}
	Destroy();
}
