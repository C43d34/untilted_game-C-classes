// Fill out your copyright notice in the Description page of Project Settings.


#include "ASC_Custom.h"


#if WITH_EDITOR
void UASC_Custom::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	FName PropertyName = (PropertyChangedEvent.Property != NULL) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	FName Property_member_name = PropertyChangedEvent.MemberProperty ? PropertyChangedEvent.MemberProperty->GetFName() : NAME_None;

	if ((Property_member_name == (GET_MEMBER_NAME_CHECKED(UAbilitySystemComponent, DefaultStartingData))))
	{
	//Add default attribute sets to defaultstartingdata if thye aren't already apart of that list 

		TArray<TSubclassOf<UAttributeSet>> default_attribute_sets_to_add = this->internal_default_attribute_sets;
		
		//Scan DefaultStartingData for any attribute sets that it is supposed to have,
		// if it already has one of these sets, remove it from the list of sets to add	
		for (FAttributeDefaults attribute_set_struct: this->DefaultStartingData)
		{
			TSubclassOf<UAttributeSet> attribute_set_class = attribute_set_struct.Attributes;
			if (default_attribute_sets_to_add.Contains(attribute_set_class)){
				default_attribute_sets_to_add.Remove(attribute_set_class);
			}
		}
		
		//add sets to DefaultStartingData
		//UE_LOG(LogTemp, Warning, TEXT("default attribute sets to re-include = % d"), default_attribute_sets_to_add.Num());
		//UE_LOG(LogTemp, Warning, TEXT("default attribute sets that must be incldued overall % d"), this->internal_default_attribute_sets.Num());

		for (TSubclassOf<UAttributeSet> missing_attribute_set : default_attribute_sets_to_add)
		{
			FAttributeDefaults attribute_set_class;
			attribute_set_class.Attributes = missing_attribute_set;
			attribute_set_class.DefaultStartingTable = nullptr;
			this->DefaultStartingData.Add(attribute_set_class);
		}

	}
	Super::PostEditChangeProperty(PropertyChangedEvent);

}
#endif


UASC_Custom::UASC_Custom()
{
	if (!HasAnyFlags(RF_ClassDefaultObject) && this->internal_default_attribute_sets.Num() > 0)
	{
		for (TSubclassOf<UAttributeSet> attribute_set_to_add_on_component_construction : this->internal_default_attribute_sets)
		{
			FAttributeDefaults attribute_set_class;
			attribute_set_class.Attributes = attribute_set_to_add_on_component_construction;
			attribute_set_class.DefaultStartingTable = nullptr;
			this->DefaultStartingData.Add(attribute_set_class);
		}
	}
}


void UASC_Custom::SetDefaultAttributeSets(TArray<TSubclassOf<UAttributeSet>>& attribute_sets)
{
	this->internal_default_attribute_sets = attribute_sets;
}
