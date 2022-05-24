// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Weapon.h"

#include "Character/ALSBaseCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"

AWeapon::AWeapon()
{
	CurrentFiringSpread = 0.0f;
	SpreadReduction = 0.0f;
}

float AWeapon::GetCurrentSpread() const
{
	float FinalSpread = WeaponBaseData.WeaponSpread + CurrentFiringSpread;
	if (CharacterBase->GetStance() == EALSStance::Crouching)
	{
		FinalSpread /= WeaponBaseData.SpreadModifier;
	}
	else if (CharacterBase->GetGait() == EALSGait::Running || CharacterBase->GetGait() == EALSGait::Walking)
	{
		FinalSpread *= 1.5f;
	}

	return FinalSpread;
}

EAmmoType AWeapon::GetAmmoType() const
{
	return EAmmoType::Bullet;
}

void AWeapon::ServerHitNotify_Implementation(const FHitResult& Hit, FVector_NetQuantizeNormal ShootDirection,
                                             int32 RandSeed, float Spread)
{
	// Convert Radians to degrees
	const float WeaponAngleDot = FMath::Abs(FMath::Sin(Spread * PI / 180.f));

	if (GetInstigator() && (Hit.GetActor() || Hit.bBlockingHit))
	{
		const FVector Source = GetMuzzleLocation();
		const FVector ViewDirection = (Hit.Location - Source).GetSafeNormal();

		const float ViewDotHitDirection = FVector::DotProduct(GetInstigator()->GetViewRotation().Vector(),
		                                                      ViewDirection);
		if (ViewDotHitDirection > WeaponBaseData.AllowedViewDotHitDir - WeaponAngleDot)
		{
			if (CurrentWeaponState != EWeaponState::None)
			{
				if (Hit.GetActor() == nullptr)
				{
					if (Hit.bBlockingHit)
					{
						HitConfirmed(Hit, Source, ShootDirection, RandSeed, Spread);
					}
					else if (Hit.GetActor()->IsRootComponentStatic() || Hit.GetActor()->IsRootComponentStationary())
					{
						HitConfirmed(Hit, Source, ShootDirection, RandSeed, Spread);
					}
					else
					{
						const FBox HitBox = Hit.GetActor()->GetComponentsBoundingBox();

						FVector BoxExtent = 0.5 * (HitBox.Max - HitBox.Min);

						BoxExtent *= WeaponBaseData.ClientSideHitLeeway;
						BoxExtent.X = FMath::Max(20.0f, BoxExtent.X);
						BoxExtent.Y = FMath::Max(20.0f, BoxExtent.Y);
						BoxExtent.Z = FMath::Max(20.0f, BoxExtent.Z);

						// Get the box center
						const FVector BoxCenter = (HitBox.Min + HitBox.Max) * 0.5;
						if (FMath::Abs(Hit.Location.Z - BoxCenter.Z) < BoxExtent.Z &&
							FMath::Abs(Hit.Location.X - BoxCenter.X) < BoxExtent.X &&
							FMath::Abs(Hit.Location.Y - BoxCenter.Y) < BoxExtent.Y)
						{
							HitConfirmed(Hit, Source, ShootDirection, RandSeed, Spread);
						}
						else
						{
							UE_LOG(LogTemp, Log,
							       TEXT("%s Rejected client side hit of %s (outside bounding box tolerance)"),
							       *GetNameSafe(this), *GetNameSafe(Hit.GetActor()));
						}
					}
				}
			}
		}
		else if (ViewDotHitDirection <= WeaponBaseData.AllowedViewDotHitDir)
		{
			UE_LOG(LogTemp, Log, TEXT("%s Rejected client side hit of %s (facing too far from the hit direction)"),
			       *GetNameSafe(this), *GetNameSafe(Hit.GetActor()));
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("%s Rejected client side hit of %s"), *GetNameSafe(this),
			       *GetNameSafe(Hit.GetActor()));
		}
	}
}

bool AWeapon::ServerHitNotify_Validate(const FHitResult& Hit, FVector_NetQuantizeNormal ShootDirection,
                                       int32 RandSeed, float Spread)
{
	return true;
}

void AWeapon::ServerMissNotify_Implementation(FVector_NetQuantizeNormal ShootDirection, int32 RandSeed,
                                              float Spread)
{
	const FVector Source = GetMuzzleLocation();
	WeaponSpread.Source = Source;
	WeaponSpread.Spread = Spread;
	WeaponSpread.RandomSeed = RandSeed;

	if (GetNetMode() != NM_DedicatedServer)
	{
		const FVector EndTrace = Source + ShootDirection * WeaponBaseData.WeaponRange;
		SpawnTrailEffect(EndTrace);
	}
}

bool AWeapon::ServerMissNotify_Validate(FVector_NetQuantizeNormal ShootDirection, int32 RandSeed, float Spread)
{
	return true;
}

void AWeapon::ProcessHit(const FHitResult& HitResult, const FVector& Source, const FVector& ShootDirection,
                         int32 RandSeed, float Spread)
{
	if (CharacterBase && CharacterBase->IsLocallyControlled() && GetNetMode() == NM_Client)
	{
		if (HitResult.GetActor() && HitResult.GetActor()->GetRemoteRole() == ROLE_Authority)
		{
			ServerHitNotify(HitResult, ShootDirection, RandSeed, Spread);
		}
		else if (HitResult.GetActor() == nullptr)
		{
			if (HitResult.bBlockingHit)
			{
				ServerHitNotify(HitResult, ShootDirection, RandSeed, Spread);
			}
			else
			{
				ServerMissNotify(ShootDirection, RandSeed, Spread);
			}
		}
	}

	HitConfirmed(HitResult, Source, ShootDirection, RandSeed, Spread);
}

