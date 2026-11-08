#include "SalamanderGameMode.h"

#include "Engine/Engine.h"
#include "Engine/EngineTypes.h"
#include "Engine/GameViewportClient.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerStart.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "SLoginWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Math/Color.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "FileSaveBackend.h"
#include "PlayerProfile.h"
#include "SalamanderInputLib.h"
#include "SalamanderCharacter.h"
#include "SalamanderPlayerState.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "GameplayTagContainer.h"
#include "GE_Damage.h"
#include "DA_DeathRules.h"
#include "EnemyCharacter.h"
#include "BossCharacter.h"
#include "RewardTypes.h"
#include "Engine/DataTable.h"
#include "SalamanderCharacter.h"
#include "GameplayTagContainer.h"
#include "SalamanderAttributeSet.h"
#include "SaveBackend.h"
#include "TimerManager.h"

// In CLI (-game), the engine overwrites the Name option with the local nickname; so we read
// the "Nick" option first (sent by start_client.bat), "Name" as fallback (editor).
static FString GetNickOption(const FString& Options)
{
	if (UGameplayStatics::HasOption(Options, TEXT("Nick")))
	{
		return UGameplayStatics::ParseOption(Options, TEXT("Nick"));
	}
	return UGameplayStatics::ParseOption(Options, TEXT("Name"));
}

ASalamanderGameMode::ASalamanderGameMode()
{
	PlayerStateClass = ASalamanderPlayerState::StaticClass();
	DefaultPawnClass = ASalamanderCharacter::StaticClass();

	// The PlayerController stays the template Third Person one (Blueprint untouched).
	static ConstructorHelpers::FClassFinder<AController> PCBP(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonPlayerController"));
	if (PCBP.Succeeded())
	{
		PlayerControllerClass = PCBP.Class;
	}
}

void ASalamanderGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	// Save backend: files on the server disk, one per player.
	SaveBackendObj = NewObject<UFileSaveBackend>(this);
	SaveBackend = Cast<ISaveBackend>(SaveBackendObj.Get());

	// Death rules data asset (PLACEHOLDER values).
	DeathRules = LoadObject<UDA_DeathRules>(nullptr, TEXT("/Game/Data/DA_DeathRules"));

	// Spawn simple enemies (server only). Count via -EnemyCount=<n> (default 2).
	if (GetWorld()->GetNetMode() != NM_Client)
	{
		int32 EnemyCount = 2;
		FParse::Value(FCommandLine::Get(), TEXT("EnemyCount="), EnemyCount);
		FVector BaseLoc = FVector::ZeroVector;
		for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
		{
			BaseLoc = It->GetActorLocation();
			break;
		}
		for (int32 i = 0; i < EnemyCount; i++)
		{
			const FVector Loc = BaseLoc + FVector(250.f * (i + 1), 250.f * i, 0.f);
			GetWorld()->SpawnActor<AEnemyCharacter>(AEnemyCharacter::StaticClass(), Loc, FRotator::ZeroRotator);
		}
		UE_LOG(LogTemp, Log, TEXT("[GameMode] spawned %d enemies"), EnemyCount);

		// One boss further away (-BossCount=<n>, default 1).
		int32 BossCount = 1;
		FParse::Value(FCommandLine::Get(), TEXT("BossCount="), BossCount);
		for (int32 i = 0; i < BossCount; i++)
		{
			const FVector Loc = BaseLoc + FVector(900.f, 500.f, 0.f);
			GetWorld()->SpawnActor<ABossCharacter>(ABossCharacter::StaticClass(), Loc, FRotator::ZeroRotator);
		}
		if (BossCount > 0)
		{
			UE_LOG(LogTemp, Log, TEXT("[GameMode] spawned %d boss"), BossCount);
		}
	}

	// Periodic save every 60 s.
	GetWorldTimerManager().SetTimer(SaveTimerHandle, this, &ASalamanderGameMode::SaveAllProfiles, 60.f, true);

	// CLI test: -TestMigrateV1=<key> writes a v1 profile, then loads it back through the save backend.
	FString MigrateKey;
	if (FParse::Value(FCommandLine::Get(), TEXT("TestMigrateV1="), MigrateKey) && !MigrateKey.IsEmpty() && SaveBackend != nullptr)
	{
		UPlayerProfile* V1 = NewObject<UPlayerProfile>(this);
		V1->SaveVersion = 1;
		V1->Name = MigrateKey;
		V1->Level = 7;
		V1->ColorRgb = FLinearColor(0.1f, 0.2f, 0.3f, 1.f);
		SaveBackend->SaveProfile(MigrateKey, V1);
		UPlayerProfile* Loaded = NewObject<UPlayerProfile>(this);
		if (SaveBackend->LoadProfile(MigrateKey, Loaded))
		{
			UE_LOG(LogTemp, Log, TEXT("[GameMode] TestMigrateV1 %s -> SaveVersion=%d Level=%d XP=%d"), *MigrateKey, Loaded->SaveVersion, Loaded->Level, Loaded->XP);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[GameMode] TestMigrateV1 %s load failed"), *MigrateKey);
		}
	}

	// Menu map (client started without IP): no pawn, show the Slate login screen.
	UE_LOG(LogTemp, Log, TEXT("[GameMode] InitGame MapName=%s NetMode=%d"), *MapName, (int32)GetWorld()->GetNetMode());
	if (GetWorld()->GetNetMode() != NM_DedicatedServer && MapName.Contains(TEXT("Menu")))
	{
		DefaultPawnClass = nullptr;
		USalamanderInputLib::ApplyAllBindings();
		ShowLoginMenu();

		// CLI test: -ConnectTo=127.0.0.1 -ConnectNick=Bob -ConnectColor=FF8800 -> auto-connect without click.
		FString ConnectIp;
		if (FParse::Value(FCommandLine::Get(), TEXT("ConnectTo="), ConnectIp) && !ConnectIp.IsEmpty())
		{
			FString ConnectNick, ConnectColor;
			FParse::Value(FCommandLine::Get(), TEXT("ConnectNick="), ConnectNick);
			FParse::Value(FCommandLine::Get(), TEXT("ConnectColor="), ConnectColor);
			ConnectToServer(ConnectIp, ConnectNick, ConnectColor);
		}
	}
}

