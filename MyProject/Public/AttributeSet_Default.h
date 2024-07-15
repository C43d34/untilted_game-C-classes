// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"


#include "AttributeSet_Default.generated.h"


#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * 
 */
UCLASS()
class MYPROJECT_API UAttributeSet_Default : public UAttributeSet
{
	GENERATED_BODY()


	UPROPERTY()
	FGameplayAttributeData health = 100.0f;
	ATTRIBUTE_ACCESSORS(UAttributeSet_Default, health)

	UPROPERTY()
	FGameplayAttributeData boost_charge_count = 1.0f;
	ATTRIBUTE_ACCESSORS(UAttributeSet_Default, boost_charge_count)

};
