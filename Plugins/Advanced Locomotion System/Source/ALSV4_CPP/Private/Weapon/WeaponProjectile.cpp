// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/WeaponProjectile.h"

#include "Components/AudioComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"

// Sets default values
AWeaponProjectile::AWeaponProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;
	SetRemoteRoleForBackwardsCompat(ROLE_SimulatedProxy);
	bReplicates = true;
	SetReplicatingMovement(true);

	ProjectileSphereComponent = CreateDefaultSubobject<USphereComponent>("CollisionComponent");
	ProjectileSphereComponent->InitSphereRadius(3.0f);
	ProjectileSphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ProjectileSphereComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	ProjectileSphereComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	ProjectileSphereComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	ProjectileSphereComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	ProjectileSphereComponent->SetCollisionObjectType(ECC_Visibility);
	ProjectileSphereComponent->AlwaysLoadOnClient = true;
	ProjectileSphereComponent->AlwaysLoadOnServer = true;
	ProjectileSphereComponent->CastShadow = false;
	RootComponent = ProjectileSphereComponent;
	
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("MovementComponent"));
	ProjectileMovementComponent->bShouldBounce = false;
	ProjectileMovementComponent->InitialSpeed = 3000.0f;
	ProjectileMovementComponent->MaxSpeed = 3000.0f;
	ProjectileMovementComponent->UpdatedComponent = ProjectileSphereComponent;
	ProjectileMovementComponent->ProjectileGravityScale = 0.1f;

	ProjectileParticleSystemComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ParticleSystem"));
	ProjectileParticleSystemComponent->bAutoDestroy = false;
	ProjectileParticleSystemComponent->bAutoActivate = false;
	ProjectileParticleSystemComponent->SetupAttachment(ProjectileSphereComponent);
}

void AWeaponProjectile::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	ProjectileMovementComponent->OnProjectileStop.AddDynamic(this, &AWeaponProjectile::OnHit);
	ProjectileSphereComponent->MoveIgnoreActors.Add(GetInstigator());

	AWeaponRocketLauncher* NewOwner = Cast<AWeaponRocketLauncher>(GetOwner());
	if (NewOwner)
	{
		NewOwner->ApplyWeaponProjectileData(ProjectileData);
	}
	
	SetLifeSpan(ProjectileData.ProjectileLifeTime);
	PC = GetInstigatorController();
}

void AWeaponProjectile::OnRep_Exploded()
{
	FVector ProjectileDirection = GetActorForwardVector();

	const FVector StartTrace = GetActorLocation() - ProjectileDirection * 150;
	const FVector EndTrace = GetActorLocation() + ProjectileDirection * 200;
	FHitResult Hit;

	DrawDebugLine(GetWorld(), StartTrace, EndTrace, FColor::Red, false, 3.f);

	if (!GetWorld()->LineTraceSingleByChannel(Hit, StartTrace, EndTrace, ECC_Visibility, FCollisionQueryParams(SCENE_QUERY_STAT(Projectile), true, GetInstigator())))
	{
		Hit.ImpactPoint = GetActorLocation();
		Hit.ImpactNormal = -ProjectileDirection;
	}

	Explode(Hit);
}

void AWeaponProjectile::Explode(const FHitResult& HitResult)
{
	if (ProjectileParticleSystemComponent)
	{
		ProjectileParticleSystemComponent->Deactivate();
	}

	const FVector HitLocation = HitResult.ImpactPoint + HitResult.ImpactNormal * 10.0f;

	if (ProjectileData.ProjectileDamage > 0.0f && ProjectileData.ProjectileRadius > 0.0f && ProjectileData.DamageType)
	{
		UGameplayStatics::ApplyRadialDamage(this, ProjectileData.ProjectileDamage, HitLocation, ProjectileData.ProjectileRadius, ProjectileData.DamageType, TArray<AActor*>{}, this, PC.Get());
	}

	bIsExploded = true;
}

void AWeaponProjectile::PostNetReceiveVelocity(const FVector& Velocity)
{
	Super::PostNetReceiveVelocity(Velocity);
}

void AWeaponProjectile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeaponProjectile, bIsExploded);
}

void AWeaponProjectile::InitializeVelocity(FVector& ShootDirection)
{
	if (ProjectileMovementComponent)
	{
		ProjectileMovementComponent->Velocity = ShootDirection * ProjectileMovementComponent->InitialSpeed;
	}
}

void AWeaponProjectile::OnHit(const FHitResult& HitResult)
{
	if (GetLocalRole() == ROLE_Authority && !bIsExploded)
	{
		Explode(HitResult);
		DestroyProjectile();
	}
}

void AWeaponProjectile::DestroyProjectile()
{
	UAudioComponent* AudioComponent = FindComponentByClass<UAudioComponent>();

	if (AudioComponent && AudioComponent->IsPlaying())
	{
		AudioComponent->FadeOut(0.1f, 0.0f);
	}

	ProjectileMovementComponent->StopMovementImmediately();
	SetLifeSpan(1.0f);
}

// Called every frame
void AWeaponProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