void ASalamanderGameMode::ShowLoginMenu()
{
	if (GEngine == nullptr || GEngine->GameViewport == nullptr || LoginWidget.IsValid())
	{
		return;
	}
	LoginWidget = SNew(SLoginWidget);
	LoginWidget->SetGameMode(this);
	GEngine->GameViewport->AddViewportWidgetContent(LoginWidget.ToSharedRef(), 100);
	UE_LOG(LogTemp, Log, TEXT("[GameMode] Login menu shown"));
}

void ASalamanderGameMode::HideLoginMenu()
{
	if (GEngine != nullptr && GEngine->GameViewport != nullptr && LoginWidget.IsValid())
	{
		GEngine->GameViewport->RemoveViewportWidgetContent(LoginWidget.ToSharedRef());
		LoginWidget.Reset();
	}
}

void ASalamanderGameMode::ConnectToServer(const FString& Ip, const FString& Nick, const FString& Color)
{
	const FString Url = FString::Printf(TEXT("%s?Nick=%s?Color=%s"), *Ip, *Nick, *Color);
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	UE_LOG(LogTemp, Log, TEXT("[GameMode] Connecting to %s (PC=%d)"), *Url, PC != nullptr);
	HideLoginMenu();
	if (GEngine != nullptr)
	{
		// Standard menu approach: client travel to the URL (IP?options).
		GEngine->SetClientTravel(GetWorld(), *Url, TRAVEL_Absolute);
	}
}

