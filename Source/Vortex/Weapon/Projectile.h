// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

class UNiagaraSystem;
class UNiagaraComponent;
class UProjectileMovementComponent;
class UBoxComponent;

UCLASS()
class VORTEX_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	AProjectile();
	virtual void Tick(float DeltaTime) override;
	virtual void Destroyed() override;

	UPROPERTY(EditAnywhere)
	float Damage = 20.f;
protected:
	virtual void BeginPlay() override;
	void StartDestroyTimer();
	void DestroyTimerFinished();
	void SpawnTrailSystem();
	void ExplodeDamage();

	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& HitResult);
	UPROPERTY(EditAnywhere)
	UBoxComponent* CollisionBox;
	UPROPERTY(EditAnywhere)
	UParticleSystem* ImpactParticles;
	UPROPERTY(EditAnywhere)
	USoundCue* ImpactSound;
	
	UPROPERTY(VisibleAnywhere)
	UProjectileMovementComponent* ProjectileMovementComponent;

	UPROPERTY(EditAnywhere)
	UNiagaraSystem* TrailSystem;
	UPROPERTY()
	UNiagaraComponent* TrailSystemComponent;
	
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* ProjectileMesh;

	UPROPERTY(EditAnywhere)
	float DamageInnerRadius = 200.f;
	UPROPERTY(EditAnywhere)
	float DamageOuterRadius = 500.f;
	
private:
	
	UPROPERTY(EditAnywhere)
	UParticleSystem* Tracer;
	UPROPERTY()
	UParticleSystemComponent* TracerComponent;

	FTimerHandle DestroyTimer;

	UPROPERTY(EditAnywhere)
	float DestroyTime = 3.f;


public:	


};
