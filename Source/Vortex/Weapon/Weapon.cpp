// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"

#include "Casing.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Vortex/Character/VortexCharacter.h"
#include "Vortex/PlayController/VortexPlayerController.h"
#include "Vortex/VortexComponents/CombatComponent.h"

// Sets default values
AWeapon::AWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);
	
	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(WeaponMesh);

	WeaponMesh->SetCollisionResponseToAllChannels(ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_BLUE);
	WeaponMesh->MarkRenderStateDirty();
	EnableCustomDepth(true);

	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	AreaSphere->SetupAttachment(RootComponent);
	AreaSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	PickupWidget->SetupAttachment(RootComponent);
}

void AWeapon::ShowPickupWidget(bool bShowWidget) {
	if (PickupWidget) {
		PickupWidget->SetVisibility(bShowWidget);
	}
}

void AWeapon::OnRep_Owner() {
	Super::OnRep_Owner();
	if (Owner == nullptr) {
		VortexOwnerController = nullptr;
		VortexOwnerCharacter = nullptr;
	}else {
		VortexOwnerCharacter = VortexOwnerCharacter == nullptr ? Cast<AVortexCharacter>(Owner) : VortexOwnerCharacter;
		if (VortexOwnerCharacter && VortexOwnerCharacter->GetEquippedWeapon() && VortexOwnerCharacter->GetEquippedWeapon() == this) {
			SetHUDAmmo();
		}
	}
}

void AWeapon::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AWeapon, WeaponState);
	DOREPLIFETIME_CONDITION(AWeapon, bUseServerSideRewind, COND_OwnerOnly);
}

void AWeapon::EnableCustomDepth(bool bEnable) {
	if (WeaponMesh) {
		WeaponMesh->SetRenderCustomDepth(bEnable);
	}
}

// Called when the game starts or when spawned
void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	if (PickupWidget) {
		PickupWidget->SetVisibility(false);
	}
	
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	AreaSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnSphereOverlap);
	AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AWeapon::OnSpheraEndOverlap);
}

void AWeapon::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                              UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {
	AVortexCharacter* VortexCharacter = Cast<AVortexCharacter>(OtherActor);
	if (VortexCharacter && PickupWidget) {
		if (WeaponType == EWeaponType::EWT_Flag && VortexCharacter->GetTeam() == Team) return;
		if (VortexCharacter->IsHoldingTheFlag()) return;
		VortexCharacter->SetOverlappingWeapon(this);
	}
}

void AWeapon::OnSpheraEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) {
	AVortexCharacter* VortexCharacter = Cast<AVortexCharacter>(OtherActor);
	if (VortexCharacter) {
		if (WeaponType == EWeaponType::EWT_Flag && VortexCharacter->GetTeam() == Team) return;
		if (VortexCharacter->IsHoldingTheFlag()) return;
		VortexCharacter->SetOverlappingWeapon(nullptr);
	}
}

void AWeapon::SetHUDAmmo() {
	VortexOwnerCharacter = VortexOwnerCharacter == nullptr ? Cast<AVortexCharacter>(GetOwner()) : VortexOwnerCharacter;
	if (VortexOwnerCharacter) {
		VortexOwnerController = VortexOwnerController == nullptr ? Cast<AVortexPlayerController>(VortexOwnerCharacter->Controller) : VortexOwnerController;
		if (VortexOwnerController) {
			VortexOwnerController->SetHUDWeaponAmmo(Ammo);
		}
	}
}

void AWeapon::SpendRound() {
	Ammo = FMath::Clamp(Ammo - 1, 0, MagCapacity);
	SetHUDAmmo();
	if (HasAuthority()) {
		ClientUpdateAmmo(Ammo);
	}else {
		++Sequence;
	}
}

void AWeapon::ClientUpdateAmmo_Implementation(int32 ServerAmmo) {
	if (HasAuthority()) return;
	Ammo = ServerAmmo;
	--Sequence;
	Ammo -= Sequence;
	SetHUDAmmo();
}

void AWeapon::AddAmmo(int32 AmmoToAdd) {
	Ammo = FMath::Clamp(Ammo + AmmoToAdd, 0, MagCapacity);
	SetHUDAmmo();
	ClientAddAmmo(AmmoToAdd);
}

void AWeapon::ClientAddAmmo_Implementation(int32 AmmoToAdd) {
	if (HasAuthority()) return;
	Ammo = FMath::Clamp(Ammo + AmmoToAdd, 0, MagCapacity);
	VortexOwnerCharacter = VortexOwnerCharacter == nullptr ? Cast<AVortexCharacter>(GetOwner()) : VortexOwnerCharacter;
	if (VortexOwnerCharacter && VortexOwnerCharacter->GetCombat() && IsFull()) {
		VortexOwnerCharacter->GetCombat()->JumpToShotgunEnd();
	}
	SetHUDAmmo();
}

void AWeapon::SetWeaponState(EWeaponState State) {
	WeaponState = State;
	OnWeaponStateSet();
}

void AWeapon::OnWeaponStateSet() {
	switch (WeaponState) {
	case EWeaponState::EWS_Equipped:
		OnEquipped();
		break;
	case EWeaponState::EWS_EquippedSecondary:
		OnEquippedSecondary();
		break;
	case EWeaponState::EWS_Dropped:
		OnDropped();
		break;
	}
}

