// Fill out your copyright notice in the Description page of Project Settings.


#include "VortexCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Vortex/VortexComponents/CombatComponent.h"
#include "Vortex/Weapon/Weapon.h"
#include "VortexAnimInstance.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "Vortex/Vortex.h"
#include "Vortex/GameMode/VortexGameMode.h"
#include "Vortex/GameState/VortexGameState.h"
#include "Vortex/PlayController/VortexPlayerController.h"
#include "Vortex/PlayerState/VortexPlayerState.h"
#include "Vortex/VortexComponents/BuffComponent.h"
#include "Vortex/VortexComponents/LagCompensationComponent.h"
#include "Vortex/Weapon/WeaponTypes.h"

DEFINE_LOG_CATEGORY(LogVortexCharacter);

AVortexCharacter::AVortexCharacter() {
	PrimaryActorTick.bCanEverTick = true;
	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 500.0f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	TurningInPlace = ETurningInPlace::ETIP_NotTurning;

	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));

	Buff = CreateDefaultSubobject<UBuffComponent>(TEXT("BuffComponent"));
	Buff->SetIsReplicated(true);

	LagCompensation = CreateDefaultSubobject<ULagCompensationComponent>(TEXT("LagCompensation"));
	
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECollisionResponse::ECR_Block);
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 850.f, 0.0f);

	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;

	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimelineComponent"));

	AttachedGrenade = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AttachedGrenade"));
	AttachedGrenade->SetupAttachment(GetMesh(), FName("GrenadeSocket"));
	AttachedGrenade->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	/*
	 * Hit boxes for server-side rewind
	 */
	head = CreateDefaultSubobject<UBoxComponent>(TEXT("head"));
	head->SetupAttachment(GetMesh(), FName("head"));
	HitCollisionBoxes.Add(FName("head"), head);

	pelvis = CreateDefaultSubobject<UBoxComponent>(TEXT("pelvis"));
	pelvis->SetupAttachment(GetMesh(), FName("pelvis"));
	HitCollisionBoxes.Add(FName("pelvis"), pelvis);

	spine_02 = CreateDefaultSubobject<UBoxComponent>(TEXT("spine_02"));
	spine_02->SetupAttachment(GetMesh(), FName("spine_02"));
	HitCollisionBoxes.Add(FName("spine_02"), spine_02);

	spine_03 = CreateDefaultSubobject<UBoxComponent>(TEXT("spine_03"));
	spine_03->SetupAttachment(GetMesh(), FName("spine_03"));
	HitCollisionBoxes.Add(FName("spine_03"), spine_03);

	upperarm_l = CreateDefaultSubobject<UBoxComponent>(TEXT("upperarm_l"));
	upperarm_l->SetupAttachment(GetMesh(), FName("upperarm_l"));
	HitCollisionBoxes.Add(FName("upperarm_l"), upperarm_l);

	upperarm_r = CreateDefaultSubobject<UBoxComponent>(TEXT("upperarm_r"));
	upperarm_r->SetupAttachment(GetMesh(), FName("upperarm_r"));
	HitCollisionBoxes.Add(FName("upperarm_r"), upperarm_r);

	lowerarm_l = CreateDefaultSubobject<UBoxComponent>(TEXT("lowerarm_l"));
	lowerarm_l->SetupAttachment(GetMesh(), FName("lowerarm_l"));
	HitCollisionBoxes.Add(FName("lowerarm_l"), lowerarm_l);

	lowerarm_r = CreateDefaultSubobject<UBoxComponent>(TEXT("lowerarm_r"));
	lowerarm_r->SetupAttachment(GetMesh(), FName("lowerarm_r"));
	HitCollisionBoxes.Add(FName("lowerarm_r"), lowerarm_r);

	hand_l = CreateDefaultSubobject<UBoxComponent>(TEXT("hand_l"));
	hand_l->SetupAttachment(GetMesh(), FName("hand_l"));
	HitCollisionBoxes.Add(FName("hand_l"), hand_l);

	hand_r = CreateDefaultSubobject<UBoxComponent>(TEXT("hand_r"));
	hand_r->SetupAttachment(GetMesh(), FName("hand_r"));
	HitCollisionBoxes.Add(FName("hand_r"), hand_r);

	backpack = CreateDefaultSubobject<UBoxComponent>(TEXT("backpack"));
	backpack->SetupAttachment(GetMesh(), FName("backpack"));
	HitCollisionBoxes.Add(FName("backpack"), backpack);

	blanket = CreateDefaultSubobject<UBoxComponent>(TEXT("blanket"));
	blanket->SetupAttachment(GetMesh(), FName("backpack"));
	HitCollisionBoxes.Add(FName("blanket"), blanket);

	thigh_l = CreateDefaultSubobject<UBoxComponent>(TEXT("thigh_l"));
	thigh_l->SetupAttachment(GetMesh(), FName("thigh_l"));
	HitCollisionBoxes.Add(FName("thigh_l"), thigh_l);

	thigh_r = CreateDefaultSubobject<UBoxComponent>(TEXT("thigh_r"));
	thigh_r->SetupAttachment(GetMesh(), FName("thigh_r"));
	HitCollisionBoxes.Add(FName("thigh_r"), thigh_r);

	calf_l = CreateDefaultSubobject<UBoxComponent>(TEXT("calf_l"));
	calf_l->SetupAttachment(GetMesh(), FName("calf_l"));
	HitCollisionBoxes.Add(FName("calf_l"), calf_l);

	calf_r = CreateDefaultSubobject<UBoxComponent>(TEXT("calf_r"));
	calf_r->SetupAttachment(GetMesh(), FName("calf_r"));
	HitCollisionBoxes.Add(FName("calf_r"), calf_r);

	foot_l = CreateDefaultSubobject<UBoxComponent>(TEXT("foot_l"));
	foot_l->SetupAttachment(GetMesh(), FName("foot_l"));
	HitCollisionBoxes.Add(FName("foot_l"), foot_l);

	foot_r = CreateDefaultSubobject<UBoxComponent>(TEXT("foot_r"));
	foot_r->SetupAttachment(GetMesh(), FName("foot_r"));
	HitCollisionBoxes.Add(FName("foot_r"), foot_r);

	for (auto& Box: HitCollisionBoxes) {
		if (Box.Value) {
			Box.Value->SetCollisionObjectType(ECC_HitBox);
			Box.Value->SetCollisionResponseToAllChannels(ECR_Ignore);
			Box.Value->SetCollisionResponseToChannel(ECC_HitBox, ECR_Block);
			Box.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}

void AVortexCharacter::PostInitializeComponents() {
	Super::PostInitializeComponents();
	if (Combat) {
		Combat->Character = this;
		Combat->SetIsReplicated(true);
	}
	if (Buff) {
		Buff->Character = this;
		Buff->SetInitialSpeed(GetCharacterMovement()->MaxWalkSpeed, GetCharacterMovement()->MaxWalkSpeedCrouched);
		Buff->SetInitialJumpVelocity(GetCharacterMovement()->JumpZVelocity);
	}
	if (LagCompensation) {
		LagCompensation->Character = this;
		if (Controller) {
			LagCompensation->Controller = Cast<AVortexPlayerController>(Controller);
		}
	}
}

void AVortexCharacter::PlayFireMontage(bool bAiming) {
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr)
		return;
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && FireWeaponMontage) {
		AnimInstance->Montage_Play(FireWeaponMontage);
		FName SectionName;
		SectionName = bAiming ? FName("RifleAim") : FName("RifleHip");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void AVortexCharacter::PlayReloadMontage() {
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr)
		return;
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ReloadMontage) {
		AnimInstance->Montage_Play(ReloadMontage);
		FName SectionName = FName("Rifle");
		switch (Combat->EquippedWeapon->GetWeaponType()) {
		case EWeaponType::EWT_AssaultRifle:
			SectionName = FName("Rifle");
			break;
		case EWeaponType::EWT_RocketLauncher:
			SectionName = FName("RocketLauncher");
			break;
		case EWeaponType::EWT_Pistol:
			SectionName = FName("Pistol");
			break;
		case EWeaponType::EWT_SubmachineGun:
			SectionName = FName("Pistol");
			break;
		case EWeaponType::EWT_Shotgun:
			SectionName = FName("Shotgun");
			break;
		case EWeaponType::EWT_SniperRifle:
			SectionName = FName("SniperRifle");
			break;
		case EWeaponType::EWT_GrenadeLauncher:
			SectionName = FName("GrenadeLauncher");
			break;
		case EWeaponType::EWT_MAX:
			break;
		}
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void AVortexCharacter::PlayElimMontage() {
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ElimMontage) {
		AnimInstance->Montage_Play(ElimMontage);
	}
}

