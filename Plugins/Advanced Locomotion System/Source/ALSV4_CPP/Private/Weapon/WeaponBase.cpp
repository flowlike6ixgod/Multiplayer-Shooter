// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/WeaponBase.h"
#include "Weapon/Weapon.h"
#include "AI/ALSAIController.h"
#include "Character/ALSPlayerCameraManager.h"
#include "Kismet/GameplayStatics.h"
#include "CollisionQueryParams.h"
#include "Character/ALSBaseCharacter.h"
#include "Character/ALSPlayerController.h"
#include "Components/AudioComponent.h"
#include "Net/UnrealNetwork.h"
#include "Particles/ParticleSystemComponent.h"

// Sets default values
AWeaponBase::AWeaponBase()
{
	WeaponSkeletalMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponSkeletalMesh"));
	RootComponent = WeaponSkeletalMeshComp;

	bWantsFire = false;
	bIsEquipped = false;
	bIsRefiring = false;
	bIsEquipped = false;
	bIsEquipping = false;
	bLoopedSound = false;
	bLoopedMuzzleFX = false;

	BurstCounter = 0;
	LastFireTime = 0.f;
	EquipStartedTime = 0.f;
	EquipDuration = 0.f;

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;
	SetRemoteRoleForBackwardsCompat(ROLE_SimulatedProxy);
	bReplicates = true;
	bNetUseOwnerRelevancy = true;
}

/**
 *  Ammo Getters
 */
int32 AWeaponBase::GetMaxClipAmmo() const
{
	return WeaponData.MaxClipAmmo;
}

int32 AWeaponBase::GetCurrentClipAmmo() const
{
	return CurrentClipAmmo;
}

int32 AWeaponBase::GetMaxWeaponAmmo() const
{
	return WeaponData.MaxWeaponAmmo;
}

int32 AWeaponBase::GetCurrentWeaponAmmo() const
{
	return CurrentWeaponAmmo;
}

FName AWeaponBase::GetWeaponName() const
{
	return FName(*WeaponData.WeaponName);
}

AALSBaseCharacter* AWeaponBase::GetOwnerCharacter() const
{
	return CharacterBase;
}

void AWeaponBase::SetWeaponOwner(AALSBaseCharacter* WeaponOwner)
{
	if (CharacterBase != WeaponOwner)
	{
		SetInstigator(WeaponOwner);
		CharacterBase = WeaponOwner;
		SetOwner(WeaponOwner);
		UE_LOG(LogTemp, Warning, TEXT("SET WEAPON OWNER FUNC()"));
	}
}

bool AWeaponBase::CanFire()
{
	bool bCanFire = CharacterBase && CharacterBase->CanFire();
	bool bStateFire = ((CurrentWeaponState == EWeaponState::None) || (CurrentWeaponState == EWeaponState::Fire));

	return ((bCanFire == true) && (bStateFire == true));
}

UAudioComponent* AWeaponBase::PlayWeaponSound(USoundCue* Sound)
{
	UAudioComponent* AudioComponent = nullptr;
	if (Sound && CharacterBase)
	{
		AudioComponent = UGameplayStatics::SpawnSoundAttached(Sound, CharacterBase->GetRootComponent());
	}

	return AudioComponent;
}

// Called when the game starts or when spawned
void AWeaponBase::BeginPlay()
{
	Super::BeginPlay();

	CurrentWeaponState = EWeaponState::None;
}

void AWeaponBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	CurrentWeaponAmmo = WeaponData.MaxWeaponAmmo;
	CurrentClipAmmo = WeaponData.MaxClipAmmo;

	DetachFromCharacter();
}

void AWeaponBase::Destroyed()
{
	Super::Destroyed();

	StopSimulatingFire();
}

void AWeaponBase::OnBurstStart()
{
	const float GameTime = GetWorld()->GetTimeSeconds();

	if (LastFireTime > 0.f && WeaponData.DelayBetweenShots > 0.f && LastFireTime + WeaponData.DelayBetweenShots >
		GameTime)
	{
		GetWorldTimerManager().SetTimer(Fire_TimerHandle, this, &AWeaponBase::HandleFire,
		                                LastFireTime + WeaponData.DelayBetweenShots - GameTime, false);
	}
	else
	{
		HandleFire();
	}
}