void ASalamanderGameMode::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	Super::PreLogin(Options, Address, UniqueId, ErrorMessage);
	if (!ErrorMessage.IsEmpty())
	{
		return; // already rejected by the parent
	}

	const FString Name = GetNickOption(Options);
	if (Name.IsEmpty())
	{
		ErrorMessage = TEXT("Nick required: open <ip>?Name=Bob?Color=FF8800");
		return;
	}
	if (Name.Len() > 16)
	{
		ErrorMessage = TEXT("Nick too long (16 chars max)");
		return;
	}
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (const ASalamanderPlayerState* PS = (*It)->GetPlayerState<ASalamanderPlayerState>())
		{
			if (PS->PlayerName.Equals(Name, ESearchCase::IgnoreCase))
			{
				ErrorMessage = TEXT("Nick already taken");
				return;
			}
		}
	}
}

APlayerController* ASalamanderGameMode::Login(UPlayer* NewPlayer, ENetRole InRemoteRole, const FString& Portal, const FString& Options, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	APlayerController* PC = Super::Login(NewPlayer, InRemoteRole, Portal, Options, UniqueId, ErrorMessage);
	if (PC == nullptr)
	{
		return nullptr;
	}

	const FString Name = GetNickOption(Options);
	const FString ColorHex = UGameplayStatics::ParseOption(Options, TEXT("Color"));

	if (ASalamanderPlayerState* PS = PC->GetPlayerState<ASalamanderPlayerState>())
	{
		PS->PlayerName = Name;
		if (!ColorHex.IsEmpty())
		{
			PS->ColorRgb = FLinearColor(FColor::FromHex(ColorHex));
		}

		// Load the saved profile (server only): level and color are restored.
		if (SaveBackend != nullptr && !PS->PlayerName.IsEmpty())
		{
			UPlayerProfile* Loaded = NewObject<UPlayerProfile>(this);
			if (SaveBackend->LoadProfile(PS->PlayerName, Loaded))
			{
				PS->Level = Loaded->Level;
				PS->XP = Loaded->XP;
				PS->ColorRgb = Loaded->ColorRgb;
				UE_LOG(LogTemp, Log, TEXT("[GameMode] Loaded profile for %s: Level=%d XP=%d"), *PS->PlayerName, PS->Level, PS->XP);
			}
		}

		PS->ApplyColorToPawn(); // server side (clients get it via OnRep_Color)
		UE_LOG(LogTemp, Log, TEXT("[GameMode] Login Name=%s Color=%s"), *Name, *ColorHex);
	}
	return PC;
}

void ASalamanderGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	UE_LOG(LogTemp, Log, TEXT("[GameMode] PostLogin %s"), *NewPlayer->GetName());

	// Automated CLI tests (server started with -TestAutoDamage=<n> and/or -TestAutoLevel=<n>).
	int32 AutoDamage = 0;
	if (FParse::Value(FCommandLine::Get(), TEXT("TestAutoDamage="), AutoDamage) && AutoDamage > 0)
	{
		const int32 Amount = AutoDamage;
		FTimerHandle Handle;
		GetWorldTimerManager().SetTimer(Handle, [this, Amount]() { ApplyDamageToAll(Amount); }, 10.f, false);
	}

	// CLI test: server-side melee swings (-TestAutoMelee=1) to validate the ability on the server.
	bool bAutoMelee = false;
	if (FParse::Bool(FCommandLine::Get(), TEXT("TestAutoMelee="), bAutoMelee) && bAutoMelee)
	{
		UE_LOG(LogTemp, Log, TEXT("[GameMode] TestAutoMelee armed pawn=%d"), NewPlayer->GetPawn() != nullptr ? 1 : 0);
		FTimerHandle MHandle;
		GetWorldTimerManager().SetTimer(MHandle, [this, NewPlayer]()
		{
			if (ASalamanderCharacter* Char = Cast<ASalamanderCharacter>(NewPlayer->GetPawn()))
			{
				Char->TryMeleeAttack();
			}
			else
			{
				UE_LOG(LogTemp, Log, TEXT("[GameMode] TestAutoMelee: pawn cast failed"));
			}
		}, 3.f, false);
	}

	// CLI test: damage the boss from the server (-TestAutoBossDamage=<n>).
	int32 BossDamage = 0;
	if (FParse::Value(FCommandLine::Get(), TEXT("TestAutoBossDamage="), BossDamage) && BossDamage > 0)
	{
		const int32 Amount = BossDamage;
		UE_LOG(LogTemp, Log, TEXT("[GameMode] TestAutoBossDamage armed amount=%d"), Amount);
		FTimerHandle BH;
		GetWorldTimerManager().SetTimer(BH, [this, Amount]()
		{
			int32 Found = 0;
			for (TActorIterator<ABossCharacter> It(GetWorld()); It; ++It)
			{
				++Found;
				if (UAbilitySystemComponent* BossASC = It->AbilitySystemComponent)
				{
					UGE_Damage::ApplyTo(BossASC, static_cast<float>(Amount));
					UE_LOG(LogTemp, Log, TEXT("[GameMode] TestAutoBossDamage %d on %s"), Amount, *It->GetName());
				}
			}
			if (Found == 0)
			{
				UE_LOG(LogTemp, Warning, TEXT("[GameMode] TestAutoBossDamage: no boss in world"));
			}
		}, 5.f, true);
	}

	int32 AutoLevel = 0;
	if (FParse::Value(FCommandLine::Get(), TEXT("TestAutoLevel="), AutoLevel) && AutoLevel > 0)
	{
		if (ASalamanderPlayerState* PS = NewPlayer->GetPlayerState<ASalamanderPlayerState>())
		{
			const int32 Level = AutoLevel;
			const FString Name = PS->PlayerName;
			FTimerHandle Handle;
			GetWorldTimerManager().SetTimer(Handle, [this, Name, Level]()
			{
				for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
				{
					if (ASalamanderPlayerState* Other = (*It)->GetPlayerState<ASalamanderPlayerState>())
					{
						if (Other->PlayerName == Name)
						{
							Other->Level = Level;
							SaveProfileFor(Other);
						}
					}
				}
			}, 10.f, false);
		}
	}
}