void AVortexCharacter::PlayThrowGrenadeMontage() {
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ThrowGrenadeMontage) {
		AnimInstance->Montage_Play(ThrowGrenadeMontage);
	}
}

void AVortexCharacter::PlaySwapMontage() {
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && SwapMontage) {
		AnimInstance->Montage_Play(SwapMontage);
	}
}

void AVortexCharacter::PlayHitReactMontage() {
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr)
		return;
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage) {
		AnimInstance->Montage_Play(HitReactMontage);
		FName SectionName("FromFront");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void AVortexCharacter::SpawnDefaultWeapon() {
	AVortexGameMode* VortexGameMode = Cast<AVortexGameMode>(UGameplayStatics::GetGameMode(this));
	UWorld* World = GetWorld();
	if (VortexGameMode && World && !bElimmed && DefaultWeaponClass) {
		AWeapon* StartingWeapon = World->SpawnActor<AWeapon>(DefaultWeaponClass);
		StartingWeapon->bDestroyWeapon = true;
		if (Combat) {
			Combat->EquipWeapon(StartingWeapon);
		}
	}
}

void AVortexCharacter::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(AVortexCharacter, OverlappingWeapon, COND_OwnerOnly);
	DOREPLIFETIME(AVortexCharacter, Health);
	DOREPLIFETIME(AVortexCharacter, bDisableGameplay);
	DOREPLIFETIME(AVortexCharacter, Shield);
}