void AWeaponBase::OnBurstFinished()
{
	BurstCounter = 0;

	StopSimulatingFire();
	
	GetWorldTimerManager().ClearTimer(Fire_TimerHandle);
	bIsRefiring = false;

	TimerIntervalAdjustment = 0.f;
}

void AWeaponBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeaponBase, CharacterBase);
	DOREPLIFETIME_CONDITION(AWeaponBase, CurrentClipAmmo, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AWeaponBase, CurrentWeaponAmmo, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AWeaponBase, BurstCounter, COND_SkipOwner);
}

void AWeaponBase::StartFire()
{
	if (GetLocalRole() < ROLE_Authority)
	{
		ServerStartFire();
	}

	if (!bWantsFire)
	{
		bWantsFire = true;
		DetermineWeaponState();
	}
}

void AWeaponBase::StopFire()
{
	if ((GetLocalRole() < ROLE_Authority) && CharacterBase && CharacterBase->IsLocallyControlled())
	{
		ServerStopFire();
	}

	if (bWantsFire)
	{
		bWantsFire = false;
		DetermineWeaponState();
	}
}

void AWeaponBase::ServerStartFire_Implementation()
{
	StartFire();
}

bool AWeaponBase::ServerStartFire_Validate()
{
	return true;
}

void AWeaponBase::ServerStopFire_Implementation()
{
	StopFire();
}

bool AWeaponBase::ServerStopFire_Validate()
{
	return true;
}

void AWeaponBase::ServerFireHandle_Implementation()
{
	HandleFire();
}

bool AWeaponBase::ServerFireHandle_Validate()
{
	return true;
}

void AWeaponBase::OnRep_CharacterBase()
{
	if (CharacterBase)
	{
		OnAddWeapon(CharacterBase);
	}
	else
	{
		OnRemoveWeapon();
	}
}

void AWeaponBase::OnRep_BurstCounter()
{
	if (BurstCounter > 0)
	{
		SimulateFire();
	}
	else
	{
		StopSimulatingFire();
	}
}

void AWeaponBase::SimulateFire()
{
	if (GetLocalRole() == ROLE_Authority && CurrentWeaponState != EWeaponState::Fire)
	{
		UE_LOG(LogTemp, Warning, TEXT("Simulate fire return in first state"));
		return;
	}

	if (MuzzleFX)
	{
		USkeletalMeshComponent* WeapMesh = GetWeaponMesh();

		if (!bLoopedMuzzleFX || MuzzleParticleSystemComponent == nullptr)
		{
			MuzzleParticleSystemComponent = UGameplayStatics::SpawnEmitterAttached(MuzzleFX, WeapMesh, MuzzleSocket);
		}
	}
	if (bLoopedSound)
	{
		if (!FireAudioComponent)
		{
			FireAudioComponent = PlayWeaponSound(FireLoopCue);
		}
	}
	else
	{
		PlayWeaponSound(FireCue);
	}

	AALSPlayerController* PC = (CharacterBase != nullptr)
		                           ? Cast<AALSPlayerController>(CharacterBase->Controller)
		                           : nullptr;
	if (PC && PC->IsLocalController())
	{
		if (CameraShake)
		{
			PC->ClientStartCameraShake(CameraShake, 1.5f);
		}
		if (FireFeedbackEffect)
		{
			FForceFeedbackParameters FeedbackParameters;
			FeedbackParameters.Tag = "Weapon";
			PC->ClientPlayForceFeedback(FireFeedbackEffect, FeedbackParameters);
		}
	}
}

void AWeaponBase::StopSimulatingFire()
{
	if (bLoopedMuzzleFX)
	{
		if (MuzzleParticleSystemComponent)
		{
			MuzzleParticleSystemComponent->DeactivateSystem();
			MuzzleParticleSystemComponent = nullptr;
		}
	}

	if (FireAudioComponent)
	{
		FireAudioComponent->FadeOut(0.2f, 0.f);
		FireAudioComponent = nullptr;

		PlayWeaponSound(FireEndCue);
	}
}

void AWeaponBase::Fire()
{
}

void AWeaponBase::HandleRefiring()
{
	UWorld* NewWorld = GetWorld();

	float SlackTimeThisFrame = FMath::Max(0.f, (NewWorld->TimeSeconds - LastFireTime) - WeaponData.DelayBetweenShots);

	if (bAllowAutoWeapon)
	{
		TimerIntervalAdjustment -= SlackTimeThisFrame;
	}

	HandleFire();
}

