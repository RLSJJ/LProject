// Copyright Epic Games, Inc. All Rights Reserved.

#include "Character/LProjectPlayerCharacter.h"

#include "AbilitySystem/Abilities/LProjectGA_Dash.h"
#include "AbilitySystem/LProjectAbilitySet.h"
#include "AbilitySystem/LProjectAbilitySystemComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Core/LProjectGameplayTags.h"
#include "Core/LProjectPawnData.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Input/LProjectInputConfig.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "InputCoreTypes.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"
#include "UObject/ConstructorHelpers.h"

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

	// Placeholder visible body: engine cube fitted to the capsule (collision stays on the capsule).
	DevVisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DevVisualMesh"));
	DevVisualMesh->SetupAttachment(GetCapsuleComponent());
	DevVisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	DevVisualMesh->SetRelativeScale3D(FVector(0.68f, 0.68f, 1.76f));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		DevVisualMesh->SetStaticMesh(CubeMesh.Object);
	}

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
}

void ALProjectPlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController); // base binds ability actor info
	EnsureDefaultPawnData();
	GrantAbilities();
}

void ALProjectPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	EnsureDefaultPawnData();

	if (const APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
		        ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (PawnData && PawnData->DefaultMappingContext)
			{
				Subsystem->AddMappingContext(PawnData->DefaultMappingContext, 0);
			}
		}
	}
}

void ALProjectPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	EnsureDefaultPawnData();

	UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	const ULProjectInputConfig* InputConfig = PawnData ? PawnData->InputConfig : nullptr;
	if (!Input || !InputConfig)
	{
		return;
	}

	// Native: movement.
	if (const UInputAction* MoveAction = InputConfig->FindNativeInputActionForTag(TAG_InputTag_Move))
	{
		Input->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ALProjectPlayerCharacter::Move);
	}

	// Abilities: activate by input tag (data-driven; add an ability input = add a config entry).
	for (const FLProjectInputAction& Action : InputConfig->AbilityInputActions)
	{
		if (Action.InputAction && Action.InputTag.IsValid())
		{
			Input->BindAction(Action.InputAction,
			    ETriggerEvent::Started,
			    this,
			    &ALProjectPlayerCharacter::Input_AbilityTagPressed,
			    Action.InputTag);
			Input->BindAction(Action.InputAction,
			    ETriggerEvent::Completed,
			    this,
			    &ALProjectPlayerCharacter::Input_AbilityTagReleased,
			    Action.InputTag);
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

void ALProjectPlayerCharacter::Input_AbilityTagPressed(const FInputActionValue& Value, FGameplayTag InputTag)
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->AbilityInputTagPressed(InputTag);
	}
}

void ALProjectPlayerCharacter::Input_AbilityTagReleased(const FInputActionValue& Value, FGameplayTag InputTag)
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->AbilityInputTagReleased(InputTag);
	}
}

void ALProjectPlayerCharacter::GrantAbilities()
{
	if (!AbilitySystemComponent || !HasAuthority())
	{
		return;
	}

	if (PawnData && PawnData->AbilitySet)
	{
		PawnData->AbilitySet->GiveToAbilitySystem(AbilitySystemComponent, this);
		return;
	}

	// Code default (no ability set assigned): grant the dash directly, tagged for input activation.
	FGameplayAbilitySpec DashSpec(ULProjectGA_Dash::StaticClass(), 1, INDEX_NONE, this);
	DashSpec.GetDynamicSpecSourceTags().AddTag(TAG_InputTag_Dash);
	AbilitySystemComponent->GiveAbility(DashSpec);
}

void ALProjectPlayerCharacter::EnsureDefaultPawnData()
{
	if (PawnData)
	{
		return;
	}

	// Input actions.
	UInputAction* MoveAction = NewObject<UInputAction>(this, TEXT("DefaultMoveAction"));
	MoveAction->ValueType = EInputActionValueType::Axis2D;
	UInputAction* DashAction = NewObject<UInputAction>(this, TEXT("DefaultDashAction"));
	DashAction->ValueType = EInputActionValueType::Boolean;

	// Mapping context: WASD -> Axis2D move (X = right, Y = forward), Space -> dash.
	UInputMappingContext* IMC = NewObject<UInputMappingContext>(this, TEXT("DefaultMappingContext"));
	IMC->MapKey(MoveAction, EKeys::D);                                                          // +X
	IMC->MapKey(MoveAction, EKeys::A).Modifiers.Add(NewObject<UInputModifierNegate>(IMC));      // -X
	IMC->MapKey(MoveAction, EKeys::W).Modifiers.Add(NewObject<UInputModifierSwizzleAxis>(IMC)); // X -> +Y
	{
		FEnhancedActionKeyMapping& Back = IMC->MapKey(MoveAction, EKeys::S);
		Back.Modifiers.Add(NewObject<UInputModifierSwizzleAxis>(IMC));
		Back.Modifiers.Add(NewObject<UInputModifierNegate>(IMC)); // X -> -Y
	}
	IMC->MapKey(DashAction, EKeys::SpaceBar);

	// Input config: action -> tag.
	ULProjectInputConfig* InputConfig = NewObject<ULProjectInputConfig>(this, TEXT("DefaultInputConfig"));
	FLProjectInputAction MoveEntry;
	MoveEntry.InputAction = MoveAction;
	MoveEntry.InputTag = TAG_InputTag_Move;
	InputConfig->NativeInputActions.Add(MoveEntry);

	FLProjectInputAction DashEntry;
	DashEntry.InputAction = DashAction;
	DashEntry.InputTag = TAG_InputTag_Dash;
	InputConfig->AbilityInputActions.Add(DashEntry);

	// Bundle it. AbilitySet stays null -> GrantAbilities() uses the direct dash grant above.
	PawnData = NewObject<ULProjectPawnData>(this, TEXT("DefaultPawnData"));
	PawnData->DefaultMappingContext = IMC;
	PawnData->InputConfig = InputConfig;
}
