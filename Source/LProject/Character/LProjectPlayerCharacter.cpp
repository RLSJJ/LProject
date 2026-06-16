// Copyright Epic Games, Inc. All Rights Reserved.

#include "Character/LProjectPlayerCharacter.h"

#include "AbilitySystem/Abilities/LProjectGA_BasicAttack.h"
#include "AbilitySystem/Abilities/LProjectGA_Counter.h"
#include "AbilitySystem/Abilities/LProjectGA_Dash.h"
#include "AbilitySystem/Abilities/LProjectSkills.h"
#include "AbilitySystem/LProjectAbilitySet.h"
#include "Animation/AnimSequence.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "AbilitySystem/LProjectAbilitySystemComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Core/LProjectGameplayTags.h"
#include "Core/LProjectPawnData.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/HitResult.h"
#include "Engine/LocalPlayer.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Input/LProjectInputConfig.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "InputCoreTypes.h"
#include "InputMappingContext.h"
#include "UObject/ConstructorHelpers.h"

ALProjectPlayerCharacter::ALProjectPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true; // needed for click-to-move auto-run

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

	// Test visual: imported CesiumMan skeletal mesh + its walk animation (replaces the dev cube).
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> PlayerSkelMesh(
	    TEXT("/Game/TestVisual/CesiumMan/CesiumMan/SkeletalMeshes/CesiumMan.CesiumMan"));
	if (PlayerSkelMesh.Succeeded())
	{
		VisualMesh = PlayerSkelMesh.Object;
	}
	static ConstructorHelpers::FObjectFinder<UAnimSequence> PlayerAnimSeq(
	    TEXT("/Game/TestVisual/CesiumMan/CesiumMan/SkeletalMeshes/CesiumMan_Anim.CesiumMan_Anim"));
	if (PlayerAnimSeq.Succeeded())
	{
		VisualAnim = PlayerAnimSeq.Object;
	}

	// Movement: face the movement direction; ignore controller rotation (camera is fixed).
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->bOrientRotationToMovement = true;
		Movement->RotationRate = FRotator(0.0f, 720.0f, 0.0f);
	}
}

void ALProjectPlayerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Auto-run: after a short right-click, keep walking to the cached point until we reach it.
	if (bAutoRunToDestination)
	{
		FVector ToDest = CachedDestination - GetActorLocation();
		ToDest.Z = 0.0f;
		if (ToDest.SizeSquared() <= FMath::Square(DestinationAcceptanceRadius))
		{
			bAutoRunToDestination = false;
		}
		else
		{
			AddMovementInput(ToDest.GetSafeNormal(), 1.0f);
		}
	}
}

void ALProjectPlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController); // base binds ability actor info
	EnsureDefaultPawnData();
	GrantAbilities();
}

void ALProjectPlayerCharacter::ApplyTestVisual()
{
	if (!ConfigureTestVisualMesh(VisualMesh, VisualTargetHeight, FRotator(0.0f, VisualMeshYaw, 0.0f)))
	{
		return;
	}

	// Real mesh is in: hide the dev cube and loop the walk animation (single-node, no AnimBP needed).
	if (DevVisualMesh)
	{
		DevVisualMesh->SetVisibility(false);
	}
	if (VisualAnim && GetMesh())
	{
		GetMesh()->SetAnimationMode(EAnimationMode::AnimationSingleNode);
		GetMesh()->PlayAnimation(VisualAnim, true);
	}
}

void ALProjectPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	ApplyTestVisual();
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

	// Native: right-mouse click-to-move (steer while held, walk-to-point on a tap).
	if (const UInputAction* MoveAction = InputConfig->FindNativeInputActionForTag(TAG_InputTag_Move))
	{
		Input->BindAction(MoveAction,
		    ETriggerEvent::Triggered,
		    this,
		    &ALProjectPlayerCharacter::OnSetDestinationTriggered);
		Input->BindAction(MoveAction,
		    ETriggerEvent::Completed,
		    this,
		    &ALProjectPlayerCharacter::OnSetDestinationReleased);
		Input->BindAction(MoveAction,
		    ETriggerEvent::Canceled,
		    this,
		    &ALProjectPlayerCharacter::OnSetDestinationReleased);
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

void ALProjectPlayerCharacter::OnSetDestinationTriggered(const FInputActionValue& Value)
{
	FollowTime += GetWorld()->GetDeltaSeconds();
	bAutoRunToDestination = false; // manual follow takes over while the button is held

	if (const APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		FHitResult Hit;
		if (PC->GetHitResultUnderCursor(ECC_Visibility, false, Hit))
		{
			CachedDestination = Hit.ImpactPoint;
		}
	}

	FVector ToDest = CachedDestination - GetActorLocation();
	ToDest.Z = 0.0f;
	if (!ToDest.IsNearlyZero())
	{
		AddMovementInput(ToDest.GetSafeNormal(), 1.0f);
	}
}

void ALProjectPlayerCharacter::OnSetDestinationReleased(const FInputActionValue& Value)
{
	// A short press (click) means "walk to that point"; a long hold was already manual steering.
	if (FollowTime <= ShortPressThreshold)
	{
		bAutoRunToDestination = true;
	}
	FollowTime = 0.0f;
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

	// Code default (no ability set assigned): grant the starter kit directly, tagged for input activation.
	FGameplayAbilitySpec DashSpec(ULProjectGA_Dash::StaticClass(), 1, INDEX_NONE, this);
	DashSpec.GetDynamicSpecSourceTags().AddTag(TAG_InputTag_Dash);
	AbilitySystemComponent->GiveAbility(DashSpec);

	FGameplayAbilitySpec BasicAttackSpec(ULProjectGA_BasicAttack::StaticClass(), 1, INDEX_NONE, this);
	BasicAttackSpec.GetDynamicSpecSourceTags().AddTag(TAG_InputTag_BasicAttack);
	AbilitySystemComponent->GiveAbility(BasicAttackSpec);

	FGameplayAbilitySpec CounterSpec(ULProjectGA_Counter::StaticClass(), 1, INDEX_NONE, this);
	CounterSpec.GetDynamicSpecSourceTags().AddTag(TAG_InputTag_Counter);
	AbilitySystemComponent->GiveAbility(CounterSpec);

	// QWER skill kit.
	auto GrantSkill = [this](TSubclassOf<ULProjectGameplayAbility> Ability, const FGameplayTag& InputTag)
	{
		FGameplayAbilitySpec Spec(Ability, 1, INDEX_NONE, this);
		Spec.GetDynamicSpecSourceTags().AddTag(InputTag);
		AbilitySystemComponent->GiveAbility(Spec);
	};
	GrantSkill(ULProjectGA_Charge::StaticClass(), TAG_InputTag_SkillQ);
	GrantSkill(ULProjectGA_Cleave::StaticClass(), TAG_InputTag_SkillW);
	GrantSkill(ULProjectGA_Bolt::StaticClass(), TAG_InputTag_SkillE);
	GrantSkill(ULProjectGA_Awakening::StaticClass(), TAG_InputTag_SkillR);
}

void ALProjectPlayerCharacter::EnsureDefaultPawnData()
{
	if (PawnData)
	{
		return;
	}

	// Input actions: move = a button (right mouse), dash = a button (space), basic attack = left mouse.
	UInputAction* MoveAction = NewObject<UInputAction>(this, TEXT("DefaultMoveAction"));
	MoveAction->ValueType = EInputActionValueType::Boolean;
	UInputAction* DashAction = NewObject<UInputAction>(this, TEXT("DefaultDashAction"));
	DashAction->ValueType = EInputActionValueType::Boolean;
	UInputAction* BasicAttackAction = NewObject<UInputAction>(this, TEXT("DefaultBasicAttackAction"));
	BasicAttackAction->ValueType = EInputActionValueType::Boolean;
	UInputAction* CounterAction = NewObject<UInputAction>(this, TEXT("DefaultCounterAction"));
	CounterAction->ValueType = EInputActionValueType::Boolean;

	auto MakeButton = [this](const TCHAR* Name)
	{
		UInputAction* A = NewObject<UInputAction>(this, Name);
		A->ValueType = EInputActionValueType::Boolean;
		return A;
	};
	UInputAction* SkillQAction = MakeButton(TEXT("DefaultSkillQAction"));
	UInputAction* SkillWAction = MakeButton(TEXT("DefaultSkillWAction"));
	UInputAction* SkillEAction = MakeButton(TEXT("DefaultSkillEAction"));
	UInputAction* SkillRAction = MakeButton(TEXT("DefaultSkillRAction"));

	// Mapping context: RMB move, Space dash, LMB 평타, F counter, Q/W/E/R skills.
	UInputMappingContext* IMC = NewObject<UInputMappingContext>(this, TEXT("DefaultMappingContext"));
	IMC->MapKey(MoveAction, EKeys::RightMouseButton);
	IMC->MapKey(DashAction, EKeys::SpaceBar);
	IMC->MapKey(BasicAttackAction, EKeys::LeftMouseButton);
	IMC->MapKey(CounterAction, EKeys::F);
	IMC->MapKey(SkillQAction, EKeys::Q);
	IMC->MapKey(SkillWAction, EKeys::W);
	IMC->MapKey(SkillEAction, EKeys::E);
	IMC->MapKey(SkillRAction, EKeys::R);

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

	FLProjectInputAction BasicAttackEntry;
	BasicAttackEntry.InputAction = BasicAttackAction;
	BasicAttackEntry.InputTag = TAG_InputTag_BasicAttack;
	InputConfig->AbilityInputActions.Add(BasicAttackEntry);

	FLProjectInputAction CounterEntry;
	CounterEntry.InputAction = CounterAction;
	CounterEntry.InputTag = TAG_InputTag_Counter;
	InputConfig->AbilityInputActions.Add(CounterEntry);

	auto AddAbilityInput = [&](UInputAction* Action, const FGameplayTag& Tag)
	{
		FLProjectInputAction Entry;
		Entry.InputAction = Action;
		Entry.InputTag = Tag;
		InputConfig->AbilityInputActions.Add(Entry);
	};
	AddAbilityInput(SkillQAction, TAG_InputTag_SkillQ);
	AddAbilityInput(SkillWAction, TAG_InputTag_SkillW);
	AddAbilityInput(SkillEAction, TAG_InputTag_SkillE);
	AddAbilityInput(SkillRAction, TAG_InputTag_SkillR);

	// Bundle it. AbilitySet stays null -> GrantAbilities() uses the direct dash grant above.
	PawnData = NewObject<ULProjectPawnData>(this, TEXT("DefaultPawnData"));
	PawnData->DefaultMappingContext = IMC;
	PawnData->InputConfig = InputConfig;
}
