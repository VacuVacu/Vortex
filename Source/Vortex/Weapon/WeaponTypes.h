#pragma once

UENUM(BlueprintType)
enum class EWeaponType: uint8 {
	EWT_AssaultRifle UMETA(DisplayName = "AssualtRifle"),
	EWT_RocketLauncher UMETA(DisplayName = "RocketLauncher"),

	EWT_MAX UMETA(DisplayName = "DefaultMax"),
};