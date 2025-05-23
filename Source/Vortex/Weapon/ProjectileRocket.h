// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "ProjectileRocket.generated.h"

class URocketMovementComponent;
class UNiagaraComponent;
class UNiagaraSystem;

UCLASS()
class VORTEX_API AProjectileRocket : public AProjectile
{
	GENERATED_BODY()
public:
	AProjectileRocket();
	virtual void Destroyed() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& Event) override;
#endif
	
protected:
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& HitResult) override;
	virtual void BeginPlay() override;
	
	UPROPERTY(EditAnywhere)
	USoundCue* ProjectileLoop;
	UPROPERTY()
	UAudioComponent* ProjectileLoopComponent;
	UPROPERTY(EditAnywhere)
	USoundAttenuation* LoopingSoundAttenuation;
	UPROPERTY(VisibleAnywhere)
	URocketMovementComponent* RocketMovementComponent;
private:

	
};
