// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponRocketLauncher.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "WeaponProjectile.generated.h"

UCLASS(Abstract, Blueprintable)
class ALSV4_CPP_API AWeaponProjectile : public AActor
{
	GENERATED_BODY()

private:
	/* Projectile movement component */
	UPROPERTY(VisibleAnywhere, Category = "Projectile")
	UProjectileMovementComponent* ProjectileMovementComponent;

	UPROPERTY(VisibleAnywhere, Category = "Projectile")
	USphereComponent* ProjectileSphereComponent;

	UPROPERTY(VisibleAnywhere, Category = "Projectile")
	UParticleSystemComponent* ProjectileParticleSystemComponent;
	
public:	
	// Sets default values for this actor's properties
	AWeaponProjectile();
	
protected:

	/* Fired controller */
	TWeakObjectPtr<AController> PC;

	UPROPERTY(Transient, ReplicatedUsing = OnRep_Exploded)
	bool bIsExploded;

	/* OnRep bIsExplode (client) */
	UFUNCTION()
	void OnRep_Exploded();

	void Explode(const FHitResult& HitResult);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void PostInitializeComponents() override;

	virtual void PostNetReceiveVelocity(const FVector& Velocity) override;

	virtual void GetLifetimeReplicatedProps( TArray< FLifetimeProperty > & OutLifetimeProps ) const override;

public:
	/** projectile data */
	struct FWeaponProjectileData ProjectileData;
	
	/* Initialize projectile velocity */
	void InitializeVelocity(FVector& ShootDirection);

	/* Handle hit */
	UFUNCTION()
	void OnHit(const FHitResult& HitResult);

	/* Destroy fired projectile */
	void DestroyProjectile();
};