void AWeaponBase::HandleFire()
{
	if (CanFire())
	{
		// if net mode is not dedicated server, start simulate fire
		if (GetNetMode() != NM_DedicatedServer)
		{
			SimulateFire();
		}
		if (CharacterBase && CharacterBase->IsLocallyControlled())
		{
			Fire();
			BurstCounter++;
		}
	}
	else if (CharacterBase && CharacterBase->IsLocallyControlled())
	{
		if (BurstCounter > 0)
		{
			OnBurstFinished();
		}
	}
	else
	{
		OnBurstFinished();
	}

	if (CharacterBase && CharacterBase->IsLocallyControlled())
	{
		// Notify the server that we are firing
		if (GetLocalRole() < ROLE_Authority)
		{
			ServerFireHandle();
		}
		// Setup refire time
		bIsRefiring = (CurrentWeaponState == EWeaponState::Fire && WeaponData.DelayBetweenShots > 0.f);
		if (bIsRefiring)
		{
			GetWorldTimerManager().SetTimer(Fire_TimerHandle, this, &AWeaponBase::HandleRefiring,
			                                FMath::Max<float>(WeaponData.DelayBetweenShots + TimerIntervalAdjustment,
			                                                  SMALL_NUMBER), false);
			TimerIntervalAdjustment = 0.f;
		}
	}

	LastFireTime = GetWorld()->GetTimeSeconds();
}

void AWeaponBase::SetWeaponState(EWeaponState NewState)
{
	const EWeaponState PreviousState = CurrentWeaponState;

	if (PreviousState == EWeaponState::Fire && NewState != EWeaponState::Fire)
	{
		OnBurstFinished();
	}

	CurrentWeaponState = NewState;

	if (PreviousState != EWeaponState::Fire && NewState == EWeaponState::Fire)
	{
		OnBurstStart();
	}
}

void AWeaponBase::DetermineWeaponState()
{
	EWeaponState State = EWeaponState::None;

	if (bIsEquipped)
	{
		if ((bWantsFire == true) && (CanFire() == true))
		{
			State = EWeaponState::Fire;
		}
	}
	else if (bIsEquipping)
	{
		State = EWeaponState::Equipping;
	}

	SetWeaponState(State);
	UE_LOG(LogTemp, Display, TEXT("WeaponState: %d"), CurrentWeaponState);
}

void AWeaponBase::OnAddWeapon(AALSBaseCharacter* NewOwner)
{
	SetWeaponOwner(NewOwner);
}

void AWeaponBase::OnRemoveWeapon()
{
	if (IsAttachedToCharacter())
	{
		OnUnEquip();
	}

	if (GetLocalRole() == ROLE_Authority)
	{
		SetWeaponOwner(nullptr);
	}
}

void AWeaponBase::AttachToCharacter()
{
	if (CharacterBase)
	{
		// Remove character mesh
		DetachFromCharacter();
		FName WeaponAttachPoint = CharacterBase->GetWeaponAttachPoint();

		if (CharacterBase->IsLocallyControlled() == true)
		{
			USkeletalMeshComponent* SkMesh = CharacterBase->GetMesh();
			WeaponSkeletalMeshComp->SetHiddenInGame(false);
			WeaponSkeletalMeshComp->AttachToComponent(SkMesh, FAttachmentTransformRules::KeepRelativeTransform,
			                                          WeaponAttachPoint);
			UE_LOG(LogTemp, Warning, TEXT("Local Attach"));
		}
		else
		{
			USkeletalMeshComponent* GunMesh = GetWeaponMesh();
			USkeletalMeshComponent* CharacterMesh = CharacterBase->GetMesh();
			GunMesh->AttachToComponent(CharacterMesh, FAttachmentTransformRules::KeepRelativeTransform,
			                           WeaponAttachPoint);
			GunMesh->SetHiddenInGame(false);
			UE_LOG(LogTemp, Warning, TEXT("Global Attach"));
		}
	}
}

void AWeaponBase::DetachFromCharacter()
{
	WeaponSkeletalMeshComp->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
	WeaponSkeletalMeshComp->SetHiddenInGame(true);
	UE_LOG(LogTemp, Warning, TEXT("DETACHED"));
}

