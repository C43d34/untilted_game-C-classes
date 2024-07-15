// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "GASAttributes_Secondary.generated.h"



#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)
/**
 * 
 */
UCLASS()
class MYPROJECT_API UGASAttributes_Secondary : public UAttributeSet
{
	GENERATED_BODY()
	
public:
	UGASAttributes_Secondary(); //default constructor

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

//DURABILITY (Damage resistance, contact damage, knockback resistance)
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Secondary", ReplicatedUsing = OnRep_Durability)
	FGameplayAttributeData Durability;
	ATTRIBUTE_ACCESSORS(UGASAttributes_Secondary, Durability);

	UFUNCTION()
	virtual void OnRep_Durability(const FGameplayAttributeData& previous_attribute_value);

//LETHALITY
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Secondary", ReplicatedUsing = OnRep_Lethality)
	FGameplayAttributeData Lethality;
	ATTRIBUTE_ACCESSORS(UGASAttributes_Secondary, Lethality);

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Secondary", ReplicatedUsing = OnRep_Lethality_weaponfirerate)
	FGameplayAttributeData Lethality_weaponfirerate;
	ATTRIBUTE_ACCESSORS(UGASAttributes_Secondary, Lethality_weaponfirerate);

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Secondary", ReplicatedUsing = OnRep_Lethality_weaponreload)
	FGameplayAttributeData Lethality_weaponreload;
	ATTRIBUTE_ACCESSORS(UGASAttributes_Secondary, Lethality_weaponreload);

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Secondary", ReplicatedUsing = OnRep_Lethality_bulletvelocity)
	FGameplayAttributeData Lethality_bulletvelocity;
	ATTRIBUTE_ACCESSORS(UGASAttributes_Secondary, Lethality_bulletvelocity);

	UFUNCTION()
	virtual void OnRep_Lethality(const FGameplayAttributeData& previous_attribute_value);

	UFUNCTION()
	virtual void OnRep_Lethality_weaponfirerate(const FGameplayAttributeData& previous_attribute_value);

	UFUNCTION()
	virtual void OnRep_Lethality_weaponreload(const FGameplayAttributeData& previous_attribute_value);

	UFUNCTION()
	virtual void OnRep_Lethality_bulletvelocity(const FGameplayAttributeData& previous_attribute_value);


//ALACRITY
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Secondary", ReplicatedUsing = OnRep_Alacrity)
	FGameplayAttributeData Alacrity;
	ATTRIBUTE_ACCESSORS(UGASAttributes_Secondary, Alacrity);

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Secondary", ReplicatedUsing = OnRep_Alacrity_energyregenspeedup)
	FGameplayAttributeData Alacrity_energyregenspeedup;
	ATTRIBUTE_ACCESSORS(UGASAttributes_Secondary, Alacrity_energyregenspeedup);

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Secondary", ReplicatedUsing = OnRep_Alacrity_cooldownspeedup)
	FGameplayAttributeData Alacrity_cooldownspeedup;
	ATTRIBUTE_ACCESSORS(UGASAttributes_Secondary, Alacrity_cooldownspeedup);

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Secondary", ReplicatedUsing = OnRep_Alacrity_healthregenspeedup)
	FGameplayAttributeData Alacrity_healthregenspeedup;
	ATTRIBUTE_ACCESSORS(UGASAttributes_Secondary, Alacrity_healthregenspeedup);

	UFUNCTION()
	virtual void OnRep_Alacrity(const FGameplayAttributeData& previous_attribute_value);

	UFUNCTION()
	virtual void OnRep_Alacrity_energyregenspeedup(const FGameplayAttributeData& previous_attribute_value);

	UFUNCTION()
	virtual void OnRep_Alacrity_cooldownspeedup(const FGameplayAttributeData& previous_attribute_value);

	UFUNCTION()
	virtual void OnRep_Alacrity_healthregenspeedup(const FGameplayAttributeData& previous_attribute_value);

};
