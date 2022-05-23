#pragma once

#include "WeaponDataStructLibrary.generated.h"

USTRUCT()
struct FWeaponData
{
	GENERATED_BODY()

	/* Ammo */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon | Ammo")
	int32 MaxClipAmmo;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon | Ammo")
	int32 MaxWeaponAmmo;

	/* Weapon name*/
	UPROPERTY(EditDefaultsOnly, Category = "Weapon | Base")
	FString WeaponName;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon | Base")
	float DelayBetweenShots;

	/* Base damage */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon | Stats")
	int32 WeaponBaseDamage;

	/* Weapon spread in degrees*/
	UPROPERTY(EditDefaultsOnly, Category = "Weapon | Accuracy")
	float WeaponSpread;

	/* Spread modifier */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon | Accuracy")
	float SpreadModifier;

	/* Max spread modifier */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon | Accuracy")
	float MaxSpreadModifier;

	/* Weapon range */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon | Stats")
	float WeaponRange;

	/* Damage amount */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon | Stats")
	float HitDamage;

	/* Type of damage */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon | Stats")
	TSubclassOf<UDamageType> DamageType;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon | Base")
	float ClientSideHitLeeway;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon | Base")
	float AllowedViewDotHitDir;
	
	FWeaponData()
	{
		WeaponName = "Weapon Unknown";
		MaxClipAmmo = 255;
		MaxWeaponAmmo = 255;
		DelayBetweenShots = 0.5f;
		WeaponBaseDamage = 30;
		WeaponSpread = 3.0f;
		SpreadModifier = 1.0f;
		MaxSpreadModifier = 10.0f;
		WeaponRange = 10000.0f;
		HitDamage = 30.0f;
		DamageType = UDamageType::StaticClass();
		ClientSideHitLeeway = 200.0f;
		AllowedViewDotHitDir = 0.8f;
	}
};

USTRUCT()
struct FWeaponSpread
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Weapon")
	float Spread;

	UPROPERTY()
	FVector Source;
	
	UPROPERTY(EditAnywhere, Category = "Weapon")
	int32 RandomSeed;
	
	FWeaponSpread() : Spread(0), Source(0), RandomSeed(0)
	{
	}; 
};
