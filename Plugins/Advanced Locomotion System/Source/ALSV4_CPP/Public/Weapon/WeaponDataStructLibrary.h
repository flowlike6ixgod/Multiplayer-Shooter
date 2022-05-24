#pragma once

#include "WeaponDataStructLibrary.generated.h"

USTRUCT()
struct FWeaponData
{
	GENERATED_BODY()

	/* Ammo */
	UPROPERTY(EditDefaultsOnly, Category = Ammo)
	int32 MaxClipAmmo;

	UPROPERTY(EditDefaultsOnly, Category = Ammo)
	int32 MaxWeaponAmmo;

	/* Weapon name*/
	UPROPERTY(EditDefaultsOnly, Category = Base)
	FString WeaponName;

	UPROPERTY(EditDefaultsOnly, Category = Base)
	float DelayBetweenShots;

	/* Base damage */
	UPROPERTY(EditDefaultsOnly, Category = Stats)
	int32 WeaponBaseDamage;

	/* Weapon spread in degrees*/
	UPROPERTY(EditDefaultsOnly, Category = Accuracy)
	float WeaponSpread;

	/* Spread modifier */
	UPROPERTY(EditDefaultsOnly, Category = Accuracy)
	float SpreadModifier;

	UPROPERTY(EditDefaultsOnly, Category = Accuracy)
	float SpreadReduction;

	/* Max spread modifier */
	UPROPERTY(EditDefaultsOnly, Category = Accuracy)
	float MaxSpreadModifier;

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
		WeaponSpread = 3.0f;
		SpreadModifier = 1.0f;
		SpreadReduction = 0.0f;
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

	UPROPERTY(EditAnywhere, Category = Weapon)
	float Spread;

	UPROPERTY()
	FVector Source;
	
	UPROPERTY(EditAnywhere, Category = Weapon)
	int32 RandomSeed;
	
	FWeaponSpread() : Spread(0), Source(0), RandomSeed(0)
	{
	}; 
};
