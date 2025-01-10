// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Logging/LogMacros.h"
#include "Vortex/HUD/VortexHUD.h"
#include "Vortex/VortexTypes/CombatState.h"
#include "Vortex/Weapon/WeaponTypes.h"
#include "CombatComponent.generated.h"


class AProjectile;
class AVortexHUD;
class AVortexPlayerController;
class AWeapon;
class AVortexCharacter;

// DECLARE_LOG_CATEGORY_EXTERN(LogCombatComponent, Log, All);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class VORTEX_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	friend class AVortexCharacter;
	UCombatComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	void EquipWeapon(AWeapon* WeaponToEquip);
	void Reload();
	void FireButtonPressed(bool bPressed);

	UFUNCTION(BlueprintCallable)
	void ShutgunShellReload();

	void JumpToShotgunEnd();

	UFUNCTION(BlueprintCallable)
	void ThrowGrenadeFinished();

	UFUNCTION(BlueprintCallable)
	void LaunchGrenade();

	UFUNCTION(Server, Reliable)
	void ServerLaunchGrenade(const FVector_NetQuantize& Target);
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	void SetAiming(bool bIsAiming);

	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bIsAiming);

	UFUNCTION()
	void OnRep_EquippedWeapon();
	void Fire();

	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(NetMulticast, Reliable)
	void MultiCastFire(const FVector_NetQuantize& TraceHitTarget);

	void TraceUnderCrossHairs(FHitResult& TraceHitResult);

	void SetHUDCorsshairs(float DeltaTime);

	UFUNCTION(Server, Reliable)
	void ServerReload();

	void HandleReload();
	int32 AmountToReload();

	void ThrowGrenade();

	UFUNCTION(Server, Reliable)
	void ServerThrowGrenade();

	UPROPERTY(EditAnywhere)
	TSubclassOf<AProjectile> GrenadeClass;
	
	UFUNCTION(BlueprintCallable)
	void FinishReloading();

	void DropEquippedWeapon();

	void AttachActorToRightHand(AActor* ActorToAttach);

	void AttachActorToLeftHand(AActor* ActorToAttach);

	void UpdateCarriedAmmo();

	void PlayEquipWeaponSound();

	void ReloadEmptyWeapon();

	void ShowAttachedGrenade(bool bShowGrenade);

	UFUNCTION()
	void OnRep_Grenades();

	void UpdateHUDGrenades();

private:
	UPROPERTY()
	AVortexCharacter* Character;
	UPROPERTY()
	AVortexPlayerController *Controller;
	UPROPERTY()
	AVortexHUD* HUD;

	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	AWeapon* EquippedWeapon;

	UPROPERTY(Replicated)
	bool bAiming;

	UPROPERTY(EditAnywhere)
	float BaseWalkSpeed;
	
	UPROPERTY(EditAnywhere)
	float AimWalkSpeed;

	bool bFireButtonPressed;
	float CrosshairVelocityFactor;
	float CrosshairInAirFactor;
	float CrosshairAimFactor;
	float CrosshairShootingFactor;

	FVector HitTarget;
	FHUDPackage HUDPackage;

	float DefaultFOV;
	UPROPERTY(EditAnywhere, Category = "Combat")
	float ZoomedFOV = 30.f;

	float CurrentFOV;
	
	UPROPERTY(EditAnywhere, Category = "Combat")
	float ZoomedInterpSpeed = 20.f;

	void InterpFOV(float DeltaTime);

	FTimerHandle FireTimer;
	bool bCanFire = true;

	void StartFireTimer();
	void FireTimerFinished();

	bool CanFire();

	// carried ammo for currently equipped weapon
	UPROPERTY(ReplicatedUsing=OnRep_CarriedAmmo)
	int32 CarriedAmmo;

	UFUNCTION()
	void OnRep_CarriedAmmo();
	
	TMap<EWeaponType, int32> CarriedAmmoMap;
	UPROPERTY(EditAnywhere)
	int32 StartingARAmmo = 30;

	UPROPERTY(EditAnywhere)
	int32 StartingRocketAmmo = 0;

	UPROPERTY(EditAnywhere)
	int32 StartingPistolAmmo = 0;

	UPROPERTY(EditAnywhere)
	int32 StartingSMGAmmo = 0;
	
	UPROPERTY(EditAnywhere)
	int32 StartingShotgunAmmo = 0;

	UPROPERTY(EditAnywhere)
	int32 StartingSniperAmmo = 0;

	UPROPERTY(EditAnywhere)
	int32 StartingGrenadeLauncherAmmo = 0;

	UPROPERTY(ReplicatedUsing=OnRep_Grenades)
	int32 Grenades = 4;
	
	UPROPERTY(EditAnywhere)
	int32 MaxGrenades = 4;
	
	void InitializeCarriedAmmo();

	UPROPERTY(ReplicatedUsing=OnRep_CombatState)
	ECombatState CombatState = ECombatState::ECS_Unoccupied;

	UFUNCTION()
	void OnRep_CombatState();

	void UpdateAmmoValues();
	void UpdateShotgunAmmoValues();
	
public:	
	FORCEINLINE int32 GetGrenades() const { return Grenades; }
		
};