void AVortexCharacter::OnRep_ReplicatedMovement() {
	Super::OnRep_ReplicatedMovement();
	SimProxiesTurn();
	TimeSinceLastMovementReplication = 0.f;
}

void AVortexCharacter::Elim(bool bPlayerLeftGame) {
	DropOrDestroyWeapons();
	MulticastElim(bPlayerLeftGame);
}

void AVortexCharacter::MulticastElim_Implementation(bool bPlayerLeftGame) {
	bLeftGame = bPlayerLeftGame;
	if (VortexPlayerController) {
		VortexPlayerController->SetHUDWeaponAmmo(0);
	}
	bElimmed = true;
	PlayElimMontage();
	//start dissolve
	if (DissolveMaterialInstance) {
		DynamicDissolveMaterialInstance = UMaterialInstanceDynamic::Create(DissolveMaterialInstance, this);
		GetMesh()->SetMaterial(0, DynamicDissolveMaterialInstance);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), 0.55f);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Glow"), 200.f);
	}
	StartDissolve();

	//disable character movement
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();
	bDisableGameplay = true;
	GetCharacterMovement()->DisableMovement();
	if (Combat) {
		Combat->FireButtonPressed(false);
	}
	
	//disable collision
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//spawn Elimbot
	if (ElimBotEffect) {
		FVector ElimBotSpawnLocation = GetActorLocation();
		ElimBotSpawnLocation.Z += 200.f;
		ElimBotComponent = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ElimBotEffect, ElimBotSpawnLocation,
		                                                            GetActorRotation());
	}
	if (ElimBotSound) {
		UGameplayStatics::SpawnSoundAtLocation(this, ElimBotSound, GetActorLocation());
	}

	bool bUseSniperRifle = IsLocallyControlled() && Combat && Combat->bAiming && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle;
	if (bUseSniperRifle) {
		ShowSniperScopeWidget(false);
	}
	if (CrownComponent) {
		CrownComponent->DestroyComponent();
	}
	GetWorldTimerManager().SetTimer(ElimTimer, this, &AVortexCharacter::ElimTimerFinished, ElimDelay);
}

