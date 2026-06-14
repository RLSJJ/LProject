// Copyright Epic Games, Inc. All Rights Reserved.

#include "Character/LProjectPlayerCharacter.h"

#include "AbilitySystem/LProjectAbilitySystemComponent.h"
#include "AbilitySystem/Abilities/LProjectGA_Dash.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputActionValue.h"

ALProjectPlayerCharacter::ALProjectPlayerCharacter()
{
	// Quarterview camera: fixed angle, does not rotate with the pawn.
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 1100.0f;
	CameraBoom->SetRelativeRotation(FRotator(-55.0f, 0.0f, 0.0f));
	CameraBoom->bDoCollisionTest = false;
	CameraBoom->bInheritPitch = false;
	CameraBoom->bInheritYaw = false;
	CameraBoom->bInheritRoll = false;
	CameraBoom->bUsePawnControlRotation = false;

	TopDownCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
	TopDownCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	TopDownCamera->bUsePawnControlRotation = false;

	// Movement: face the movement direction; ignore controller rotation (camera is fixed).
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->bOrientRotationToMovement = true;
		Movement->RotationRate = FRotator(0.0f, 720.0f, 0.0f);
		Movement->bConstrainToPlane = true;
		Movement->bSnapToPlaneAtStart = true;
	}

	DashAbility = ULProjectGA_Dash::StaticClass();
}

void ALProjectPlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController); // base binds ability actor info
	GrantDefaultAbilities();
}

void ALProjectPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (const APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
		        ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}
	}
}

void ALProjectPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveAction)
		{
			Input->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ALProjectPlayerCharacter::Move);
		}
		if (DashAction)
		{
			Input->BindAction(DashAction, ETriggerEvent::Started, this, &ALProjectPlayerCharacter::Input_Dash);
		}
	}
}

void ALProjectPlayerCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	// Quarterview with a fixed, world-aligned camera: move along world axes.
	AddMovementInput(FVector::ForwardVector, Axis.Y);
	AddMovementInput(FVector::RightVector, Axis.X);
}

void ALProjectPlayerCharacter::Input_Dash(const FInputActionValue& Value)
{
	if (AbilitySystemComponent && DashAbility)
	{
		AbilitySystemComponent->TryActivateAbilityByClass(DashAbility);
	}
}

void ALProjectPlayerCharacter::GrantDefaultAbilities()
{
	if (!AbilitySystemComponent || !HasAuthority())
	{
		return;
	}

	TArray<TSubclassOf<ULProjectGameplayAbility>> Abilities = DefaultAbilities;
	if (DashAbility)
	{
		Abilities.AddUnique(DashAbility);
	}

	for (const TSubclassOf<ULProjectGameplayAbility>& Ability : Abilities)
	{
		if (Ability)
		{
			AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(Ability, 1));
		}
	}
}
