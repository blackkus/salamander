#include "SalamanderCharacter.h"

#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "SalamanderCharacterMovementComponent.h"

ASalamanderCharacter::ASalamanderCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<USalamanderCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	GetMesh()->SetRelativeLocationAndRotation(FVector(0.f, 0.f, -90.f), FRotator(0.f, -90.f, 0.f));
	GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);

	// Mesh et AnimBP du template Third Person (assets existants, non modifiés).
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshAsset(TEXT("/Game/Characters/Mannequins/Meshes/SKM_Quinn_Simple"));
	if (MeshAsset.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(MeshAsset.Object);
	}

	static ConstructorHelpers::FObjectFinder<UAnimBlueprint> AnimAsset(TEXT("/Game/Characters/Mannequins/Anims/Unarmed/ABP_Unarmed"));
	if (AnimAsset.Succeeded())
	{
		GetMesh()->SetAnimInstanceClass(AnimAsset.Object->GeneratedClass);
	}

	// Assets d'input du template + IA_Sprint / IA_Crouch (creés par script éditeur).
	static ConstructorHelpers::FObjectFinder<UInputMappingContext> IMCAsset(TEXT("/Game/Input/IMC_Default"));
	if (IMCAsset.Succeeded())
	{
		DefaultMappingContext = IMCAsset.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> MoveAsset(TEXT("/Game/Input/Actions/IA_Move"));
	if (MoveAsset.Succeeded())
	{
		MoveAction = MoveAsset.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> JumpAsset(TEXT("/Game/Input/Actions/IA_Jump"));
	if (JumpAsset.Succeeded())
	{
		JumpAction = JumpAsset.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> LookAsset(TEXT("/Game/Input/Actions/IA_Look"));
	if (LookAsset.Succeeded())
	{
		LookAction = LookAsset.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> MouseLookAsset(TEXT("/Game/Input/Actions/IA_MouseLook"));
	if (MouseLookAsset.Succeeded())
	{
		MouseLookAction = MouseLookAsset.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> SprintAsset(TEXT("/Game/Input/Actions/IA_Sprint"));
	if (SprintAsset.Succeeded())
	{
		SprintAction = SprintAsset.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> CrouchAsset(TEXT("/Game/Input/Actions/IA_Crouch"));
	if (CrouchAsset.Succeeded())
	{
		CrouchAction = CrouchAsset.Object;
	}
}

void ASalamanderCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (APlayerController* PC = Cast<APlayerController>(NewController))
	{
		if (ULocalPlayer* LocalPlayer = PC->GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
			{
				if (DefaultMappingContext)
				{
					Subsystem->AddMappingContext(DefaultMappingContext, 0);
				}
			}
		}
	}
}

void ASalamanderCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveAction)
		{
			EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASalamanderCharacter::Move);
		}
		if (JumpAction)
		{
			EnhancedInput->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
			EnhancedInput->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		}
		if (LookAction)
		{
			EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASalamanderCharacter::Look);
		}
		if (MouseLookAction)
		{
			EnhancedInput->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &ASalamanderCharacter::Look);
		}
		if (SprintAction)
		{
			EnhancedInput->BindAction(SprintAction, ETriggerEvent::Started, this, &ASalamanderCharacter::StartSprint);
			EnhancedInput->BindAction(SprintAction, ETriggerEvent::Completed, this, &ASalamanderCharacter::StopSprint);
		}
		if (CrouchAction)
		{
			EnhancedInput->BindAction(CrouchAction, ETriggerEvent::Started, this, &ASalamanderCharacter::StartCrouch);
			EnhancedInput->BindAction(CrouchAction, ETriggerEvent::Completed, this, &ASalamanderCharacter::StopCrouch);
		}
	}
}

void ASalamanderCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ASalamanderCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ASalamanderCharacter::StartSprint()
{
	if (USalamanderCharacterMovementComponent* MoveComp = Cast<USalamanderCharacterMovementComponent>(GetCharacterMovement()))
	{
		MoveComp->bSprinting = true;
	}
	if (!HasAuthority())
	{
		ServerSetSprinting(true);
	}
}

void ASalamanderCharacter::StopSprint()
{
	if (USalamanderCharacterMovementComponent* MoveComp = Cast<USalamanderCharacterMovementComponent>(GetCharacterMovement()))
	{
		MoveComp->bSprinting = false;
	}
	if (!HasAuthority())
	{
		ServerSetSprinting(false);
	}
}

void ASalamanderCharacter::ServerSetSprinting_Implementation(bool bSprint)
{
	if (USalamanderCharacterMovementComponent* MoveComp = Cast<USalamanderCharacterMovementComponent>(GetCharacterMovement()))
	{
		MoveComp->bSprinting = bSprint;
	}
}

void ASalamanderCharacter::StartCrouch()
{
	Crouch();
}

void ASalamanderCharacter::StopCrouch()
{
	UnCrouch();
}
