// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

class ACasing;
class UWidgetComponent;
class USphereComponent;
class UTexture2D;

UENUM(BlueprintType)
enum class EWeaponState: uint8 {
	EWS_Initial UMETA(DisplayName = "Initial State"),
	EWS_Equipped UMETA(DisplayName = "Equipped"),
	EWS_Dropped UMETA(DisplayName = "Dropped"),
	EWS_MAX UMETA(DisplayName = "DefaultMax")
};

UCLASS()
class VORTEX_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	AWeapon();
	virtual void Tick(float DeltaTime) override;
	void ShowPickupWidget(bool bShowWidget);
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Fire(const FVector& HitTarget);

	
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

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnSpheraEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);

private:
	UPROPERTY(VisibleAnywhere, Category="Weapon")
	USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnywhere, Category="Weapon")
	USphereComponent* AreaSphere;

	UPROPERTY(ReplicatedUsing=OnRep_WeaponState ,VisibleAnywhere, Category="Weapon")
	EWeaponState WeaponState;

	UPROPERTY(VisibleAnywhere, Category="Weapon")
	UWidgetComponent* PickupWidget;

	UPROPERTY(EditAnywhere, Category="Weapon")
	UAnimationAsset* FireAnimation;

	UPROPERTY(EditAnywhere)
	TSubclassOf<ACasing> CasingClass;
	
	UFUNCTION()
	void OnRep_WeaponState();
	
public:	
	void SetWeaponState(EWeaponState State);
	FORCEINLINE USphereComponent* GetAreaSphere() const {return AreaSphere;}
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const {return WeaponMesh;}
};