void ASalamanderGameMode::Logout(AController* Exiting)
{
	// Save on disconnect.
	if (Exiting != nullptr && Exiting->IsPlayerController())
	{
		if (const ASalamanderPlayerState* PS = Exiting->GetPlayerState<ASalamanderPlayerState>())
		{
			SaveProfileFor(PS);
		}
	}
	Super::Logout(Exiting);
}

void ASalamanderGameMode::SaveProfileFor(const ASalamanderPlayerState* PS)
{
	if (SaveBackend == nullptr || PS == nullptr || PS->PlayerName.IsEmpty())
	{
		return;
	}

	UPlayerProfile* Profile = NewObject<UPlayerProfile>(this);
	Profile->Name = PS->PlayerName;
	Profile->ColorRgb = PS->ColorRgb;
	Profile->Level = PS->Level;
	Profile->XP = PS->XP;

	if (SaveBackend->SaveProfile(PS->PlayerName, Profile))
	{
		UE_LOG(LogTemp, Log, TEXT("[GameMode] Saved profile: %s (Level=%d)"), *PS->PlayerName, PS->Level);
	}
}

void ASalamanderGameMode::SaveAllProfiles()
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (const ASalamanderPlayerState* PS = (*It)->GetPlayerState<ASalamanderPlayerState>())
		{
			SaveProfileFor(PS);
		}
	}
}

void ASalamanderGameMode::ApplyDamageToAll(int32 Amount)
{
	// Damage via GameplayEffect (GE_Damage, SetByCaller Data.Damage), applied on the server only.
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (ASalamanderPlayerState* PS = (*It)->GetPlayerState<ASalamanderPlayerState>())
		{
			if (UAbilitySystemComponent* ASC = PS->GetASC())
			{
				FGameplayEffectContextHandle Ctx = ASC->MakeEffectContext();
				// Set the magnitude on the GE default object before making the spec (debug path).
				if (UGE_Damage* Def = Cast<UGE_Damage>(UGE_Damage::StaticClass()->GetDefaultObject()))
				{
					for (FGameplayModifierInfo& Mod : Def->Modifiers)
					{
						if (Mod.Attribute == USalamanderAttributeSet::GetHealthAttribute())
						{
							Mod.ModifierMagnitude = FScalableFloat(-static_cast<float>(Amount));
						}
					}
				}
				FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(UGE_Damage::StaticClass(), 1.f, Ctx);
				if (Spec.IsValid())
				{
					ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
				}
			}
		}
	}
}

