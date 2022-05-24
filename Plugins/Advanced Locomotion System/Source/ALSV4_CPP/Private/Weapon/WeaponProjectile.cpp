// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/WeaponProjectile.h"

#include "Kismet/GameplayStatics.h"
#include "Weapon/WeaponRocketLauncher.h"

void AWeaponProjectile::ApplyWeaponProjectileData(FWeaponProjectileData& Data)
{
	Data = ProjectileData;
}

EAmmoType AWeaponProjectile::GetAmmoType() const
{
	return EAmmoType::Bullet;
}

void AWeaponProjectile::Fire()
{
	Super::Fire();

	UE_LOG(LogTemp, Warning, TEXT("WeaponBaseProjectileData Fire()"));
	FVector ShootDirection = GetAdjustedAim();
	FVector Source = GetMuzzleLocation();

	const float ProjectileRange = 5000.f;
	FVector StartTrace = GetCameraDamageStartLocation(ShootDirection);
	FVector EndTrace = ShootDirection * Source * ProjectileRange;
	FHitResult Hit = WeaponHitTrace(StartTrace, EndTrace);

	if (Hit.bBlockingHit)
	{
		const FVector AdjustedDirection = (Hit.ImpactPoint - Source).GetSafeNormal();
		bool bWeaponHit = false;

		const float DirectionDot = FVector::DotProduct(AdjustedDirection, ShootDirection);
		if (DirectionDot < 0.0f)
		{
			bWeaponHit = true;
		}
		else if (DirectionDot < 0.5f)
		{
			FVector MuzzleStartTrace = Source - GetMuzzleLocation() * 150.f;
			FVector MuzzleEndTrace = Source;
			FHitResult MuzzleHit = WeaponHitTrace(MuzzleStartTrace, MuzzleEndTrace);

			if (MuzzleHit.bBlockingHit)
			{
				bWeaponHit = true;
			}
		}

		if (bWeaponHit)
		{
			Source = Hit.ImpactPoint - ShootDirection * 10.f;
		}
		else
		{
			ShootDirection = AdjustedDirection;
		}
	}
	DrawDebugLine(GetWorld(), StartTrace, EndTrace, FColor::Yellow, false, 3.f);

	ServerFireProjectile(Source, ShootDirection);
}

void AWeaponProjectile::ServerFireProjectile_Implementation(FVector Source,
                                                                    FVector_NetQuantizeNormal ShootDirection)
{
	FTransform SpawnTransform(ShootDirection.Rotation(), Source);
	AWeaponRocketLauncher* Projectile = Cast<AWeaponRocketLauncher>(
		UGameplayStatics::BeginDeferredActorSpawnFromClass(this, ProjectileData.WeaponProjectile, SpawnTransform));

	if (Projectile)
	{
		Projectile->SetInstigator(GetInstigator());
		Projectile->SetOwner(this);
		Projectile->InitializeVelocity(ShootDirection);

		UGameplayStatics::FinishSpawningActor(this, SpawnTransform);
	}
}

bool AWeaponProjectile::ServerFireProjectile_Validate(FVector Source, FVector_NetQuantizeNormal ShootDirection)
{
	return true;
}
