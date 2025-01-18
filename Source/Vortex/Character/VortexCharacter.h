// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/TimelineComponent.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "Vortex/VortexTypes/TurningInPlace.h"
#include "Vortex/Interfaces/InteractWithCrosshairsInterface.h"
#include "Vortex/VortexTypes/CombatState.h"
#include "Vortex/VortexTypes/Team.h"
#include "VortexCharacter.generated.h"

class AVortexGameMode;
class UNiagaraSystem;
class UNiagaraComponent;
class ULagCompensationComponent;
class UBoxComponent;
class UBuffComponent;
class AVortexPlayerState;
class UCombatComponent;
class AWeapon;
class USpringArmComponent;
class UCameraComponent;
class UWidgetComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
class AVortexPlayerController;

DECLARE_LOG_CATEGORY_EXTERN(LogVortexCharacter, Log, All);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLeftGame);

UCLASS(config=Game)
class VORTEX_API AVortexCharacter : public ACharacter, public IInteractWithCrosshairsInterface {
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AVortexCharacter();
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void PostInitializeComponents() override;

	void PlayFireMontage(bool bAiming);
	void PlayReloadMontage();
	void PlayElimMontage();
	void PlayThrowGrenadeMontage();
	void PlaySwapMontage();

	virtual void OnRep_ReplicatedMovement() override;

	void Elim(bool bPlayerLeftGame);
	UFUNCTION(NetMulticast, Reliable)
	void MulticastElim(bool bPlayerLeftGame);

	virtual void Destroyed() override;

	UPROPERTY(Replicated)
	bool bDisableGameplay = false;

	UFUNCTION(BlueprintImplementableEvent)
	void ShowSniperScopeWidget(bool bShowScope);

	void UpdateHUDHealth();
	void UpdateHUDShield();
	void UpdateHUDAmmo();

	UPROPERTY()
	TMap<FName, UBoxComponent*> HitCollisionBoxes;

	bool bFinsihedSwapping = false;

	FOnLeftGame OnLeftGame;
	UFUNCTION(Server, Reliable)
	void ServerLeaveGame();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastGainedTheLead();
	UFUNCTION(NetMulticast, Reliable)
	void MulticastLostTheLead();

	void SetTeamColor(ETeam Team);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void Jump() override;
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Equip(const FInputActionValue& Value);
	void Crouching(const FInputActionValue& Value);
	void ReloadButtonPressed(const FInputActionValue& Value);
	void Aim(const FInputActionValue& Value);
	void AimOffset(float DeltaTime);
	void SimProxiesTurn();
	void Fire(const FInputActionValue& Value);
	void GrenadeButtonPressed(const FInputActionValue& Value);
	void CalculateAO_Pitch();
	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
	                   AController* InstigatedController, AActor* DamageCauser);
	void PollInit();
	void RotateInPlace(float DeltaTime);
	void PlayHitReactMontage();

	void SpawnDefaultWeapon();
	void DropOrDestroyWeapon(AWeapon* Weapon);
	void DropOrDestroyWeapons();

	void SetSpawnPoint();
	void OnPlayerStateInitialized();

	/*
	 * hit boxes used for server-side rewind
	 */
	UPROPERTY(EditAnywhere)
	UBoxComponent* head;
	UPROPERTY(EditAnywhere)
	UBoxComponent* pelvis;
	UPROPERTY(EditAnywhere)
	UBoxComponent* spine_02;
	UPROPERTY(EditAnywhere)
	UBoxComponent* spine_03;
	UPROPERTY(EditAnywhere)
	UBoxComponent* upperarm_l;
	UPROPERTY(EditAnywhere)
	UBoxComponent* upperarm_r;
	UPROPERTY(EditAnywhere)
	UBoxComponent* lowerarm_l;
	UPROPERTY(EditAnywhere)
	UBoxComponent* lowerarm_r;
	UPROPERTY(EditAnywhere)
	UBoxComponent* hand_l;
	UPROPERTY(EditAnywhere)
	UBoxComponent* hand_r;
	UPROPERTY(EditAnywhere)
	UBoxComponent* backpack;
	UPROPERTY(EditAnywhere)
	UBoxComponent* blanket;
	UPROPERTY(EditAnywhere)
	UBoxComponent* thigh_l;
	UPROPERTY(EditAnywhere)
	UBoxComponent* thigh_r;
	UPROPERTY(EditAnywhere)
	UBoxComponent* calf_l;
	UPROPERTY(EditAnywhere)
	UBoxComponent* calf_r;
	UPROPERTY(EditAnywhere)
	UBoxComponent* foot_l;
	UPROPERTY(EditAnywhere)
	UBoxComponent* foot_r;
	
