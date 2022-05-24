// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NiagaraComponentPool.h"
#include "GameFramework/Actor.h"
#include "Sound/SoundCue.h"
#include "WeaponEffects.generated.h"

UCLASS()
class ALSV4_CPP_API AWeaponEffects : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeaponEffects();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

protected:
	// Impact FX 
	//
	UPROPERTY(EditAnywhere, Category = Effects)
	UParticleSystem* DefaultFX;

	/* Metal impact FX */
	UPROPERTY(EditAnywhere, Category = Effects)
	UParticleSystem* MetalFX;

	/* Wood impact FX */
	UPROPERTY(EditAnywhere, Category = Effects)
	UParticleSystem* WoodFX;

	UPROPERTY(EditAnywhere, Category = Effects)
	UParticleSystem* BodyFX;

	// Sound
	//
	UPROPERTY(EditAnywhere, Category = Sound)
	USoundCue* DefaultSound;

	/* Impact on Surface */
	UPROPERTY(EditAnywhere, Category = Sound)
	USoundCue* ImpactSurfaceSound;

	/* Impact on body */
	UPROPERTY(EditAnywhere, Category = Sound)
	USoundCue* ImpactBodySound;

	UNiagaraEmitter* Trail;

public:
	/* Result of surface hit */
	FHitResult SurfaceHit;
	
	/* Get impact FX */
	UParticleSystem* GetImpactFX(TEnumAsByte<EPhysicalSurface> SurfaceType) const;

	/* Get impact sound */
	USoundCue* GetImpactSound(TEnumAsByte<EPhysicalSurface> SurfaceType) const;
	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void PostInitializeComponents() override;

};