void AWeapon::OnPingTooHigh(bool bPingTooHigh) {
	bUseServerSideRewind = !bPingTooHigh;
}

void AWeapon::OnRep_WeaponState() {
	OnWeaponStateSet();
}

void AWeapon::OnEquipped() {
	ShowPickupWidget(false);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetSimulatePhysics(false);
	WeaponMesh->SetEnableGravity(false);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (WeaponType == EWeaponType::EWT_SubmachineGun) {
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	}
	EnableCustomDepth(false);

	VortexOwnerCharacter = VortexOwnerCharacter == nullptr ? Cast<AVortexCharacter>(GetOwner()) : VortexOwnerCharacter;
	if (VortexOwnerCharacter && bUseServerSideRewind) {
		VortexOwnerController = VortexOwnerController == nullptr ? Cast<AVortexPlayerController>(VortexOwnerCharacter->Controller) : VortexOwnerController;
		if (VortexOwnerController && HasAuthority() && !VortexOwnerController->HighPingDelegate.IsBound()) {
			VortexOwnerController->HighPingDelegate.AddDynamic(this, &AWeapon::OnPingTooHigh);
		}
	}
}

void AWeapon::OnDropped() {
	if (HasAuthority()) {
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
	WeaponMesh->SetSimulatePhysics(true);
	WeaponMesh->SetEnableGravity(true);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	WeaponMesh->SetCollisionResponseToAllChannels(ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	WeaponMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_BLUE);
	WeaponMesh->MarkRenderStateDirty();
	EnableCustomDepth(true);
	VortexOwnerCharacter = VortexOwnerCharacter == nullptr ? Cast<AVortexCharacter>(GetOwner()) : VortexOwnerCharacter;
	if (VortexOwnerCharacter && bUseServerSideRewind) {
		VortexOwnerController = VortexOwnerController == nullptr ? Cast<AVortexPlayerController>(VortexOwnerCharacter->Controller) : VortexOwnerController;
		if (VortexOwnerController && HasAuthority() && VortexOwnerController->HighPingDelegate.IsBound()) {
			VortexOwnerController->HighPingDelegate.RemoveDynamic(this, &AWeapon::OnPingTooHigh);
		}
	}
}

void AWeapon::OnEquippedSecondary() {
	ShowPickupWidget(false);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetSimulatePhysics(false);
	WeaponMesh->SetEnableGravity(false);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (WeaponType == EWeaponType::EWT_SubmachineGun) {
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	}
	if (WeaponMesh) {
		WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_TAN);
		WeaponMesh->MarkRenderStateDirty();
	}
	VortexOwnerCharacter = VortexOwnerCharacter == nullptr ? Cast<AVortexCharacter>(GetOwner()) : VortexOwnerCharacter;
	if (VortexOwnerCharacter && bUseServerSideRewind) {
		VortexOwnerController = VortexOwnerController == nullptr ? Cast<AVortexPlayerController>(VortexOwnerCharacter->Controller) : VortexOwnerController;
		if (VortexOwnerController && HasAuthority() && VortexOwnerController->HighPingDelegate.IsBound()) {
			VortexOwnerController->HighPingDelegate.RemoveDynamic(this, &AWeapon::OnPingTooHigh);
		}
	}
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWeapon::Fire(const FVector& HitTarget) {
	if (FireAnimation) {
		WeaponMesh->PlayAnimation(FireAnimation, false);
	}
	if (CasingClass) {
		const USkeletalMeshSocket* AmmoEjectSocket = WeaponMesh->GetSocketByName(FName("AmmoEject"));
		if (AmmoEjectSocket) {
			FTransform SocketTransform = AmmoEjectSocket->GetSocketTransform(WeaponMesh);
			UWorld* World = GetWorld();
			if (World) {
				World->SpawnActor<ACasing>(CasingClass, SocketTransform.GetLocation(), SocketTransform.GetRotation().Rotator());
			}
		}
	}
	SpendRound();
}

void AWeapon::Dropped() {
	SetWeaponState(EWeaponState::EWS_Dropped);
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	WeaponMesh->DetachFromComponent(DetachRules);
	SetOwner(nullptr);
	VortexOwnerCharacter = nullptr;
	VortexOwnerController = nullptr;
}

bool AWeapon::IsEmpty() {
	return Ammo <= 0;
}

bool AWeapon::IsFull() {
	return Ammo == MagCapacity;
}


FVector AWeapon::TraceEndWithScatter(const FVector& HitTarget) {
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket == nullptr) return FVector();
	
	const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	const FVector TraceStart = SocketTransform.GetLocation();
	
	const FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
	const FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;
	const FVector RandomVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
	const FVector EndLoc = SphereCenter + RandomVec;
	const FVector ToEndLoc = EndLoc - TraceStart;
	// DrawDebugSphere(GetWorld(), SphereCenter, SphereRadius, 12, FColor::Red, true);
	// DrawDebugSphere(GetWorld(), EndLoc, 4.f, 12, FColor::Orange, true);
	// DrawDebugLine(GetWorld(), TraceStart, FVector(TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size()), FColor::Cyan, true);
	return FVector(TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size());
}