private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* CharacterMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* EquipAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* CrouchAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* AimAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* FireAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ReloadAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* GrenadeAction;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UWidgetComponent* OverheadWidget;

	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	AWeapon* OverlappingWeapon;

	/*
	 *  Vortex Components
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UCombatComponent* Combat;
	
	UPROPERTY(VisibleAnywhere)
	UBuffComponent* Buff;
	
	UPROPERTY(VisibleAnywhere)
	ULagCompensationComponent* LagCompensation;

	UPROPERTY(EditAnywhere, Category="Combat")
	UAnimMontage* FireWeaponMontage;
	
	UPROPERTY(EditAnywhere, Category="Combat")
	UAnimMontage* ReloadMontage;

	UPROPERTY(EditAnywhere, Category="Combat")
	UAnimMontage* HitReactMontage;

	UPROPERTY(EditAnywhere, Category="Combat")
	UAnimMontage* ElimMontage;

	UPROPERTY(EditAnywhere, Category="Combat")
	UAnimMontage* ThrowGrenadeMontage;

	UPROPERTY(EditAnywhere, Category="Combat")
	UAnimMontage* SwapMontage;

	UPROPERTY(EditAnywhere)
	float CameraThreshold = 200.f;

	float AO_Yaw;
	float AO_Pitch;
	float InterpAO_Yaw;
	FRotator StartingAimRotation;

	bool bRotateRootBone;
	float TurnThreshold = 0.5f;
	FRotator ProxyRotationLastFrame;
	FRotator ProxyRotation;
	float ProxyYaw;
	float TimeSinceLastMovementReplication;

	ETurningInPlace TurningInPlace;
	void TurnInPlace(float DeltaTime);

	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);

	UFUNCTION(Server, Reliable)
	void ServerEquipButtonPressed();

	void HideCameraIfCharacterClose();
	float CalculateSpeed();

	/**
	 * PLayer Health
	 */
	UPROPERTY(EditAnywhere, Category = "Player States")
	float MaxHealth = 100.f;
	UPROPERTY(ReplicatedUsing=OnRep_Health, VisibleAnywhere, Category="Player States")
	float Health = 100.f;
	UFUNCTION()
	void OnRep_Health(float LastHealth);

	/*
	 * Player shield
	 */
	UPROPERTY(EditAnywhere, Category = "Player States")
	float MaxShield = 100.f;
	UPROPERTY(ReplicatedUsing=OnRep_Shield, VisibleAnywhere, Category="Player States")
	float Shield = 50.f;
	UFUNCTION()
	void OnRep_Shield(float LastShield);
	
	UPROPERTY()
	AVortexPlayerController* VortexPlayerController = nullptr;

	bool bElimmed = false;

	FTimerHandle ElimTimer;
	UPROPERTY(EditDefaultsOnly)
	float ElimDelay = 3.f;
	void ElimTimerFinished();

	bool bLeftGame = false;
	/*
	 * Dissolve Effect
	 */
	UPROPERTY(VisibleAnywhere)
	UTimelineComponent* DissolveTimeline;
	FOnTimelineFloat DissolveTrack;

	UPROPERTY(EditAnywhere)
	UCurveFloat* DissolveCurve;

	UFUNCTION()
	void UpdateDissolveMaterial(float DissolveValue);
	void StartDissolve();

	UPROPERTY(VisibleAnywhere, Category="Elim")
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstance;
	UPROPERTY(VisibleAnywhere, Category="Elim")
	UMaterialInstance* DissolveMaterialInstance;

	/*
	 * Team Colors
	 */
	UPROPERTY(EditAnywhere, Category="Elim")
	UMaterialInstance* RedDissolveMatInstance;
	UPROPERTY(EditAnywhere, Category="Elim")
	UMaterialInstance* RedMaterial;
	
	
	UPROPERTY(EditAnywhere, Category="Elim")
	UMaterialInstance* BlueDissolveMatInstance;
	UPROPERTY(EditAnywhere, Category="Elim")
	UMaterialInstance* BlueMaterial;

	UPROPERTY(EditAnywhere, Category="Elim")
	UMaterialInstance* OriginalMaterial;
	
	/*
	 * Elim effects
	 */
	UPROPERTY(EditAnywhere)
	UParticleSystem* ElimBotEffect;
	UPROPERTY(VisibleAnywhere)
	UParticleSystemComponent* ElimBotComponent;
	UPROPERTY(EditAnywhere)
	USoundCue* ElimBotSound;

	UPROPERTY()
	AVortexPlayerState* VortexPlayerState = nullptr;

	UPROPERTY(EditAnywhere)
	UNiagaraSystem* CrownSystem;
	
	UPROPERTY()
	UNiagaraComponent* CrownComponent;
	/*
	 * Grenade
	 */
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* AttachedGrenade;

	/*
	 * Default Weapon
	 */
	UPROPERTY(EditAnywhere)
	TSubclassOf<AWeapon> DefaultWeaponClass;

	UPROPERTY()
	AVortexGameMode* VortexGameMode = nullptr;
	
public:
	void SetOverlappingWeapon(AWeapon* Weapon);
	bool IsWeaponEquipped();
	bool IsAiming();
	FORCEINLINE float GetAO_Yaw() const { return AO_Yaw; }
	FORCEINLINE float GetAO_Pitch() const { return AO_Pitch; }
	FORCEINLINE ETurningInPlace GetTurningInPlace() const { return TurningInPlace; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE bool ShouldRotateRootBone() const { return bRotateRootBone; }
	FORCEINLINE bool IsElimmed() const { return bElimmed; }
	FORCEINLINE float GetHealth() const { return Health; }
	FORCEINLINE void SetHealth(float Amount) {	Health = Amount; }
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }
	FORCEINLINE UCombatComponent* GetCombat() const { return Combat; }
	FORCEINLINE bool GetDisableGameplay() const { return bDisableGameplay; }
	FORCEINLINE UAnimMontage* GetReloadMontage() const { return ReloadMontage; }
	FORCEINLINE UStaticMeshComponent* GetAttachedGrenade() const { return AttachedGrenade; }
	FORCEINLINE UBuffComponent* GetBuff() const { return Buff; }
	FORCEINLINE float GetShield() const { return Shield; }
	FORCEINLINE float GetMaxShield() const { return MaxShield; }
	FORCEINLINE void SetShield(float Amount) { Shield = Amount; }
	FORCEINLINE ULagCompensationComponent* GetLagCompensation() const { return LagCompensation; }
	FORCEINLINE bool IsHoldingTheFlag() const;
	bool IsLocallyReloading();
	ECombatState GetCombatState() const;
	AWeapon* GetEquippedWeapon();
	FVector GetHitTarget() const;
	ETeam GetTeam();
	void SetHoldingTheFlag(bool bHolding);
};