void AVortexCharacter::ElimTimerFinished() {
	AVortexGameMode* VortexGameMode = GetWorld()->GetAuthGameMode<AVortexGameMode>();
	if (VortexGameMode && !bLeftGame) {
		VortexGameMode->RequestRespawn(this, Controller);
	}
	if (bLeftGame && IsLocallyControlled()) {
		OnLeftGame.Broadcast();
	}
}

void AVortexCharacter::ServerLeaveGame_Implementation() {
	AVortexGameMode* VortexGameMode = GetWorld()->GetAuthGameMode<AVortexGameMode>();
	VortexPlayerState = VortexPlayerState == nullptr ? GetPlayerState<AVortexPlayerState>() : VortexPlayerState; 
	if (VortexGameMode && VortexPlayerState) {
		VortexGameMode->PlayerLeftGame(VortexPlayerState);
	}
}

void AVortexCharacter::DropOrDestroyWeapon(AWeapon* Weapon) {
	if (Weapon == nullptr) return;
	if (Weapon->bDestroyWeapon) {
		Weapon->Destroy();
	}else {
		Weapon->Dropped();
	}
}

void AVortexCharacter::DropOrDestroyWeapons() {
	if (Combat) {
		if (Combat->EquippedWeapon) {
			DropOrDestroyWeapon(Combat->EquippedWeapon);
		}
		if (Combat->SecondaryWeapon) {
			DropOrDestroyWeapon(Combat->SecondaryWeapon);
		}
	}
}

void AVortexCharacter::Destroyed() {
	Super::Destroyed();
	if (ElimBotComponent) {
		ElimBotComponent->DestroyComponent();
	}
	AVortexGameMode* VortexGameMode = Cast<AVortexGameMode>(UGameplayStatics::GetGameMode(this));
	bool bMatchNotInProgress = VortexGameMode && VortexGameMode->GetMatchState() != MatchState::InProgress;
	if (Combat && Combat->EquippedWeapon && bMatchNotInProgress) {
		Combat->EquippedWeapon->Destroy();
	}
}

void AVortexCharacter::MulticastGainedTheLead_Implementation() {
	if (CrownSystem == nullptr) return;
	if (CrownComponent == nullptr) {
		CrownComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			CrownSystem,
			GetCapsuleComponent(),
			FName(),
			GetActorLocation() + FVector(0, 0, 110.f),
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition,
			false
		);
	}
	if (CrownComponent) {
		CrownComponent->Activate();
	}
}

void AVortexCharacter::MulticastLostTheLead_Implementation() {
	if (CrownComponent) {
		CrownComponent->DestroyComponent();
	}
}

void AVortexCharacter::BeginPlay() {
	Super::BeginPlay();
	
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController())) {
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<
			UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer())) {
			Subsystem->AddMappingContext(CharacterMappingContext, 0);
		}
	}
	
	SpawnDefaultWeapon();
	UpdateHUDAmmo();
	UpdateHUDHealth();
	UpdateHUDShield();

	if (HasAuthority()) {
		OnTakeAnyDamage.AddDynamic(this, &ThisClass::AVortexCharacter::ReceiveDamage);
	}
	if (AttachedGrenade) {
		AttachedGrenade->SetVisibility(false);
	}
	
}

void AVortexCharacter::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);

	RotateInPlace(DeltaTime);
	HideCameraIfCharacterClose();
	PollInit();
}

void AVortexCharacter::RotateInPlace(float DeltaTime) {
	if (bDisableGameplay) { return; }
	if (GetLocalRole() > ROLE_SimulatedProxy && IsLocallyControlled()) {
		bUseControllerRotationRoll = false;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		AimOffset(DeltaTime);
	}
	else {
		TimeSinceLastMovementReplication += DeltaTime;
		if (TimeSinceLastMovementReplication > 0.25f) {
			OnRep_ReplicatedMovement();
		}
		CalculateAO_Pitch();
	}
}

void AVortexCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) {
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AVortexCharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AVortexCharacter::Move);
		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AVortexCharacter::Look);
		// Equip
		EnhancedInputComponent->BindAction(EquipAction, ETriggerEvent::Started, this, &AVortexCharacter::Equip);
		// Crouch
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &AVortexCharacter::Crouching);
		// Aim
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Started, this, &AVortexCharacter::Aim);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &AVortexCharacter::Aim);
		// Fire
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &AVortexCharacter::Fire);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &AVortexCharacter::Fire);
		// Reload
		EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Started, this,
		                                   &AVortexCharacter::ReloadButtonPressed);
		//Throw Grenade
		EnhancedInputComponent->BindAction(GrenadeAction, ETriggerEvent::Started, this,&AVortexCharacter::GrenadeButtonPressed);
	}
	else {
		UE_LOG(LogVortexCharacter, Error,
		       TEXT(
			       "'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."
		       ), *GetNameSafe(this));
	}
}

void AVortexCharacter::Jump() {
	if (bDisableGameplay) return;
	if (bIsCrouched) {
		UnCrouch();
	}
	else {
		Super::Jump();
	}
}

void AVortexCharacter::Move(const FInputActionValue& Value) {
	UE_LOG(LogVortexCharacter, Error, TEXT("Move"));
	if (bDisableGameplay) return;
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr) {
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AVortexCharacter::Look(const FInputActionValue& Value) {
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr) {
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AVortexCharacter::Equip(const FInputActionValue& Value) {
	if (bDisableGameplay) return;
	bool bEquip = Value.Get<bool>();
	if (Combat) {
		if (Combat->CombatState == ECombatState::ECS_Unoccupied)
			ServerEquipButtonPressed();
		
		bool bSwap = Combat->ShouldSwapWeapons() &&
			!HasAuthority() &&
			Combat->CombatState == ECombatState::ECS_Unoccupied &&
			OverlappingWeapon == nullptr;
		if (bSwap) {
			PlaySwapMontage();
			Combat->CombatState = ECombatState::ECS_SwappingWeapons;
			bFinsihedSwapping = false;
		} 
		// if (HasAuthority()) {
		// 	if (OverlappingWeapon) {
		// 		Combat->EquipWeapon(OverlappingWeapon);
		// 	}else if (Combat->ShouldSwapWeapons()) {
		// 		Combat->SwapWeapons();
		// 		PlaySwapMontage();
		// 		Combat->CombatState = ECombatState::ECS_SwappingWeapons;
		// 	}
		// }
		// else {
		// 	ServerEquipButtonPressed();
		// }
	}
}

void AVortexCharacter::ServerEquipButtonPressed_Implementation() {
	if (Combat) {
		if (OverlappingWeapon) {
			Combat->EquipWeapon(OverlappingWeapon);
		}else if (Combat->ShouldSwapWeapons()) {
			Combat->SwapWeapons();
		}
	}
}

void AVortexCharacter::Crouching(const FInputActionValue& Value) {
	if (bDisableGameplay) return;
	if (bIsCrouched) {
		Super::UnCrouch();
	}
	else {
		Super::Crouch();
	}
}

void AVortexCharacter::ReloadButtonPressed(const FInputActionValue& Value) {
	if (bDisableGameplay) return;
	if (Combat) {
		Combat->Reload();
	}
}

void AVortexCharacter::Aim(const FInputActionValue& Value) {
	if (bDisableGameplay) return;
	bool bAim = Value.Get<bool>();
	if (Combat) {
		Combat->SetAiming(bAim);
	}
}

void AVortexCharacter::Fire(const FInputActionValue& Value) {
	if (bDisableGameplay) return;
	bool bFire = Value.Get<bool>();
	if (Combat) {
		Combat->FireButtonPressed(bFire);
	}
}

void AVortexCharacter::GrenadeButtonPressed(const FInputActionValue& Value) {
	if (Combat) {
		Combat->ThrowGrenade();
	}
}

void AVortexCharacter::CalculateAO_Pitch() {
	AO_Pitch = GetBaseAimRotation().Pitch;
	if (AO_Pitch > 90.f && !IsLocallyControlled()) {
		// map Pitch from [270, 360] to [-90, 0]
		FVector2D InRange(270.f, 360.f);
		FVector2D OutRange(-90.f, 0.f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}
}

void AVortexCharacter::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
                                     AController* InstigatedController, AActor* DamageCauser) {
	if (bElimmed) return;

	float DamageToHealth = Damage;
	if (Shield > 0.f) {
		if (Shield >= Damage) {
			Shield = FMath::Clamp(Shield - Damage, 0.f, MaxShield);
			DamageToHealth = 0.f;
		}else {
			DamageToHealth = FMath::Clamp(DamageToHealth - Shield, 0.f, Damage);
			Shield = 0.f;
		}
	}
	Health = FMath::Clamp(Health - DamageToHealth, 0.0f, MaxHealth);
	UpdateHUDHealth();
	UpdateHUDShield();
	PlayHitReactMontage();
	if (Health == 0.0f) {
		AVortexGameMode* VortexGameMode = GetWorld()->GetAuthGameMode<AVortexGameMode>();
		if (VortexGameMode) {
			VortexPlayerController = VortexPlayerController == nullptr
				                         ? Cast<AVortexPlayerController>(Controller)
				                         : VortexPlayerController;
			AVortexPlayerController* AttackerController = Cast<AVortexPlayerController>(InstigatedController);
			VortexGameMode->PlayerEliminated(this, VortexPlayerController, AttackerController);
		}
	}
}

float AVortexCharacter::CalculateSpeed() {
	FVector Veocity = GetVelocity();
	Veocity.Z = 0.f;
	return Veocity.Size();
}

void AVortexCharacter::AimOffset(float DeltaTime) {
	if (Combat && Combat->EquippedWeapon == nullptr) {
		return;
	}
	float Speed = CalculateSpeed();
	bool bIsInAir = GetCharacterMovement()->IsFalling();

	if (Speed == 0.f && !bIsInAir) {
		bRotateRootBone = true;
		FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation);
		AO_Yaw = DeltaAimRotation.Yaw;
		if (TurningInPlace == ETurningInPlace::ETIP_NotTurning) {
			InterpAO_Yaw = AO_Yaw;
		}
		bUseControllerRotationYaw = true;
		TurnInPlace(DeltaTime);
	}
	if (Speed > 0.f || bIsInAir) {
		bRotateRootBone = false;
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		AO_Yaw = 0.f;
		bUseControllerRotationYaw = true;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}

	CalculateAO_Pitch();
}

void AVortexCharacter::SimProxiesTurn() {
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) { return; }
	bRotateRootBone = false;
	float Speed = CalculateSpeed();
	if (Speed > 0.f) {
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}
	ProxyRotationLastFrame = ProxyRotation;
	ProxyRotation = GetActorRotation();
	ProxyYaw = UKismetMathLibrary::NormalizedDeltaRotator(ProxyRotation, ProxyRotationLastFrame).Yaw;

	if (FMath::Abs(ProxyYaw) > TurnThreshold) {
		if (ProxyYaw > TurnThreshold) {
			TurningInPlace = ETurningInPlace::ETIP_Right;
		}
		else if (ProxyYaw < -TurnThreshold) {
			TurningInPlace = ETurningInPlace::ETIP_Left;
		}
		else {
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		}
		return;
	}
	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
}

