// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayAbilitySpec.h" // for FGameplayAbilitySpecHandle
#include "GameAbilityInputSetAsset.generated.h"



class UInputAction;

USTRUCT()
struct FGameAbilityInputMapping
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category = "GameplayInputAbilityInfo")
	TSubclassOf<UGameplayAbility> GameplayAbilityClass;

	UPROPERTY(Transient, EditAnywhere, Category = "GameplayInputAbilityInfo")
	TObjectPtr<UInputAction> InputAction;

	/** It will be generated automatically. */
	UPROPERTY(VisibleAnywhere, Category = "GameplayInputAbilityInfo")
	int32 InputID;

	bool IsValid() const
	{
		return GameplayAbilityClass && InputAction;
	}

	bool operator==(const FGameAbilityInputMapping& Other) const
	{
		return GameplayAbilityClass == Other.GameplayAbilityClass && InputAction == Other.InputAction;
	}

	bool operator!=(const FGameAbilityInputMapping& Other) const
	{
		return !operator==(Other);
	}

	friend uint32 GetTypeHash(const FGameAbilityInputMapping& Item)
	{
		return HashCombine(GetTypeHash(Item.GameplayAbilityClass), GetTypeHash(Item.InputAction));
	}
};


/**
 * 
 */
UCLASS()
class MYPROJECT_API UGameAbilityInputSetAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, Category = "AbilitySystem")
	TSet<FGameAbilityInputMapping> game_abilities_with_mapped_input_set;

public:
	const TSet<FGameAbilityInputMapping>& GetAbilitiesWithInputsSet() const;


	/*
	* Binds the abiltiies to the appropriate input with the specified ability system component & input system.
	*/
	UFUNCTION(BlueprintCallable, Category = "AbilitySystem")
	void AddAbilitiesToAbilityComponent(UAbilitySystemComponent* ability_sys_component);


#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
