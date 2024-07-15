// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "GASAttributes_Main.generated.h"



#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)
/**
 * 
 */
UCLASS()
class MYPROJECT_API UGASAttributes_Main : public UAttributeSet
{
	GENERATED_BODY()
	
public:
	UGASAttributes_Main(); //default constructor

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

//Define generic attributes that will probably be used across all actors

//HEALTH
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Main", ReplicatedUsing = OnRep_Health)
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UGASAttributes_Main, Health);

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Main", ReplicatedUsing = OnRep_Health_regen)
	FGameplayAttributeData Health_regen;
	ATTRIBUTE_ACCESSORS(UGASAttributes_Main, Health_regen);

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Main", ReplicatedUsing = OnRep_Health_max)
	FGameplayAttributeData Health_max;
	ATTRIBUTE_ACCESSORS(UGASAttributes_Main, Health_max);

	UFUNCTION()
	virtual void OnRep_Health(const FGameplayAttributeData& previous_attribute_value);

	UFUNCTION()
	virtual void OnRep_Health_regen(const FGameplayAttributeData& previous_attribute_value);
	
	UFUNCTION()
	virtual void OnRep_Health_max(const FGameplayAttributeData& previous_attribute_value);


//ENERGY (generic resource used to activate abilities)
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Main", ReplicatedUsing = OnRep_Energy)
	FGameplayAttributeData Energy;
	ATTRIBUTE_ACCESSORS(UGASAttributes_Main, Energy);

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Main", ReplicatedUsing = OnRep_Energy_regen)
	FGameplayAttributeData Energy_regen;
	ATTRIBUTE_ACCESSORS(UGASAttributes_Main, Energy_regen);

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Main", ReplicatedUsing = OnRep_Energy_max)
	FGameplayAttributeData Energy_max;
	ATTRIBUTE_ACCESSORS(UGASAttributes_Main, Energy_max);

	UFUNCTION()
	virtual void OnRep_Energy(const FGameplayAttributeData& previous_attribute_value);

	UFUNCTION()
	virtual void OnRep_Energy_regen(const FGameplayAttributeData& previous_attribute_value);

	UFUNCTION()
	virtual void OnRep_Energy_max(const FGameplayAttributeData& previous_attribute_value);

//ENERGYCHARGES (alternative representation of energy in discrete chunks (typically used alongside energy attribute))
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Main", ReplicatedUsing = OnRep_EnergyCharges)
	FGameplayAttributeData EnergyCharges;
	ATTRIBUTE_ACCESSORS(UGASAttributes_Main, EnergyCharges);

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Main", ReplicatedUsing = OnRep_EnergyCharges_max)
	FGameplayAttributeData EnergyCharges_max;
	ATTRIBUTE_ACCESSORS(UGASAttributes_Main, EnergyCharges_max);

	UFUNCTION()
	virtual void OnRep_EnergyCharges(const FGameplayAttributeData& previous_attribute_value);

	UFUNCTION()
	virtual void OnRep_EnergyCharges_max(const FGameplayAttributeData& previous_attribute_value);

//POWER (main stat used for calculating damage packets made by instigator) 
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Main", ReplicatedUsing = OnRep_Power)
	FGameplayAttributeData Power;
	ATTRIBUTE_ACCESSORS(UGASAttributes_Main, Power);

	UFUNCTION()
	virtual void OnRep_Power(const FGameplayAttributeData& previous_attribute_value);

};
