// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "EntitySystem/MovieSceneEntitySystemRunner.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Vortex/Character/VortexCharacter.h"
#include "Vortex/Weapon/Weapon.h"
#include "Vortex/PlayController/VortexPlayerController.h"
#include "Vortex/HUD/VortexHUD.h"

DEFINE_LOG_CATEGORY(LogCombatComponent);

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	BaseWalkSpeed = 600.f;
	AimWalkSpeed = 450.f;
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, bAiming);
}

void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip) {
	if (Character==nullptr||WeaponToEquip==nullptr) {
		return;
	}
	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if (HandSocket) {
		HandSocket->AttachActor(EquippedWeapon, Character->GetMesh());
	}
	EquippedWeapon->SetOwner(Character);
	
	Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	Character->bUseControllerRotationYaw = true;
}

// Called when the game starts
void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	if (Character) {
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
	}
}

void UCombatComponent::SetAiming(bool bIsAiming) {
	bAiming = bIsAiming;
	ServerSetAiming(bIsAiming);
	if (Character) {
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
}

void UCombatComponent::ServerSetAiming_Implementation(bool bIsAiming) {
	bAiming = bIsAiming;
	if (Character) {
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
}

void UCombatComponent::OnRep_EquippedWeapon() {
	if (EquippedWeapon && Character) {
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;
	}
}

void UCombatComponent::Fire(bool bPressed) {
	bFireButtonPressed = bPressed;
	if (bFireButtonPressed) {
		FHitResult HitResult;
		TraceUnderCrossHairs(HitResult);
		ServerFire(HitResult.ImpactPoint);
	}
}

void UCombatComponent::TraceUnderCrossHairs(FHitResult& TraceHitResult) {
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport) {
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	FVector2D CrossHairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
	FVector CrossHairWorldPosition;
	FVector CrossHairWorldDirection;
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(UGameplayStatics::GetPlayerController(this, 0),
		CrossHairLocation,CrossHairWorldPosition, CrossHairWorldDirection);
	if (bScreenToWorld) {
		FVector Start = CrossHairWorldPosition;
		FVector End = Start + CrossHairWorldDirection * TRACE_LENGTH;

		GetWorld()->LineTraceSingleByChannel(TraceHitResult, Start, End, ECC_Visibility);
	}
}

void UCombatComponent::MultiCastFire_Implementation(const FVector_NetQuantize& TraceHitTarget) {
	if (EquippedWeapon == nullptr)
		return;
	if (Character) {
		Character->PlayFireMontage(bAiming);
		EquippedWeapon->Fire(TraceHitTarget);
	}
}

inline void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget) {
	MultiCastFire(TraceHitTarget);
}

// Called every frame
void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	SetHUDCorsshairs(DeltaTime);
}

void UCombatComponent::SetHUDCorsshairs(float DeltaTime) {
	if (Character==nullptr || Character->Controller == nullptr) return;
	Controller = Controller==nullptr ? Cast<AVortexPlayerController>(Character->Controller) :Controller;
	if (Controller) {
		HUD = HUD == nullptr ? Cast<AVortexHUD>(Controller->GetHUD()) : HUD;
		if (HUD) {
			FHUDPackage HUDPackage;
			if (EquippedWeapon) {
				HUDPackage.CrosshairCenter = EquippedWeapon->CrosshairsCenter;
				HUDPackage.CrosshairLeft = EquippedWeapon->CrosshairsLeft;
				HUDPackage.CrosshairRight = EquippedWeapon->CrosshairsRight;
				HUDPackage.CrosshairTop = EquippedWeapon->CrosshairsTop;
				HUDPackage.CrosshairBottom = EquippedWeapon->CrosshairsBottom;
			}else {
				HUDPackage.CrosshairCenter = nullptr;
				HUDPackage.CrosshairLeft = nullptr;
				HUDPackage.CrosshairRight = nullptr;
				HUDPackage.CrosshairTop = nullptr;
				HUDPackage.CrosshairBottom = nullptr;
			}
			// calculate corsshair spread
			FVector2D WalkSpeedRange(0.f, Character->GetCharacterMovement()->MaxWalkSpeed);
			FVector2D VelocityMultiplierRange(0.f, 1.f);
			FVector Velocity = Character->GetVelocity();
			Velocity.Z = 0.f;
			CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange, Velocity.Size());
			if (Character->GetCharacterMovement()->IsFalling()) {
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.25f, DeltaTime, 2.25f);	
			}else {
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.f, DeltaTime, 2.25f);	
			}
			HUDPackage.CrosshairSpread = CrosshairVelocityFactor + CrosshairInAirFactor;
			
			HUD->SetHUDPackage(HUDPackage);
		}
	}
}