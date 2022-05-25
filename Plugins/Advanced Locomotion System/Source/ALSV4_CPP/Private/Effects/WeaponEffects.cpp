// Fill out your copyright notice in the Description page of Project Settings.


#include "Effects/WeaponEffects.h"

#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "PhysicalMaterials/PhysicalMaterial.h"

// Surfaces types
#define SURFACE_IMPACT		SurfaceType1
#define SURFACE_METAL		SurfaceType2
#define SURFACE_WOOD		SurfaceType3
#define SURFACE_BODY		SurfaceType4

// Sets default values
AWeaponEffects::AWeaponEffects()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SetAutoDestroyWhenFinished(true);
}

// Called when the game starts or when spawned
void AWeaponEffects::BeginPlay()
{
	Super::BeginPlay();
}

UParticleSystem* AWeaponEffects::GetImpactFX(TEnumAsByte<EPhysicalSurface> SurfaceType) const
{
	UParticleSystem* HitFX = nullptr;

	// Add here another surface FXs
	switch (SurfaceType)
	{
	case SURFACE_METAL:		HitFX = MetalFX;
		break;
	case SURFACE_WOOD:		HitFX = WoodFX;
		break;
	case SURFACE_BODY:		HitFX = BodyFX;
		break;
	default:				HitFX = DefaultFX;
		break;
	}

	return HitFX;
}

USoundCue* AWeaponEffects::GetImpactSound(TEnumAsByte<EPhysicalSurface> SurfaceType) const
{
	USoundCue* SoundFX = nullptr;

	// Add here another surface sounds
	switch (SurfaceType)
	{
	case SURFACE_IMPACT:	SoundFX = ImpactSurfaceSound;
		break;
	case SURFACE_BODY:		SoundFX = ImpactBodySound;
		break;
	default:				SoundFX = DefaultSound;
		break;
	}

	return SoundFX;
}

// Called every frame
void AWeaponEffects::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AWeaponEffects::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	UPhysicalMaterial* HitPhysicalMaterial = SurfaceHit.PhysMaterial.Get();
	EPhysicalSurface HitSurfaceType = UPhysicalMaterial::DetermineSurfaceType(HitPhysicalMaterial);

	// Show impact particles and play impact sound
	UParticleSystem* HitFX = GetImpactFX(HitSurfaceType);
	USoundCue* SoundFX = GetImpactSound(HitSurfaceType);
	if (HitFX && SoundFX)
	{
		UGameplayStatics::SpawnEmitterAtLocation(this, HitFX, GetActorLocation(), GetActorRotation());
		UGameplayStatics::PlaySoundAtLocation(this, SoundFX, GetActorLocation(), GetActorRotation());
	}
	
}
