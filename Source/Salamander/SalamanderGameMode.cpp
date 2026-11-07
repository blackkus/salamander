#include "SalamanderGameMode.h"

#include "Engine/Engine.h"
#include "Engine/EngineTypes.h"
#include "Engine/GameViewportClient.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "SLoginWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Math/Color.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "FileSaveBackend.h"
#include "PlayerProfile.h"
#include "SalamanderCharacter.h"
#include "SalamanderPlayerState.h"
#include "SaveBackend.h"
#include "StatsComponent.h"
#include "TimerManager.h"

// En CLI (-game), le moteur remplace l'option Name par le surnom local ; on lit donc
// l'option "Pseudo" en priorite (passe par start_client.bat), "Name" en fallback (éditeur).
static FString GetPseudoOption(const FString& Options)
{
	if (UGameplayStatics::HasOption(Options, TEXT("Pseudo")))
	{
		return UGameplayStatics::ParseOption(Options, TEXT("Pseudo"));
	}
	return UGameplayStatics::ParseOption(Options, TEXT("Name"));
}

ASalamanderGameMode::ASalamanderGameMode()
{
	PlayerStateClass = ASalamanderPlayerState::StaticClass();
	DefaultPawnClass = ASalamanderCharacter::StaticClass();

	// Le PlayerController reste celui du template Third Person (Blueprint non modifie).
	static ConstructorHelpers::FClassFinder<AController> PCBP(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonPlayerController"));
	if (PCBP.Succeeded())
	{
		PlayerControllerClass = PCBP.Class;
	}
}

void ASalamanderGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	// Backend de sauvegarde : fichiers sur le disque du serveur, un par joueur.
	SaveBackendObj = NewObject<UFileSaveBackend>(this);
	SaveBackend = Cast<ISaveBackend>(SaveBackendObj.Get());

	// Sauvegarde periodique toutes les 60 s.
	GetWorldTimerManager().SetTimer(SaveTimerHandle, this, &ASalamanderGameMode::SaveAllProfiles, 60.f, true);

	// Map Menu (client lance sans IP) : pas de pawn, on affiche l'ecran de connexion Slate.
	UE_LOG(LogTemp, Log, TEXT("[GameMode] InitGame MapName=%s NetMode=%d"), *MapName, (int32)GetWorld()->GetNetMode());
	if (GetWorld()->GetNetMode() != NM_DedicatedServer && MapName.Contains(TEXT("Menu")))
	{
		DefaultPawnClass = nullptr;
		ShowLoginMenu();

		// Test CLI : -ConnectTo=127.0.0.1 -ConnectPseudo=Bob -ConnectColor=FF8800 -> connexion auto sans clic.
		FString ConnectIp;
		if (FParse::Value(FCommandLine::Get(), TEXT("ConnectTo="), ConnectIp) && !ConnectIp.IsEmpty())
		{
			FString ConnectPseudo, ConnectColor;
			FParse::Value(FCommandLine::Get(), TEXT("ConnectPseudo="), ConnectPseudo);
			FParse::Value(FCommandLine::Get(), TEXT("ConnectColor="), ConnectColor);
			ConnectToServer(ConnectIp, ConnectPseudo, ConnectColor);
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
	UE_LOG(LogTemp, Log, TEXT("[GameMode] Menu de connexion affiche"));
}

void ASalamanderGameMode::HideLoginMenu()
{
	if (GEngine != nullptr && GEngine->GameViewport != nullptr && LoginWidget.IsValid())
	{
		GEngine->GameViewport->RemoveViewportWidgetContent(LoginWidget.ToSharedRef());
		LoginWidget.Reset();
	}
}

void ASalamanderGameMode::ConnectToServer(const FString& Ip, const FString& Pseudo, const FString& Color)
{
	const FString Url = FString::Printf(TEXT("%s?Pseudo=%s?Color=%s"), *Ip, *Pseudo, *Color);
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	UE_LOG(LogTemp, Log, TEXT("[GameMode] Connexion a %s (PC=%d)"), *Url, PC != nullptr);
	HideLoginMenu();
	if (GEngine != nullptr)
	{
		// Méthode standard des menus : travel client vers l'URL (IP?options).
		GEngine->SetClientTravel(GetWorld(), *Url, TRAVEL_Absolute);
	}
}

void ASalamanderGameMode::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	Super::PreLogin(Options, Address, UniqueId, ErrorMessage);
	if (!ErrorMessage.IsEmpty())
	{
		return; // deja refuse par le parent
	}

	const FString Name = GetPseudoOption(Options);
	if (Name.IsEmpty())
	{
		ErrorMessage = TEXT("Pseudo requis : open <ip>?Name=Bob?Color=FF8800");
		return;
	}
	if (Name.Len() > 16)
	{
		ErrorMessage = TEXT("Pseudo trop long (16 caracteres max)");
		return;
	}
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (const ASalamanderPlayerState* PS = (*It)->GetPlayerState<ASalamanderPlayerState>())
		{
			if (PS->PlayerName.Equals(Name, ESearchCase::IgnoreCase))
			{
				ErrorMessage = TEXT("Pseudo deja pris");
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

	const FString Name = GetPseudoOption(Options);
	const FString ColorHex = UGameplayStatics::ParseOption(Options, TEXT("Color"));

	if (ASalamanderPlayerState* PS = PC->GetPlayerState<ASalamanderPlayerState>())
	{
		PS->PlayerName = Name;
		if (!ColorHex.IsEmpty())
		{
			PS->ColorRgb = FLinearColor(FColor::FromHex(ColorHex));
		}

		// Charge le profil sauvegarde (serveur uniquement) : niveau et couleur sont restaures.
		if (SaveBackend != nullptr && !PS->PlayerName.IsEmpty())
		{
			UPlayerProfile* Loaded = NewObject<UPlayerProfile>(this);
			if (SaveBackend->LoadProfile(PS->PlayerName, Loaded))
			{
				PS->Level = Loaded->Level;
				PS->ColorRgb = Loaded->ColorRgb;
				UE_LOG(LogTemp, Log, TEXT("[GameMode] Profil charge pour %s: Level=%d"), *PS->PlayerName, PS->Level);
			}
		}

		PS->ApplyColorToPawn(); // cote serveur (les clients recoivent via OnRep_Color)
		UE_LOG(LogTemp, Log, TEXT("[GameMode] Login Name=%s Color=%s"), *Name, *ColorHex);
	}
	return PC;
}

void ASalamanderGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	// Le composant de stats est cree et attache cote serveur uniquement (AddInstanceComponent
	// pour qu'il soit pris en compte par la replication) ; il est replique vers les clients.
	if (APawn* Pawn = NewPlayer->GetPawn())
	{
		if (Pawn->FindComponentByClass<UStatsComponent>() == nullptr)
		{
			UStatsComponent* Stats = NewObject<UStatsComponent>(Pawn, TEXT("StatsComponent"));
			Pawn->AddInstanceComponent(Stats);
			Stats->RegisterComponent();
		}
	}

	UE_LOG(LogTemp, Log, TEXT("[GameMode] PostLogin %s"), *NewPlayer->GetName());

	// Tests CLI automatises (serveur lance avec -TestAutoDamage=<n> et/ou -TestAutoLevel=<n>).
	int32 AutoDamage = 0;
	if (FParse::Value(FCommandLine::Get(), TEXT("TestAutoDamage="), AutoDamage) && AutoDamage > 0)
	{
		const int32 Amount = AutoDamage;
		FTimerHandle Handle;
		GetWorldTimerManager().SetTimer(Handle, [this, Amount]() { ApplyDamageToAll(Amount); }, 10.f, false);
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
	// Sauvegarde a la deconnexion.
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

	if (SaveBackend->SaveProfile(PS->PlayerName, Profile))
	{
		UE_LOG(LogTemp, Log, TEXT("[GameMode] Profil sauvegarde: %s (Level=%d)"), *PS->PlayerName, PS->Level);
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
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (APawn* Pawn = It->Get()->GetPawn())
		{
			if (UStatsComponent* Stats = Pawn->FindComponentByClass<UStatsComponent>())
			{
				Stats->ApplyDamage(Amount);
			}
		}
	}
}

void ASalamanderGameMode::DebugDamage(int32 Amount)
{
	// Execute uniquement sur le serveur (console du serveur dedie).
	ApplyDamageToAll(Amount);
}

void ASalamanderGameMode::DebugSetLevel(const FString& Name, int32 NewLevel)
{
	// Execute uniquement sur le serveur (console du serveur dedie).
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
