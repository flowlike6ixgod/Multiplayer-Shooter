// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/WeaponDataStructLibrary.h"
#include "Character/ALSPlayerCameraManager.h"
#include "GameFramework/Actor.h"
#include "Library/ALSCharacterEnumLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Sound/SoundCue.h"
#include "WeaponBase.generated.h"

UCLASS(Abstract, Blueprintable)
class ALSV4_CPP_API AWeaponBase : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AWeaponBase();

public:
	UPROPERTY(EditAnywhere, Category = "Weapon | Base")
	TObjectPtr<USkeletalMeshComponent> WeaponSkeletalMeshComp;

public:
	//////////////* Weapon Getters/Setters *//////////////////

	UFUNCTION(BlueprintCallable)
	int32 GetMaxClipAmmo() const;

	UFUNCTION(BlueprintCallable)
	int32 GetCurrentClipAmmo() const;

	UFUNCTION(BlueprintCallable)
	int32 GetMaxWeaponAmmo() const;

	UFUNCTION(BlueprintCallable)
	int32 GetCurrentWeaponAmmo() const;

	UFUNCTION(BlueprintCallable)
	FName GetWeaponName() const;

	UFUNCTION(BlueprintCallable)
	EWeaponState GetCurrentWeaponState() { return CurrentWeaponState; };

	UFUNCTION(BlueprintCallable)
	class AALSBaseCharacter* GetOwnerCharacter() const;

	/* Set the weapon owner */
	void SetWeaponOwner(AALSBaseCharacter *WeaponOwner);

	/* Checks if player can start fire */
	bool CanFire();
	
public:
	/////////////////* Actions *//////////////

	virtual void StartFire();

	virtual void StopFire();

	/* StartReload() and StopReload() */

	protected:
	////////////////* Base info *////////////////
	///
	/* Owner*/
	UPROPERTY(Transient, ReplicatedUsing = OnRep_CharacterBase)
	AALSBaseCharacter* CharacterBase;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon | Base")
	FWeaponData WeaponData;

	/* Current weapon clip ammo */
	UPROPERTY(Transient, Replicated)
	uint32 CurrentClipAmmo;

	/* Current total weapon ammo */
	UPROPERTY(Transient, Replicated)
	uint32 CurrentWeaponAmmo;

	UPROPERTY(Transient, ReplicatedUsing = OnRep_BurstCounter)
	int32 BurstCounter;

	/* Current weapon state */
	EWeaponState CurrentWeaponState;

	/* is ready to fire? */
	uint32 bWantsFire : 1;
	
	uint32 bIsRefiring;

	/* Time of last weapon fire */
	float LastFireTime;

	UPROPERTY(Config)
	bool bAllowAutoWeapon = true;

	UPROPERTY(Transient)
	float TimerIntervalAdjustment;

	/** Managment of fire timer */
	FTimerHandle  Fire_TimerHandle;

	FTimerHandle OnEquip_TimerHandle;

public:
	///////////////* Server side *////////////
	///
	UFUNCTION(Reliable, Server, WithValidation)
	void ServerStartFire();

	UFUNCTION(Reliable, Server, WithValidation)
	void ServerStopFire();

	UFUNCTION(Reliable, Server, WithValidation)
	void ServerFireHandle();

public:
	/////////////* Replication */////////////
	///
	UFUNCTION()
	void OnRep_CharacterBase();

	UFUNCTION()
	void OnRep_BurstCounter();

	virtual void SimulateFire();
	
	virtual void StopSimulatingFire();

public:

	/* weapon specific fire implementation */
	virtual void Fire();

	/* Start fire */
	virtual void OnBurstStart(); 

	/* Stop fire */
	virtual void OnBurstFinished();
	
	/* handle weapon refire */
	void HandleRefiring();

	/* handle weapon fire */
	void HandleFire();

	/* setting up weapon state */
	void SetWeaponState(EWeaponState NewState);

	/* determine current weapon state */
	void DetermineWeaponState();
	
public:

	/* Attachs weapon to character skeletal mesh */
	void AttachToCharacter();

	/* Detaches weapon from character mesh */
	void DetachFromCharacter();

	FVector GetAdjustedAim() const;
	
	/* Get the aim of the camera */
	FVector GetCameraAim() const;

	/* Get the weapon muzzle location */
	FVector GetMuzzleLocation() const;

	/* Get the weapon muzzle rotation */
	FVector GetMuzzleRotation() const;

	/* get the originating location for camera damage */
	FVector GetCameraDamageStartLocation(const FVector& AimDirection);

	FHitResult WeaponHitTrace(const FVector& From, const FVector& To) const;
	

public:
	/// Weapon inventory
	///
	/* Check if player equip weapon */
	bool IsEquip() const;

	/* Weapon is equipped */
	virtual void OnEquip(const AWeaponBase* LastWeapon);

	/* Weapon is holstered */
	virtual void OnUnEquip();

	/* Weapon can be used now */
	virtual void OnEquipFinished();
	
	/* Add weapon to player's inventory */
	virtual void OnAddWeapon(AALSBaseCharacter* NewOwner);

	/* Remove weapon from player's inventory */
	virtual void OnRemoveWeapon();

	/* Check if weapon attached to character */
	bool IsAttachedToCharacter() const;

protected:
	/// Inventory
	//
	uint8 bIsEquipped : 1;
	
	float EquipStartedTime;

	float EquipDuration;

	bool bIsEquipping;

protected:
	/// Effects and Sound
	//
	/* Muzzle FX */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon | Effects")
	UParticleSystem* MuzzleFX;

	/* Muzzle FX Component */
	UPROPERTY(Transient)
	UParticleSystemComponent* MuzzleParticleSystemComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon | Effects")
	FName MuzzleSocket;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon | Effects")
	UForceFeedbackEffect* FireFeedbackEffect;

	/// Sound
	//
	/* fire audio component (bLoopedSound) */
	UPROPERTY(Transient)
	UAudioComponent* FireAudioComponent;
	
	/* firing sound (single shot)*/
	UPROPERTY(EditDefaultsOnly, Category = "Weapon | Sounds")
	USoundCue* FireCue;

	/* firing sound (Lopped shot)*/
	UPROPERTY(EditDefaultsOnly, Category = "Weapon | Sounds")
	USoundCue* FireLoopCue;

	/* Firing end sound */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon | Sounds")
	USoundCue* FireEndCue;

	/* Reload sound */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon | Sounds")
	USoundCue* ReloadCue;

	/* Equip Sound */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon | Sounds")
	USoundCue* EquipSound;
	
	/* Shake camera when firing */
	TSubclassOf<UCameraShakeBase> CameraShake;

	/* Is Muzzled FX looped */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon | Effects")
	uint32 bLoopedMuzzleFX : 1;

	/* Is fire sound looped */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon | Sounds")
	uint32 bLoopedSound : 1;
	
public:

	/* Play weapon sounds */
	UAudioComponent* PlayWeaponSound(USoundCue* Sound);
	
public:

	/* Return current ammo type */
	virtual EAmmoType GetAmmoType() const { return EAmmoType::Bullet; };

	virtual void BeginPlay() override;

	virtual void PostInitializeComponents() override;

	virtual void Destroyed() override;

	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;


public:
	USkeletalMeshComponent* GetWeaponMesh() const { return WeaponSkeletalMeshComp; } ;
};
