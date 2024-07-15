// Fill out your copyright notice in the Description page of Project Settings.

#include "GASAttributes_Secondary.h"
#include "Net/UnrealNetwork.h"


UGASAttributes_Secondary::UGASAttributes_Secondary()
{
}

void UGASAttributes_Secondary::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UGASAttributes_Secondary, Durability, COND_AutonomousOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGASAttributes_Secondary, Lethality, COND_AutonomousOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGASAttributes_Secondary, Lethality_weaponfirerate, COND_AutonomousOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGASAttributes_Secondary, Lethality_weaponreload, COND_AutonomousOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGASAttributes_Secondary, Lethality_bulletvelocity, COND_AutonomousOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGASAttributes_Secondary, Alacrity, COND_AutonomousOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGASAttributes_Secondary, Alacrity_energyregenspeedup, COND_AutonomousOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGASAttributes_Secondary, Alacrity_cooldownspeedup, COND_AutonomousOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGASAttributes_Secondary, Alacrity_healthregenspeedup, COND_AutonomousOnly, REPNOTIFY_Always);
}

void UGASAttributes_Secondary::OnRep_Durability(const FGameplayAttributeData& previous_attribute_value)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGASAttributes_Secondary, Durability, previous_attribute_value);
}

void UGASAttributes_Secondary::OnRep_Lethality(const FGameplayAttributeData& previous_attribute_value)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGASAttributes_Secondary, Lethality, previous_attribute_value);
}

void UGASAttributes_Secondary::OnRep_Lethality_weaponfirerate(const FGameplayAttributeData& previous_attribute_value)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGASAttributes_Secondary, Lethality_weaponfirerate, previous_attribute_value);
}

void UGASAttributes_Secondary::OnRep_Lethality_weaponreload(const FGameplayAttributeData& previous_attribute_value)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGASAttributes_Secondary, Lethality_weaponreload, previous_attribute_value);
}

void UGASAttributes_Secondary::OnRep_Lethality_bulletvelocity(const FGameplayAttributeData& previous_attribute_value)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGASAttributes_Secondary, Lethality_bulletvelocity, previous_attribute_value);
}

void UGASAttributes_Secondary::OnRep_Alacrity(const FGameplayAttributeData& previous_attribute_value)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGASAttributes_Secondary, Alacrity, previous_attribute_value);
}

void UGASAttributes_Secondary::OnRep_Alacrity_energyregenspeedup(const FGameplayAttributeData& previous_attribute_value)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGASAttributes_Secondary, Alacrity_energyregenspeedup, previous_attribute_value);
}

void UGASAttributes_Secondary::OnRep_Alacrity_cooldownspeedup(const FGameplayAttributeData& previous_attribute_value)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGASAttributes_Secondary, Alacrity_cooldownspeedup, previous_attribute_value);
}

void UGASAttributes_Secondary::OnRep_Alacrity_healthregenspeedup(const FGameplayAttributeData& previous_attribute_value)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGASAttributes_Secondary, Alacrity_healthregenspeedup, previous_attribute_value);
}
