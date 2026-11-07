#include "SalamanderGameMode.h"

#include "Engine/EngineTypes.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Math/Color.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "SalamanderCharacter.h"
#include "SalamanderPlayerState.h"
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

	// Test CLI automatise : si le serveur est lance avec -TestAutoDamage=<n>,
	// applique <n> degats a tous les joueurs 10 s apres chaque connexion.
	int32 AutoDamage = 0;
	if (FParse::Value(FCommandLine::Get(), TEXT("TestAutoDamage="), AutoDamage) && AutoDamage > 0)
	{
		const int32 Amount = AutoDamage;
		FTimerHandle Handle;
		GetWorldTimerManager().SetTimer(Handle, [this, Amount]()
		{
			ApplyDamageToAll(Amount);
		}, 10.f, false);
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
