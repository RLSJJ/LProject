// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/Abilities/LProjectSkills.h"

#include "AbilitySystem/Effects/LProjectGE_AttackUp.h"
#include "AbilitySystemComponent.h"
#include "Core/LProjectGameplayTags.h"
#include "GameFramework/Character.h"

// ============================= Charge (Q) =============================
ULProjectGA_Charge::ULProjectGA_Charge()
{
	CooldownTag = TAG_Cooldown_Charge;
	CooldownDuration = 6.0f;
	IdentityGain = 600.0f;
}

void ULProjectGA_Charge::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ACharacter* Avatar = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (!Avatar)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	const FVector Dir = GetAimDirection();
	Avatar->SetActorRotation(Dir.Rotation());
	Avatar->LaunchCharacter(Dir * LaunchStrength, true, true);

	const FVector Center = Avatar->GetActorLocation() + Dir * 260.0f;
	ApplyShapeDamage(Center, Dir.Rotation().Quaternion(), FCollisionShape::MakeSphere(260.0f), Damage, StaggerDamage);
	GainIdentity(IdentityGain);

	GrantIFramesAndEndLater(0.35f);
}

// ============================= Cleave (W) =============================
ULProjectGA_Cleave::ULProjectGA_Cleave()
{
	CooldownTag = TAG_Cooldown_Cleave;
	CooldownDuration = 9.0f;
	IdentityGain = 600.0f;
}

void ULProjectGA_Cleave::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ACharacter* Avatar = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (Avatar)
	{
		const FVector Dir = GetAimDirection();
		Avatar->SetActorRotation(Dir.Rotation());
		const FVector Center = Avatar->GetActorLocation() + Dir * 320.0f;
		ApplyShapeDamage(Center,
		    Dir.Rotation().Quaternion(),
		    FCollisionShape::MakeBox(FVector(360, 320, 220)),
		    Damage,
		    StaggerDamage);
		GainIdentity(IdentityGain);
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

// ============================= Bolt (E) =============================
ULProjectGA_Bolt::ULProjectGA_Bolt()
{
	CooldownTag = TAG_Cooldown_Bolt;
	CooldownDuration = 4.0f;
	IdentityGain = 600.0f;
}

void ULProjectGA_Bolt::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (ACharacter* Avatar = Cast<ACharacter>(GetAvatarActorFromActorInfo()))
	{
		const FVector Dir = GetAimDirection();
		Avatar->SetActorRotation(Dir.Rotation());
		const FVector Center = Avatar->GetActorLocation() + Dir * (Range * 0.5f);
		ApplyShapeDamage(Center,
		    Dir.Rotation().Quaternion(),
		    FCollisionShape::MakeBox(FVector(Range * 0.5f, 70, 90)),
		    Damage,
		    StaggerDamage);
		GainIdentity(IdentityGain);
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

// ============================= Awakening (R) =============================
ULProjectGA_Awakening::ULProjectGA_Awakening()
{
	CooldownTag = TAG_Cooldown_Awakening;
	CooldownDuration = 45.0f;
	IdentityGain = 0.0f;
	SelfBuff = ULProjectGE_AttackUp::StaticClass();
}

void ULProjectGA_Awakening::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
	// Requires (near-)full Identity; if not, cancel without consuming the cooldown.
	if (GetIdentity() < GetIdentityMax() * 0.99f)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ACharacter* Avatar = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	if (Avatar && SourceASC)
	{
		ApplyShapeDamage(Avatar->GetActorLocation(),
		    FQuat::Identity,
		    FCollisionShape::MakeSphere(Radius),
		    Damage,
		    StaggerDamage);

		// Self attack buff.
		if (SelfBuff)
		{
			FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
			Context.AddSourceObject(this);
			FGameplayEffectSpecHandle Spec = SourceASC->MakeOutgoingSpec(SelfBuff, GetAbilityLevel(), Context);
			if (Spec.IsValid())
			{
				SourceASC->ApplyGameplayEffectSpecToTarget(*Spec.Data, SourceASC);
			}
		}

		// Spend all Identity.
		GainIdentity(-GetIdentityMax());
	}

	GrantIFramesAndEndLater(0.5f);
}