FVector AWeaponBase::GetAdjustedAim() const
{
	AALSPlayerController* PC = GetInstigatorController<AALSPlayerController>();
	FVector Aim = FVector::ZeroVector;
	if (PC)
	{
		FVector CameraLocation;
		FRotator CameraRotation;

		PC->GetPlayerViewPoint(CameraLocation, CameraRotation);

		Aim = CameraRotation.Vector();
	}

	return Aim;
}

FVector AWeaponBase::GetCameraAim() const
{
	AALSPlayerController* PC = GetInstigatorController<AALSPlayerController>();
	FVector Aim = FVector::ZeroVector;
	if (PC)
	{
		FVector CameraLocation;
		FRotator CameraRotation;

		PC->GetPlayerViewPoint(CameraLocation, CameraRotation);
		Aim = CameraRotation.Vector();
	}

	return Aim;
}

FVector AWeaponBase::GetMuzzleLocation() const
{
	USkeletalMeshComponent* WeaponMesh = GetWeaponMesh();
	return WeaponMesh->GetSocketLocation(FName("MuzzleFlash"));
}

FVector AWeaponBase::GetMuzzleRotation() const
{
	USkeletalMeshComponent* WeaponMesh = GetWeaponMesh();
	return WeaponMesh->GetSocketRotation(FName("MuzzleFlash")).Vector();
}

FVector AWeaponBase::GetCameraDamageStartLocation(const FVector& AimDirection)
{
	AALSPlayerController* PC = CharacterBase ? Cast<AALSPlayerController>(CharacterBase->Controller) : nullptr;
	FVector StartTrace = FVector::ZeroVector;

	if (PC)
	{
		FRotator Rotation;
		PC->GetPlayerViewPoint(StartTrace, Rotation);
		StartTrace = StartTrace + AimDirection * (GetInstigator()->GetActorLocation() - StartTrace | AimDirection);
		UE_LOG(LogTemp, Warning, TEXT("GetCameraDamageStartLocation() called"));
	}
	return StartTrace;
}

FHitResult AWeaponBase::WeaponHitTrace(const FVector& From, const FVector& To) const
{
	FCollisionQueryParams CollisionTraceParams(SCENE_QUERY_STAT(WeaponHitTrace), true, GetInstigator());
	CollisionTraceParams.bReturnPhysicalMaterial = true;

	FHitResult Hit(ForceInit);
	GetWorld()->LineTraceSingleByChannel(Hit, From, To, ECC_Visibility, CollisionTraceParams);
	DrawDebugLine(GetWorld(), From, To, FColor::Green, false, 5.0f, 0, 3.0f);
	UE_LOG(LogTemp, Warning, TEXT("Hit Result"));

	return Hit;
}

bool AWeaponBase::IsEquip() const
{
	return bIsEquipped;
}

void AWeaponBase::OnEquip(const AWeaponBase* LastWeapon)
{
	AttachToCharacter();
	bIsEquipping = true;
	DetermineWeaponState();

	if (LastWeapon)
	{
		float Duration = 0.5f;
		EquipStartedTime = GetWorld()->GetTimeSeconds();
		EquipDuration = Duration;

		GetWorldTimerManager().SetTimer(OnEquip_TimerHandle, this, &AWeaponBase::OnEquipFinished, Duration, false);
	}
	else
	{
		OnEquipFinished();
	}

	AALSBaseCharacter::NotifyEquipWeapon.Broadcast(CharacterBase, this);
}

void AWeaponBase::OnEquipFinished()
{
	AttachToCharacter();

	bIsEquipped = true;
	bIsEquipping = false;

	DetermineWeaponState();
}

void AWeaponBase::OnUnEquip()
{
	DetachFromCharacter();
	bIsEquipped = false;
	StopFire();

	if (bIsEquipping)
	{
		bIsEquipping = false;
		GetWorldTimerManager().ClearTimer(OnEquip_TimerHandle);
	}

	AALSBaseCharacter::NotifyUnEquipWeapon.Broadcast(CharacterBase, this);

	DetermineWeaponState();
}

bool AWeaponBase::IsAttachedToCharacter() const
{
	return bIsEquipped || bIsEquipping;
}
