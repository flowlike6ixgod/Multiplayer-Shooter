// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/WeaponBase.h"
#include "Weapon/WeaponDataStructLibrary.h"
#include "Weapon.generated.h"

//TODO:
//1. Create hit effect
//


/**
 * 
 */
UCLASS(Abstract)
class ALSV4_CPP_API AWeapon : public AWeaponBase
{
	GENERATED_BODY()

public:

	AWeapon();
	
	/* Return current weapon spread */
	float GetCurrentSpread() const;

	/* Return ammo type */
	virtual EAmmoType GetAmmoType() const override;

protected:
	/* Weapon data */
	FWeaponData WeaponBaseData;

	/* Gun fire smoke */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon | Effects")
	UParticleSystem* TrailFX;
	
	UPROPERTY(EditDefaultsOnly, Category = "Weapon | Effects")
	FName TrailTargetParam;

	/* replicated hit notify */
	UPROPERTY(Transient, ReplicatedUsing = OnRep_HitNotify)
	FWeaponSpread WeaponSpread;

	/* Current firing spread */
	float CurrentFiringSpread;


public:
	/* Server notified of hit */
	UFUNCTION(Reliable, Server, WithValidation)
	void ServerHitNotify(const FHitResult& Hit, FVector_NetQuantizeNormal ShootDirection, int32 RandSeed, float Spread);

	/* Server notified of miss and show trail FX */
	UFUNCTION(Unreliable, Server, WithValidation)
	void ServerMissNotify(FVector_NetQuantizeNormal ShootDirection, int32 RandSeed, float Spread);

	/* process the instant hit and notify the server if necessary */
	void ProcessHit(const FHitResult& HitResult, const FVector& Source, const FVector& ShootDirection, int32 RandSeed, float Spread);

	/* continue processing the instant hit, as if it has been confirmed by the server */
	void HitConfirmed(const FHitResult& HitResult, const FVector& Source, const FVector& ShootDirection, int32 RandSeed, float Spread);

	/* Check if weapon should deal damage to ohter actors */
	bool ShouldDealDamage(AActor* Actor) const;

	/* Handle deal damage */
	void DealDamage(const FHitResult& Hit, const FVector& ShootDirection);

	/* Weapon fire */
	virtual void Fire() override;

	/* Update spread */
	virtual void OnBurstFinished() override;

	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;

public:
	/* Hit notify replication */
	UFUNCTION()
	void OnRep_HitNotify();

	void SimulateHit(const FVector& Source, int32 RandSeed, float Spread);

	void SpawnHitEffect(const FHitResult& Hit);
	
	void SpawnTrailEffect(const FVector& EndPoint);
	
};
