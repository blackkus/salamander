#include "SalamanderGameMode.h"

#include "GameFramework/Pawn.h"
#include "SalamanderCharacter.h"
#include "SalamanderPlayerState.h"
#include "StatsComponent.h"

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

void ASalamanderGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	// Le composant de stats est cree et attache cote serveur uniquement ;
	// il est replique automatiquement vers les clients.
	if (APawn* Pawn = NewPlayer->GetPawn())
	{
		if (Pawn->FindComponentByClass<UStatsComponent>() == nullptr)
		{
			UStatsComponent* Stats = NewObject<UStatsComponent>(Pawn, TEXT("StatsComponent"));
			Stats->RegisterComponent();
		}
	}

	UE_LOG(LogTemp, Log, TEXT("[GameMode] PostLogin %s"), *NewPlayer->GetName());
}

void ASalamanderGameMode::DebugDamage(int32 Amount)
{
	// Execute uniquement sur le serveur (console du serveur dedie).
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