void ASalamanderGameMode::DebugDamage(int32 Amount)
{
	// Server console only (dedicated server console).
	ApplyDamageToAll(Amount);
}

void ASalamanderGameMode::DebugSetLevel(const FString& Name, int32 NewLevel)
{
	// Server console only (dedicated server console).
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (ASalamanderPlayerState* PS = (*It)->GetPlayerState<ASalamanderPlayerState>())
		{
			if (PS->PlayerName.Equals(Name, ESearchCase::IgnoreCase))
			{
				PS->Level = NewLevel;
				SaveProfileFor(PS);
				UE_LOG(LogTemp, Log, TEXT("[GameMode] DebugSetLevel %s -> %d"), *Name, NewLevel);
			}
		}
	}
}

void ASalamanderGameMode::HandlePlayerDeath(APawn* Pawn)
{
	AController* Controller = Pawn != nullptr ? Pawn->GetController() : nullptr;
	if (ASalamanderPlayerState* PS = Controller != nullptr ? Controller->GetPlayerState<ASalamanderPlayerState>() : nullptr)
	{
		UE_LOG(LogTemp, Log, TEXT("[GameMode] %s died (HP=0)"), *PS->PlayerName);
	}

	// Mark the player as dead (GameplayTag State.Dead, replicated) -> blocks inputs.
	if (ASalamanderPlayerState* PS = Controller != nullptr ? Controller->GetPlayerState<ASalamanderPlayerState>() : nullptr)
	{
		if (UAbilitySystemComponent* ASC = PS->GetASC())
		{
			ASC->AddLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("State.Dead")));
		}
	}

	const float Delay = DeathRules != nullptr ? DeathRules->RespawnDelay : 3.f; // PLACEHOLDER value inside the DA
	if (Controller != nullptr)
	{
		Controller->UnPossess();
		TWeakObjectPtr<AController> WeakController = Controller;
		FTimerHandle Handle;
		GetWorldTimerManager().SetTimer(Handle, [this, WeakController]()
		{
			if (AController* C = WeakController.Get())
			{
				// Respawn at the start point; the new pawn comes back with full HP (no penalty).
				RestartPlayer(C);
				if (ASalamanderPlayerState* PS = C->GetPlayerState<ASalamanderPlayerState>())
				{
					if (UAbilitySystemComponent* ASC = PS->GetASC())
					{
						ASC->RemoveLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("State.Dead")));
					}
					UE_LOG(LogTemp, Log, TEXT("[GameMode] %s respawned at start point"), *PS->PlayerName);
				}
			}
		}, Delay, false);
	}

	if (Pawn != nullptr)
	{
		Pawn->Destroy();
	}
}


void ASalamanderGameMode::OnBossKilled(ABossCharacter* Boss)
{
	UE_LOG(LogTemp, Log, TEXT("[GameMode] boss %s killed -> reward to all players"), Boss ? *Boss->GetName() : TEXT("?"));

	// Reward from the Data Table DT_BossReward (row "Boss1").
	UDataTable* RewardTable = LoadObject<UDataTable>(nullptr, TEXT("/Game/Data/DT_BossReward"));
	const FRewardRow* Row = RewardTable != nullptr ? RewardTable->FindRow<FRewardRow>(FName("Boss1"), TEXT("BossReward")) : nullptr;
	const int32 XPReward = Row ? Row->XP : 100;      // PLACEHOLDER fallback
	const FString ItemName = Row ? Row->ItemName : TEXT("FictifSword"); // PLACEHOLDER fallback

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (ASalamanderPlayerState* PS = (*It)->GetPlayerState<ASalamanderPlayerState>())
		{
			PS->XP += XPReward;
			SaveProfileFor(PS); // XP recorded in the saved profile
			UE_LOG(LogTemp, Log, TEXT("[GameMode] reward: %s +%d XP (item %s), total XP=%d"), *PS->PlayerName, XPReward, *ItemName, PS->XP);
		}
	}
}
