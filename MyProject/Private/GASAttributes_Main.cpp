// Fill out your copyright notice in the Description page of Project Settings.

#include "GASAttributes_Main.h"
#include "Net/UnrealNetwork.h"


UGASAttributes_Main::UGASAttributes_Main()
{
}

void UGASAttributes_Main::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UGASAttributes_Main, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGASAttributes_Main, Health_regen, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGASAttributes_Main, Health_max, COND_AutonomousOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGASAttributes_Main, Energy, COND_AutonomousOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGASAttributes_Main, Energy_regen, COND_AutonomousOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGASAttributes_Main, Energy_max, COND_AutonomousOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGASAttributes_Main, EnergyCharges, COND_AutonomousOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGASAttributes_Main, EnergyCharges_max, COND_AutonomousOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGASAttributes_Main, Power, COND_AutonomousOnly, REPNOTIFY_Always);
}

void UGASAttributes_Main::OnRep_Health(const FGameplayAttributeData& previous_attribute_value)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGASAttributes_Main, Health, previous_attribute_value);
}

void UGASAttributes_Main::OnRep_Health_regen(const FGameplayAttributeData& previous_attribute_value)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGASAttributes_Main, Health_regen, previous_attribute_value);
}

void UGASAttributes_Main::OnRep_Health_max(const FGameplayAttributeData& previous_attribute_value)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGASAttributes_Main, Health_max, previous_attribute_value);
}

void UGASAttributes_Main::OnRep_Energy(const FGameplayAttributeData& previous_attribute_value)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGASAttributes_Main, Energy, previous_attribute_value);
}

void UGASAttributes_Main::OnRep_Energy_regen(const FGameplayAttributeData& previous_attribute_value)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGASAttributes_Main, Energy_regen, previous_attribute_value);
}

void UGASAttributes_Main::OnRep_Energy_max(const FGameplayAttributeData& previous_attribute_value)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGASAttributes_Main, Energy_max, previous_attribute_value);
}

void UGASAttributes_Main::OnRep_EnergyCharges(const FGameplayAttributeData& previous_attribute_value)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGASAttributes_Main, EnergyCharges, previous_attribute_value);
}

void UGASAttributes_Main::OnRep_EnergyCharges_max(const FGameplayAttributeData& previous_attribute_value)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGASAttributes_Main, EnergyCharges_max, previous_attribute_value);
}

void UGASAttributes_Main::OnRep_Power(const FGameplayAttributeData& previous_attribute_value)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGASAttributes_Main, Power, previous_attribute_value);
}
