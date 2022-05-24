// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/WeaponBase.h"
#include "WeaponRocketLauncher.generated.h"


USTRUCT()
struct FWeaponProjectileData
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "Projectile | Data")
	TSubclassOf<class AWeaponProjectile> WeaponProjectile;

	UPROPERTY(EditDefaultsOnly, Category = "Projectile | Data")
	float ProjectileLifeTime;

	UPROPERTY(EditDefaultsOnly, Category = "Projectile | Data")
	TSubclassOf<UDamageType> DamageType;

	UPROPERTY(EditDefaultsOnly, Category = "Projectile | Data")
	int32 ProjectileDamage;

	UPROPERTY(EditDefaultsOnly, Category = "Projectile | Data")
	float ProjectileRadius;

	FWeaponProjectileData()
	{
		WeaponProjectile = nullptr;
		ProjectileLifeTime = 5.f;
		DamageType = UDamageType::StaticClass();
		ProjectileDamage = 50;
		ProjectileRadius = 20.f;
	}
};


/**
 * 
 */
UCLASS(Abstract)
class ALSV4_CPP_API AWeaponRocketLauncher : public AWeaponBase
{
	GENERATED_BODY()

public:
	/* Apply projectile data*/
	void ApplyWeaponProjectileData(struct FWeaponProjectileData& Data);

	/* Return current ammo type */
	virtual EAmmoType GetAmmoType() const override;

	UPROPERTY(EditDefaultsOnly, Category = "Data")
	FWeaponProjectileData ProjectileData;
	
	virtual void Fire() override;

	/* Server spawn projectile */
	UFUNCTION(Reliable, Server, WithValidation)
	void ServerFireProjectile(FVector Source, FVector_NetQuantizeNormal ShootDirection);
};
