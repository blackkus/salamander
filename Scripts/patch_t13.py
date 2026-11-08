# -*- coding: utf-8 -*-
# T13: main menu with key rebinding, AZERTY default keys, mouse-look context,
# overhead name tags + health bars, enemy faces the player.
import io

BASE = 'C:/Users/Blackkus/Documents/Unreal Projects/Salamander/Source/Salamander/'

def patch(path, pairs):
    p = BASE + path
    s = io.open(p, encoding='utf-8').read()
    for old, new in pairs:
        assert old in s, '%s: anchor not found: %r' % (path, old[:60])
        s = s.replace(old, new, 1)
    io.open(p, 'w', encoding='utf-8', newline='').write(s)
    print('patched', path)

# ---------- SalamanderCharacter.h ----------
patch('SalamanderCharacter.h', [
    ("class UInputAction;\n",
     "class UInputAction;\nclass UWidgetComponent;\n"),
    ("""	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input", meta=(AllowPrivateAccess="true"))
	UInputAction* AttackAction;

public:""",
     """	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input", meta=(AllowPrivateAccess="true"))
	UInputAction* AttackAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input", meta=(AllowPrivateAccess="true"))
	UInputMappingContext* MouseLookContext;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="UI", meta=(AllowPrivateAccess="true"))
	UWidgetComponent* NameTagComponent;

public:"""),
    ("""	// Activates the melee attack ability (tag Ability.MeleeAttack).
	void TryMeleeAttack();
};""",
     """	// Activates the melee attack ability (tag Ability.MeleeAttack).
	void TryMeleeAttack();

	// Overhead name tag: nick from PlayerState.
	FString GetDisplayName() const;
	// Health fraction [0,1] from the PlayerState ASC.
	float GetHealthFraction() const;
};"""),
])

# ---------- SalamanderCharacter.cpp ----------
patch('SalamanderCharacter.cpp', [
    ('#include "TimerManager.h"\n',
     '#include "TimerManager.h"\n#include "Components/WidgetComponent.h"\n#include "NameTagWidget.h"\n#include "SalamanderInputLib.h"\n#include "SalamanderAttributeSet.h"\n'),
    ("""	static ConstructorHelpers::FObjectFinder<UInputMappingContext> IMCAsset(TEXT("/Game/Input/IMC_Default"));
	if (IMCAsset.Succeeded())
	{
		DefaultMappingContext = IMCAsset.Object;
	}
""",
     """	static ConstructorHelpers::FObjectFinder<UInputMappingContext> IMCAsset(TEXT("/Game/Input/IMC_Default"));
	if (IMCAsset.Succeeded())
	{
		DefaultMappingContext = IMCAsset.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputMappingContext> MouseIMCAsset(TEXT("/Game/Input/IMC_MouseLook"));
	if (MouseIMCAsset.Succeeded())
	{
		MouseLookContext = MouseIMCAsset.Object;
	}
"""),
    ("""					Subsystem->AddMappingContext(DefaultMappingContext, 0);
""",
     """					Subsystem->AddMappingContext(DefaultMappingContext, 0);

					// Mouse look (mouse X/Y) lives in IMC_MouseLook.
					if (MouseLookContext)
					{
						Subsystem->AddMappingContext(MouseLookContext, 1);
					}

					// Saved key bindings + AZERTY alt keys (WASD/ZQSD).
					USalamanderInputLib::ApplyAllBindings();
"""),
    ("""void ASalamanderCharacter::BeginPlay()
{
	Super::BeginPlay();
""",
     """void ASalamanderCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Overhead name tag + health bar (created on every client).
	NameTagComponent = UNameTagWidget::CreateNameTagComponent(this);
"""),
    ("""	const bool bDead = IsDead();
	if (bDead != bWasDead)
	{
		UE_LOG(LogTemp, Log, TEXT("[Char] %s dead-tag %s"), *GetName(), bDead ? TEXT("ON") : TEXT("OFF"));
		bWasDead = bDead;
	}
}""",
     """	const bool bDead = IsDead();
	if (bDead != bWasDead)
	{
		UE_LOG(LogTemp, Log, TEXT("[Char] %s dead-tag %s"), *GetName(), bDead ? TEXT("ON") : TEXT("OFF"));
		bWasDead = bDead;
	}
	if (NameTagComponent != nullptr)
	{
		UNameTagWidget::UpdateNameTag(NameTagComponent, GetDisplayName(), GetHealthFraction(), GetWorld());
	}
}

FString ASalamanderCharacter::GetDisplayName() const
{
	if (const ASalamanderPlayerState* PS = GetPlayerState<ASalamanderPlayerState>())
	{
		return PS->PlayerName;
	}
	return TEXT("?");
}

float ASalamanderCharacter::GetHealthFraction() const
{
	if (const ASalamanderPlayerState* PS = GetPlayerState<ASalamanderPlayerState>())
	{
		if (const UAbilitySystemComponent* ASC = PS->GetASC())
		{
			const float H = ASC->GetGameplayAttributeValue(USalamanderAttributeSet::GetHealthAttribute());
			const float M = ASC->GetGameplayAttributeValue(USalamanderAttributeSet::GetMaxHealthAttribute());
			return M > 0.f ? H / M : 1.f;
		}
	}
	return 1.f;
}"""),
])