void AVortexCharacter::TurnInPlace(float DeltaTime) {
	if (AO_Yaw > 90.f) {
		TurningInPlace = ETurningInPlace::ETIP_Right;
	}
	else if (AO_Yaw < -90.f) {
		TurningInPlace = ETurningInPlace::ETIP_Left;
	}
	if (TurningInPlace != ETurningInPlace::ETIP_NotTurning) {
		InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, 0.f, DeltaTime, 5.f);
		AO_Yaw = InterpAO_Yaw;
		if (FMath::Abs(AO_Yaw) < 15.f) {
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
			StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		}
	}
}

void AVortexCharacter::HideCameraIfCharacterClose() {
	if (!IsLocallyControlled()) {
		return;
	}
	if ((FollowCamera->GetComponentLocation() - GetActorLocation()).Size() < CameraThreshold) {
		GetMesh()->SetVisibility(false);
		if (Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh()) {
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}
	}
	else {
		GetMesh()->SetVisibility(true);
		if (Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh()) {
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
	}
}

void AVortexCharacter::OnRep_Health(float LastHealth) {
	UpdateHUDHealth();
	if (Health < LastHealth) {
		PlayHitReactMontage();
	}
}

void AVortexCharacter::OnRep_Shield(float LastShield) {
	UpdateHUDShield();
	if (Shield < LastShield) {
		PlayHitReactMontage();
	}
}

void AVortexCharacter::UpdateHUDHealth() {
	VortexPlayerController = VortexPlayerController == nullptr
		                         ? Cast<AVortexPlayerController>(Controller)
		                         : VortexPlayerController;
	if (VortexPlayerController) {
		VortexPlayerController->SetHUDHealth(Health, MaxHealth);
	}
}

void AVortexCharacter::UpdateHUDShield() {
	VortexPlayerController = VortexPlayerController == nullptr
							 ? Cast<AVortexPlayerController>(Controller)
							 : VortexPlayerController;
	if (VortexPlayerController) {
		VortexPlayerController->SetHUDShield(Shield, MaxShield);
	}
}

void AVortexCharacter::UpdateHUDAmmo() {
	VortexPlayerController = VortexPlayerController == nullptr
						 ? Cast<AVortexPlayerController>(Controller)
						 : VortexPlayerController;
	if (VortexPlayerController && Combat && Combat->EquippedWeapon) {
		VortexPlayerController->SetHUDCarriedAmmo(Combat->CarriedAmmo);
		VortexPlayerController->SetHUDWeaponAmmo(Combat->EquippedWeapon->GetAmmo());
	}
}

void AVortexCharacter::PollInit() {
	if (VortexPlayerState == nullptr) {
		VortexPlayerState = GetPlayerState<AVortexPlayerState>();
		if (VortexPlayerState) {
			VortexPlayerState->AddToScore(0.f);
			VortexPlayerState->AddToDefeats(0);
		}
		AVortexGameState* VortexGameState = Cast<AVortexGameState>(UGameplayStatics::GetGameState(this));
		if (VortexGameState && VortexGameState->TopScoringPlayers.Contains(VortexPlayerState)) {
			MulticastGainedTheLead();
		}
	}
}

void AVortexCharacter::UpdateDissolveMaterial(float DissolveValue) {
	if (DynamicDissolveMaterialInstance) {
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
	}
}

void AVortexCharacter::StartDissolve() {
	DissolveTrack.BindDynamic(this, &AVortexCharacter::UpdateDissolveMaterial);
	if (DissolveCurve && DissolveTimeline) {
		DissolveTimeline->AddInterpFloat(DissolveCurve, DissolveTrack);
		DissolveTimeline->Play();
	}
}

void AVortexCharacter::SetOverlappingWeapon(AWeapon* Weapon) {
	if (OverlappingWeapon) {
		OverlappingWeapon->ShowPickupWidget(false);
	}

	OverlappingWeapon = Weapon;
	if (IsLocallyControlled()) {
		if (OverlappingWeapon) {
			OverlappingWeapon->ShowPickupWidget(true);
		}
	}
}

void AVortexCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon) {
	if (OverlappingWeapon) {
		OverlappingWeapon->ShowPickupWidget(true);
	}
	if (LastWeapon) {
		LastWeapon->ShowPickupWidget(false);
	}
}

bool AVortexCharacter::IsWeaponEquipped() {
	return (Combat && Combat->EquippedWeapon);
}

bool AVortexCharacter::IsAiming() {
	return (Combat && Combat->bAiming);
}

AWeapon* AVortexCharacter::GetEquippedWeapon() {
	if (Combat == nullptr) {
		return nullptr;
	}
	return Combat->EquippedWeapon;
}

FVector AVortexCharacter::GetHitTarget() const {
	if (Combat == nullptr) return FVector();
	return Combat->HitTarget;
}

bool AVortexCharacter::IsLocallyReloading() {
	if (Combat == nullptr) return false;
	return Combat->bLocallyReloading;
}

ECombatState AVortexCharacter::GetCombatState() const {
	if (Combat == nullptr) return ECombatState::ECS_Max;
	return Combat->CombatState;
}
