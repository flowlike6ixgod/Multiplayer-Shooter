#pragma once

#include "WeaponDataStructLibrary.generated.h"

USTRUCT()
struct FWeaponData
{
	GENERATED_BODY()

	/* Weapon name*/
	UPROPERTY(EditDefaultsOnly, Category = Base)
	FString WeaponName;
	
	/* Ammo */
	UPROPERTY(EditDefaultsOnly, Category = Ammo)
	int32 MaxClipAmmo;

	UPROPERTY(EditDefaultsOnly, Category = Ammo)
	int32 MaxWeaponAmmo;

	UPROPERTY(EditDefaultsOnly, Category = Base)
	float DelayBetweenShots;

	/* Base damage */
	UPROPERTY(EditDefaultsOnly, Category = Stats)
	int32 WeaponBaseDamage;

	/* Weapon range */
	UPROPERTY(EditDefaultsOnly, Category = Stats)
	float WeaponRange;

	/* Damage amount */
	UPROPERTY(EditDefaultsOnly, Category = Stats)
	float HitDamage;

	/* Type of damage */
	UPROPERTY(EditDefaultsOnly, Category = Stats)
	TSubclassOf<UDamageType> DamageType;

	UPROPERTY(EditDefaultsOnly, Category = Base)
	float ClientSideHitLeeway;

	UPROPERTY(EditDefaultsOnly, Category = Base)
	float AllowedViewDotHitDir;
	
	FWeaponData()
	{
		WeaponName = "Weapon Unknown";
		MaxClipAmmo = 255;
		MaxWeaponAmmo = 255;
		DelayBetweenShots = 0.5f;
		WeaponBaseDamage = 30;
		WeaponRange = 10000.0f;
		HitDamage = 30.0f;
		DamageType = UDamageType::StaticClass();
		ClientSideHitLeeway = 200.0f;
		AllowedViewDotHitDir = 0.8f;
	}
};

USTRUCT()
struct FSpreadData
{
	GENERATED_BODY()

	/* Weapon spread in degrees*/
	UPROPERTY(EditDefaultsOnly, Category = Accuracy)
	float WeaponSpread;

	/* Spread modifier */
	UPROPERTY(EditDefaultsOnly, Category = Accuracy)
	float SpreadModifier;

	/* Max spread modifier */
	UPROPERTY(EditDefaultsOnly, Category = Accuracy)
	float MaxSpreadModifier;

	UPROPERTY(EditDefaultsOnly, Category = Accuracy)
	float SpreadReduction;

	FSpreadData()
	{
		WeaponSpread = 3.0f;
		SpreadModifier = 1.0f;
		MaxSpreadModifier = 10.0f;
		SpreadReduction = 15.0f;
	}
	
};

USTRUCT()
struct FWeaponSpread
{
	GENERATED_BODY()

	UPROPERTY()
	float Spread;

	UPROPERTY()
	FVector Source;
	
	UPROPERTY()
	int32 RandomSeed;
	
	FWeaponSpread() : Spread(0), Source(0), RandomSeed(0)
	{
	}; 
};
