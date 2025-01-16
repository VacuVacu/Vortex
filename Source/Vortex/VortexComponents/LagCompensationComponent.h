// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LagCompensationComponent.generated.h"

class AWeapon;
class AVortexPlayerController;
class AVortexCharacter;

USTRUCT(BlueprintType)
struct FBoxInformation {
	GENERATED_BODY()
	UPROPERTY()
	FVector Location;
	UPROPERTY()
	FRotator Rotation;
	UPROPERTY()
	FVector BoxExtent;
};

USTRUCT(BlueprintType)
struct FFramePackage {
	GENERATED_BODY()
	UPROPERTY()
	float Time;
	UPROPERTY()
	TMap<FName, FBoxInformation> HitBoxInfo;
	UPROPERTY()
	AVortexCharacter* Character;
};

USTRUCT(BlueprintType)
struct FServerSideRewindResult {
	GENERATED_BODY()
	UPROPERTY()
	bool bHitComfirmed = false;
	UPROPERTY()
	bool bHeadShot = false;
};

USTRUCT(BlueprintType)
struct FShotgunServerSideRewindResult {
	GENERATED_BODY()
	UPROPERTY()
	TMap<AVortexCharacter*, uint32> HeadShots;
	UPROPERTY()
	TMap<AVortexCharacter*, uint32> BodyShots;
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class VORTEX_API ULagCompensationComponent : public UActorComponent
{
	GENERATED_BODY()
	friend class AVortexCharacter;
public:	
	ULagCompensationComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void ShowFramePackage(const FFramePackage& Package,const FColor& Color);
	FServerSideRewindResult ServerSideRewind(AVortexCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime);
	FShotgunServerSideRewindResult ShotgunServerSideRewind(
		const TArray<AVortexCharacter*>& HitCharacters,
		const FVector_NetQuantize& TraceStart,
		const TArray<FVector_NetQuantize>& HitLocations,
		float HitTime);

	UFUNCTION(Server, Reliable)
	void ServerSocreRequest(AVortexCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime, AWeapon* DamageCauser);
	UFUNCTION(Server, Reliable)
	void SHotgunServerScoreRequest(
		const TArray<AVortexCharacter*>& HitCharacters,
		const FVector_NetQuantize& TraceStart,
		const TArray<FVector_NetQuantize>& HitLocations,
		float HitTime);
protected:
	virtual void BeginPlay() override;
	void SaveFramePackage(FFramePackage& Package);
	FFramePackage InterpBetweenFrames(const FFramePackage& OlderFrame, const FFramePackage& YoungerFrame, float HitTime);
	FServerSideRewindResult ConfirmHit(const FFramePackage& Package, AVortexCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation);
	void CacheBoxPositions(AVortexCharacter* HitCharacter, FFramePackage& OutFramePackage);
	void MoveBoxes(AVortexCharacter* HitCharacter, const FFramePackage& Package);
	void ResetHitBoxes(AVortexCharacter* HitCharacter, const FFramePackage& Package);
	void EnbaleCharacterMeshCollision(AVortexCharacter* HitCharacter, ECollisionEnabled::Type CollisionEnabled);
	void SaveFramePackage();
	FFramePackage GetFrameToCheck(AVortexCharacter* HitCharacter, float HitTime);
	/*
	 * Shotgun
	 */

	FShotgunServerSideRewindResult ShotgunConfirmHit(
		const TArray<FFramePackage>& FramePackages,
		const FVector_NetQuantize& TraceStart,
		const TArray<FVector_NetQuantize>& HitLocations);
	
private:
	UPROPERTY()
	AVortexCharacter* Character;

	UPROPERTY()
	AVortexPlayerController* Controller;

	TDoubleLinkedList<FFramePackage> FrameHistory;

	UPROPERTY(EditAnywhere)
	float MaxRecordTime = 4.f;
	
public:	
	
		
};