# ---------- EnemyCharacter.h ----------
patch('EnemyCharacter.h', [
    ("class USalamanderAttributeSet;\n",
     "class USalamanderAttributeSet;\nclass UWidgetComponent;\n"),
    ("""	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<USalamanderAttributeSet> AttributeSet;
""",
     """	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<USalamanderAttributeSet> AttributeSet;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	TObjectPtr<UWidgetComponent> NameTagComponent;
"""),
    ("""	AActor* FindNearestPlayer() const;
	void SetBodyColor(const FLinearColor& Color);
	void DoAttackHit(AActor* Target);
	void OnHealthChanged(const FOnAttributeChangeData& Data);
};""",
     """	AActor* FindNearestPlayer() const;
	void SetBodyColor(const FLinearColor& Color);
	void DoAttackHit(AActor* Target);
	void OnHealthChanged(const FOnAttributeChangeData& Data);

public:
	// Overhead name tag label ("Enemy", boss overrides to "Boss").
	virtual FString GetDisplayName() const { return TEXT("Enemy"); }
	// Health fraction [0,1] from the own ASC.
	float GetHealthFraction() const;
};"""),
])

# ---------- EnemyCharacter.cpp ----------
patch('EnemyCharacter.cpp', [
    ('#include "SalamanderGameMode.h"\n',
     '#include "SalamanderGameMode.h"\n#include "Components/WidgetComponent.h"\n#include "NameTagWidget.h"\n'),
    ("""void AEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();
""",
     """void AEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Overhead name tag + health bar (created on every client).
	NameTagComponent = UNameTagWidget::CreateNameTagComponent(this);
"""),
    ("""void AEnemyCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!HasAuthority())
	{
		return;
	}
""",
     """void AEnemyCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Name tag on every machine.
	if (NameTagComponent != nullptr)
	{
		UNameTagWidget::UpdateNameTag(NameTagComponent, GetDisplayName(), GetHealthFraction(), GetWorld());
	}

	if (!HasAuthority())
	{
		return;
	}
"""),
    ("""		// Chase directly (no navmesh yet) toward the nearest player.
		const FVector Dir = (Target->GetActorLocation() - GetActorLocation()).GetSafeNormal();
		AddMovementInput(Dir, 1.f);
		break;""",
     """		// Chase directly (no navmesh yet) toward the nearest player.
		const FVector Dir = (Target->GetActorLocation() - GetActorLocation()).GetSafeNormal();
		// Face the target: never walk backwards toward the player.
		const FRotator FaceRot = FRotationMatrix::MakeFromX(Dir).Rotator();
		SetActorRotation(FMath::RInterpTo(GetActorRotation(), FaceRot, DeltaSeconds, 12.f));
		AddMovementInput(Dir, 1.f);
		break;"""),
    ("""	case EEnemyState::Warn:
		StateTimer -= DeltaSeconds;""",
     """	case EEnemyState::Warn:
		// Keep facing the target during the telegraph.
		if (Target != nullptr)
		{
			const FVector ToTarget = Target->GetActorLocation() - GetActorLocation();
			const FRotator FaceRot = FRotationMatrix::MakeFromX(ToTarget.GetSafeNormal()).Rotator();
			SetActorRotation(FMath::RInterpTo(GetActorRotation(), FaceRot, DeltaSeconds, 12.f));
		}
		StateTimer -= DeltaSeconds;"""),
    ("""AActor* AEnemyCharacter::FindNearestPlayer() const""",
     """float AEnemyCharacter::GetHealthFraction() const
{
	if (AbilitySystemComponent != nullptr)
	{
		const float H = AbilitySystemComponent->GetGameplayAttributeValue(USalamanderAttributeSet::GetHealthAttribute());
		const float M = AbilitySystemComponent->GetGameplayAttributeValue(USalamanderAttributeSet::GetMaxHealthAttribute());
		return M > 0.f ? H / M : 1.f;
	}
	return 1.f;
}

AActor* AEnemyCharacter::FindNearestPlayer() const"""),
])

# ---------- BossCharacter.h ----------
patch('BossCharacter.h', [
    ("""	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
""",
     """	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	virtual FString GetDisplayName() const override { return TEXT("Boss"); }
"""),
])

# ---------- SalamanderGameMode.cpp: apply bindings on client menu ----------
patch('SalamanderGameMode.cpp', [
    ('#include "PlayerProfile.h"\n',
     '#include "PlayerProfile.h"\n#include "SalamanderInputLib.h"\n'),
    ("""		DefaultPawnClass = nullptr;
		ShowLoginMenu();
""",
     """		DefaultPawnClass = nullptr;
		USalamanderInputLib::ApplyAllBindings();
		ShowLoginMenu();
"""),
])

print('patch_t13 OK')
