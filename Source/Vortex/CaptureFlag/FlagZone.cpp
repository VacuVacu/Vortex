// Fill out your copyright notice in the Description page of Project Settings.


#include "FlagZone.h"
#include "Components/SphereComponent.h"
#include "Vortex/Character/VortexCharacter.h"
#include "Vortex/GameMode/CaptureFlagGameMode.h"
#include "Vortex/Weapon/Flag.h"

AFlagZone::AFlagZone()
{
 	PrimaryActorTick.bCanEverTick = false;
	ZoneSphere = CreateDefaultSubobject<USphereComponent>("ZoneSphere");
	SetRootComponent(ZoneSphere);
	
}

void AFlagZone::BeginPlay()
{
	Super::BeginPlay();
	ZoneSphere->OnComponentBeginOverlap.AddDynamic(this, &AFlagZone::OnSphereOverlap);
}

void AFlagZone::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {
	AFlag* OverlappingFlag = Cast<AFlag>(OtherActor);
	if (OverlappingFlag && OverlappingFlag->GetTeam() != Team) {
		ACaptureFlagGameMode* GameMode = GetWorld()->GetAuthGameMode<ACaptureFlagGameMode>();
		if (GameMode) {
			GameMode->FlagCaptured(OverlappingFlag, this);
		}
		OverlappingFlag->ResetFlag();
	}
}

