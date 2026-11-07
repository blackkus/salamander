#include "SalamanderCharacter.h"

#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "InputTriggers.h"
#include "SalamanderCharacterMovementComponent.h"
#include "SalamanderPlayerState.h"
#include "AbilitySystemComponent.h"
#include "GE_InitStats.h"
#include "GameplayTagContainer.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "TimerManager.h"

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

	// Mesh and AnimBP from the Third Person template (existing assets, untouched).
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

	// Input assets from the template + IA_Sprint / IA_Crouch (created by editor script).
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

	// GAS: bind the PlayerState ASC to this pawn (owner=PlayerState, avatar=pawn).
	if (ASalamanderPlayerState* PS = GetPlayerState<ASalamanderPlayerState>())
	{
		if (UAbilitySystemComponent* ASC = PS->GetASC())
		{
			ASC->InitAbilityActorInfo(PS, this);
			if (HasAuthority())
			{
				// Init attributes (Health/MaxHealth = 100) via GameplayEffect, server only.
				FGameplayEffectContextHandle Ctx = ASC->MakeEffectContext();
				Ctx.AddSourceObject(this);
				FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(UGE_InitStats::StaticClass(), 1.f, Ctx);
				if (Spec.IsValid())
				{
					ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
				}
			}
		}
	}

	if (APlayerController* PC = Cast<APlayerController>(NewController))
	{
		if (ULocalPlayer* LocalPlayer = PC->GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
			{
				if (DefaultMappingContext)
				{
					Subsystem->AddMappingContext(DefaultMappingContext, 0);

					// IA_Sprint / IA_Crouch are created by an editor script; the key mapping
					// is added here in code (idempotent thanks to UnmapKey).
					if (SprintAction)
					{
						DefaultMappingContext->UnmapKey(SprintAction, EKeys::LeftShift);
						FEnhancedActionKeyMapping& SprintMapping = DefaultMappingContext->MapKey(SprintAction, EKeys::LeftShift);
						SprintMapping.Triggers.Add(NewObject<UInputTriggerPressed>(DefaultMappingContext));
					}
					if (CrouchAction)
					{
						DefaultMappingContext->UnmapKey(CrouchAction, EKeys::LeftControl);
						FEnhancedActionKeyMapping& CrouchMapping = DefaultMappingContext->MapKey(CrouchAction, EKeys::LeftControl);
						CrouchMapping.Triggers.Add(NewObject<UInputTriggerPressed>(DefaultMappingContext));
					}
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
	if (IsDead())
	{
		return;
	}
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
	if (IsDead())
	{
		return;
	}
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ASalamanderCharacter::StartSprint()
{
	if (IsDead())
	{
		return;
	}
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
	if (IsDead())
	{
		return;
	}
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
	if (IsDead())
	{
		return;
	}
	Crouch();
}

void ASalamanderCharacter::StopCrouch()
{
	if (IsDead())
	{
		return;
	}
	UnCrouch();
}

void ASalamanderCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	if (ASalamanderPlayerState* PS = GetPlayerState<ASalamanderPlayerState>())
	{
		if (UAbilitySystemComponent* ASC = PS->GetASC())
		{
			ASC->InitAbilityActorInfo(PS, this); // client side, on PlayerState replication
		}
		UE_LOG(LogTemp, Log, TEXT("[Char] PlayerState received: Name=%s Level=%d"), *PS->PlayerName, PS->Level);
	}
}

void ASalamanderCharacter::BeginPlay()
{
	Super::BeginPlay();

	// CLI tests (client only): auto play sprint / crouch after connecting.
	if (!HasAuthority())
	{
		bool bSprint = false, bCrouch = false;
		FParse::Bool(FCommandLine::Get(), TEXT("TestAutoSprint="), bSprint);
		FParse::Bool(FCommandLine::Get(), TEXT("TestAutoCrouch="), bCrouch);
		FParse::Bool(FCommandLine::Get(), TEXT("TestAutoMove="), bAutoMove);
		if (bSprint)
		{
			FTimerHandle H1, H2;
			GetWorldTimerManager().SetTimer(H1, [this]() { UE_LOG(LogTemp, Log, TEXT("[Char] AUTO SPRINT ON")); StartSprint(); }, 5.f, false);
			GetWorldTimerManager().SetTimer(H2, [this]() { UE_LOG(LogTemp, Log, TEXT("[Char] AUTO SPRINT OFF")); StopSprint(); }, 10.f, false);
		}
		if (bCrouch)
		{
			FTimerHandle H1, H2;
			GetWorldTimerManager().SetTimer(H1, [this]() { UE_LOG(LogTemp, Log, TEXT("[Char] AUTO CROUCH ON")); StartCrouch(); }, 5.f, false);
			GetWorldTimerManager().SetTimer(H2, [this]() { UE_LOG(LogTemp, Log, TEXT("[Char] AUTO CROUCH OFF")); StopCrouch(); }, 10.f, false);
		}
	}
}

void ASalamanderCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (bAutoMove)
	{
		AddMovementInput(FVector(1.f, 0.f, 0.f), 1.f);
	}
	// Log dead/alive transitions (replicated tag) so clients see death and respawn.
	const bool bDead = IsDead();
	if (bDead != bWasDead)
	{
		UE_LOG(LogTemp, Log, TEXT("[Char] %s dead-tag %s"), *GetName(), bDead ? TEXT("ON") : TEXT("OFF"));
		bWasDead = bDead;
	}
}

bool ASalamanderCharacter::IsDead() const
{
	if (const ASalamanderPlayerState* PS = GetPlayerState<ASalamanderPlayerState>())
	{
		if (const UAbilitySystemComponent* ASC = PS->GetASC())
		{
			return ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("State.Dead")));
		}
	}
	return false;
}