void AWeapon::HitConfirmed(const FHitResult& HitResult, const FVector& Source, const FVector& ShootDirection,
                           int32 RandSeed, float Spread)
{
	if (ShouldDealDamage(HitResult.GetActor()))
	{
		DealDamage(HitResult, ShootDirection);
	}

	if (GetLocalRole() == ROLE_Authority)
	{
		WeaponSpread.Source = Source;
		WeaponSpread.Spread = Spread;
		WeaponSpread.RandomSeed = RandSeed;
	}

	if (GetNetMode() != NM_DedicatedServer)
	{
		const FVector EndTrace = Source + ShootDirection * WeaponBaseData.WeaponRange;
		const FVector EndPoint = HitResult.GetActor() ? HitResult.ImpactPoint : EndTrace;

		SpawnHitEffect(HitResult);
		SpawnTrailEffect(EndPoint);
	}
}

bool AWeapon::ShouldDealDamage(AActor* Actor) const
{
	if (Actor)
	{
		if (GetNetMode() != NM_Client || Actor->GetLocalRole() == ROLE_Authority || Actor->GetTearOff())
		{
			return true;
		}
	}

	return false;
}

void AWeapon::DealDamage(const FHitResult& Hit, const FVector& ShootDirection)
{
	FPointDamageEvent DamageEvent;
	DamageEvent.DamageTypeClass = WeaponBaseData.DamageType;
	DamageEvent.HitInfo = Hit;
	DamageEvent.ShotDirection = ShootDirection;
	DamageEvent.Damage = WeaponBaseData.HitDamage;

	Hit.GetActor()->TakeDamage(DamageEvent.Damage, DamageEvent, CharacterBase->Controller, this);
}

void AWeapon::Fire()
{
	Super::Fire();

	const int32 RandomSeed = FMath::Rand();
	FRandomStream WeaponRandomStream(RandomSeed);
	const float CurrentSpread = GetCurrentSpread();
	const float ConeHalfAngle = FMath::DegreesToRadians(CurrentSpread / SpreadReduction);

	const FVector AimDir = GetAdjustedAim();
	const FVector StartTrace = GetCameraDamageStartLocation(AimDir);
	const FVector ShootDirection = WeaponRandomStream.VRandCone(AimDir, ConeHalfAngle, ConeHalfAngle);
	const FVector EndTrace = StartTrace + ShootDirection * WeaponBaseData.WeaponRange;

	const FHitResult Hit = WeaponHitTrace(StartTrace, EndTrace);
	ProcessHit(Hit, StartTrace, ShootDirection, RandomSeed, CurrentSpread);
	
	CurrentFiringSpread = FMath::Min(WeaponBaseData.MaxSpreadModifier,(CurrentFiringSpread) + WeaponBaseData.SpreadModifier);
	
}

void AWeapon::OnBurstFinished()
{
	Super::OnBurstFinished();

	CurrentFiringSpread = 0.0f;
}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AWeapon, WeaponSpread, COND_SkipOwner);
}

void AWeapon::OnRep_HitNotify()
{
	SimulateHit(WeaponSpread.Source, WeaponSpread.RandomSeed, WeaponSpread.Spread);
}

void AWeapon::SimulateHit(const FVector& Source, int32 RandSeed, float Spread)
{
	FRandomStream WeaponRandomStream(RandSeed);
	const float ConeHalfAngle = FMath::DegreesToRadians(Spread / WeaponBaseData.SpreadReduction);

	const FVector StartTrace = Source;
	const FVector AimDir = GetAdjustedAim();
	const FVector ShootDir = WeaponRandomStream.VRandCone(AimDir, ConeHalfAngle, ConeHalfAngle);
	const FVector EndTrace = StartTrace + ShootDir * WeaponBaseData.WeaponRange;

	FHitResult Hit = WeaponHitTrace(StartTrace, EndTrace);

	if (Hit.bBlockingHit)
	{
		SpawnHitEffect(Hit);
		SpawnTrailEffect(Hit.ImpactPoint);
	}
	else
	{
		SpawnTrailEffect(EndTrace);
	}
}

void AWeapon::SpawnHitEffect(const FHitResult& Hit)
{
	if (ImpactEffects && Hit.bBlockingHit)
	{
		FHitResult UseHit = Hit;

		if (Hit.Component.IsValid())
		{
			const FVector StartTrace = Hit.ImpactPoint + Hit.ImpactNormal * 10.f;
			const FVector EndTrace = Hit.ImpactPoint - Hit.ImpactNormal * 10.f;

			FHitResult HitResult = WeaponHitTrace(StartTrace, EndTrace);
			UseHit = HitResult;
		}

		FTransform const SpawnTransform(Hit.ImpactNormal.Rotation(), Hit.ImpactPoint);
		AWeaponEffects* Effect = GetWorld()->SpawnActorDeferred<AWeaponEffects>(ImpactEffects, SpawnTransform);
		if (Effect)
		{
			Effect->SurfaceHit = UseHit;
			UGameplayStatics::FinishSpawningActor(Effect, SpawnTransform);
		}
	}
}

void AWeapon::SpawnTrailEffect(const FVector& EndPoint)
{
	if (TrailFX)
	{
		const FVector Source = GetMuzzleLocation();

		UParticleSystemComponent* TrailParticle = UGameplayStatics::SpawnEmitterAtLocation(this, TrailFX, Source);
		if (TrailParticle)
		{
			TrailParticle->SetVectorParameter(TrailTargetParam, EndPoint);
		}
	}
}
