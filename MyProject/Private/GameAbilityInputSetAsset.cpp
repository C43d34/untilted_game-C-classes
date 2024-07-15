// Fill out your copyright notice in the Description page of Project Settings.


#include "GameAbilityInputSetAsset.h"


const TSet<FGameAbilityInputMapping>& UGameAbilityInputSetAsset::GetAbilitiesWithInputsSet() const
{
	return game_abilities_with_mapped_input_set;
}



void UGameAbilityInputSetAsset::AddAbilitiesToAbilityComponent(UAbilitySystemComponent* ability_sys_component)
{
}



#if WITH_EDITOR
void UGameAbilityInputSetAsset::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FProperty* Property = PropertyChangedEvent.Property;
	if (Property && Property->GetFName() == GET_MEMBER_NAME_CHECKED(UGameAbilityInputSetAsset, game_abilities_with_mapped_input_set) && !game_abilities_with_mapped_input_set.IsEmpty())
	{
		TArray<FGameAbilityInputMapping> InputAbilitiesArray = game_abilities_with_mapped_input_set.Array();
		game_abilities_with_mapped_input_set.Reset();

		for (int32 i = 0; i < InputAbilitiesArray.Num(); ++i)
		{
			FGameAbilityInputMapping& ability_map = InputAbilitiesArray[i];
			ability_map.InputID = i;
			game_abilities_with_mapped_input_set.Add(ability_map);
		}
	}
}
#endif
