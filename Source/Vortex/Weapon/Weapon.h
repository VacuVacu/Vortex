// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponTypes.h"
#include "Net/UnrealNetwork.h"
#include "Weapon.generated.h"

class AVortexPlayerController;
class AVortexCharacter;
class ACasing;
class UWidgetComponent;
class USphereComponent;
class UTexture2D;
class USoundCue;

UENUM(BlueprintType)
enum class EWeaponState: uint8 {
	EWS_Initial UMETA(DisplayName = "Initial State"),
	EWS_Equipped UMETA(DisplayName = "Equipped"),
	EWS_EquippedSecondary UMETA(DisplayName = "Equipped Secondary"),
	EWS_Dropped UMETA(DisplayName = "Dropped"),
	EWS_MAX UMETA(DisplayName = "DefaultMax")
};

UENUM(BlueprintType)
enum class EFireType: uint8 {
	EFT_HitScan UMETA(DisplayName = "Hit Scan Weapon"),
	EFT_Projectile UMETA(DisplayName = "Projectile Weapon"),
	EFT_Shotgun UMETA(DisplayName = "Shotgun Weapon"),
	EFT_Max UMETA(DisplayName = "DefaultMax")
};

UCLASS()
class VORTEX_API AWeapon : public AActor {
	GENERATED_BODY()

public:
	AWeapon();
	virtual void Tick(float DeltaTime) override;
	void ShowPickupWidget(bool bShowWidget);
	virtual void OnRep_Owner() override;
	void SetHUDAmmo();
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Fire(const FVector& HitTarget);
	void Dropped();
	void AddAmmo(int32 AmmoToAdd);
	FVector TraceEndWithScatter(const FVector& HitTarget);

	/*
	 * enable or disable custom depth
	 */
	void EnableCustomDepth(bool bEnable);

	//Texture for crosshairs
	UPROPERTY(EditAnywhere, Category="CrossHairs")
	UTexture2D* CrosshairsCenter;
	UPROPERTY(EditAnywhere, Category="CrossHairs")
	UTexture2D* CrosshairsLeft;
	UPROPERTY(EditAnywhere, Category="CrossHairs")
	UTexture2D* CrosshairsRight;
	UPROPERTY(EditAnywhere, Category="CrossHairs")
	UTexture2D* CrosshairsTop;
	UPROPERTY(EditAnywhere, Category="CrossHairs")
	UTexture2D* CrosshairsBottom;

	// Zoomed FOV while aiming
	UPROPERTY(EditAnywhere)
	float ZoomedFOV = 30.f;
	UPROPERTY(EditAnywhere)
	float ZoomedInterpSpeed = 20.f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float FireDelay = 0.15f;
	UPROPERTY(EditAnywhere, Category = "Combat")
	bool bAutomatic = true;

	UPROPERTY(EditAnywhere)
	USoundCue* EquipSound;

	bool bDestroyWeapon = false;

	UPROPERTY(EditAnywhere)
	EFireType FireType;

	UPROPERTY(EditAnywhere, Category="WeaponScatter")
	bool bUseScatter = false;
	
protected:
	virtual void BeginPlay() override;
	virtual void OnWeaponStateSet();
	virtual void OnEquipped();
	virtual void OnDropped();
	virtual void OnEquippedSecondary();
	
	UFUNCTION()
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	                             UPrimitiveComponent* OtherComp,
	                             int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnSpheraEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	                        UPrimitiveComponent* OtherComp,
	                        int32 OtherBodyIndex);

	/*
	 * Trace end with scatter
	 */
	UPROPERTY(EditAnywhere, Category="WeaponScatter")
	float DistanceToSphere = 800.f;
	
	UPROPERTY(EditAnywhere, Category="WeaponScatter")
	float SphereRadius = 75.f;

	UPROPERTY(EditAnywhere)
	float Damage = 20.f;

	UPROPERTY(Replicated, EditAnywhere)
	bool bUseServerSideRewind = false;

	UPROPERTY()
	AVortexCharacter* VortexOwnerCharacter;
	UPROPERTY()
	AVortexPlayerController* VortexOwnerController;

private:
	UPROPERTY(VisibleAnywhere, Category="Weapon")
	USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnywhere, Category="Weapon")
	USphereComponent* AreaSphere;

	UPROPERTY(ReplicatedUsing=OnRep_WeaponState, VisibleAnywhere, Category="Weapon")
	EWeaponState WeaponState;

	UPROPERTY(VisibleAnywhere, Category="Weapon")
	UWidgetComponent* PickupWidget;

	UPROPERTY(EditAnywhere, Category="Weapon")
	UAnimationAsset* FireAnimation;

	UPROPERTY(EditAnywhere)
	TSubclassOf<ACasing> CasingClass;

	UPROPERTY(EditAnywhere)
	int32 Ammo;

	UFUNCTION(Client, Reliable)
	void ClientUpdateAmmo(int32 ServerAmmo);
	UFUNCTION(Client, Reliable)
	void ClientAddAmmo(int32 AmmoToAdd);
	
	void SpendRound();

	UPROPERTY(EditAnywhere)
	int32 MagCapacity;

	// The number of unprocessed server requests for ammo
	// incremented in SpendRound decrement in ClientUpdateAmmo
	int32 Sequence = 0;

	UFUNCTION()
	void OnRep_WeaponState();

	UPROPERTY(EditAnywhere)
	EWeaponType WeaponType;

	UFUNCTION()
	void OnPingTooHigh(bool bPingTooHigh);

public:
	void SetWeaponState(EWeaponState State);
	FORCEINLINE USphereComponent* GetAreaSphere() const { return AreaSphere; }
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }
	FORCEINLINE float GetZoomedFOV() const { return ZoomedFOV; }
	FORCEINLINE float GetZoomedInterpSpeed() const { return ZoomedInterpSpeed; }
	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }
	FORCEINLINE int32 GetAmmo() const { return Ammo; }
	FORCEINLINE int32 GetMagCapacity() const { return MagCapacity; }
	FORCEINLINE float GetDamage() const { return Damage; }
	bool IsEmpty();
	bool IsFull();
};




