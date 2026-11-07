#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StatsComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SALAMANDER_API UStatsComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UStatsComponent();

	// Santé courante. Seul le serveur la modifie (ApplyDamage), répliquée vers tous les clients.
	UPROPERTY(ReplicatedUsing=OnRep_Health, BlueprintReadOnly, Category="Stats")
	int32 Health;

	UPROPERTY(BlueprintReadOnly, Category="Stats")
	int32 MaxHealth;

	// ServerOnly : sans autorité, no-op.
	UFUNCTION(BlueprintCallable, Category="Stats")
	void ApplyDamage(int32 Amount);

	UFUNCTION()
	void OnRep_Health(int32 OldHealth);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;
};
