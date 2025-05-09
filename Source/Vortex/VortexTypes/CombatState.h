﻿#pragma once

UENUM(BlueprintType)
enum class ECombatState: uint8 {
	ECS_Unoccupied UMETA(DisplayName = "Unoccupied"),
	ECS_Reloading UMETA(DisplayName = "Reloading"),
	ECS_ThrowingGrenade UMETA(DisplayName = "ThrowingGrenade"),
	ECS_SwappingWeapons UMETA(DisplayName = "Swapping Weapons"),
	ECS_Max UMETA(DisplayName = "DefaultMax"),
};