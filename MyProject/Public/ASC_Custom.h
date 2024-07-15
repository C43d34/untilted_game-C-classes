// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include <GameplayEffectTypes.h>

#include "ASC_Custom.generated.h"

/**
 * 
 */
UCLASS()
class MYPROJECT_API UASC_Custom : public UAbilitySystemComponent
{
	GENERATED_BODY()
	
private:

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	//Internal attribute sets that will always be included as apart of this ASC instance. Only really useful in C++ when attaching an ASC component to an actor in said C++ environment
	TArray<TSubclassOf<UAttributeSet>> internal_default_attribute_sets;
public:
	UASC_Custom();


	/*
	* Defines a number of attribute sets to appear on this ability system components DefaultStartingData array. (Attribute set present in this array are automatically instantiated on its ability system component. 
	*/
	void SetDefaultAttributeSets(TArray<TSubclassOf<UAttributeSet>>& attribute_sets);

};
