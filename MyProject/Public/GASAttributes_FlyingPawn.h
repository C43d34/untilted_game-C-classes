// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"

#include "GASAttributes_FlyingPawn.generated.h"



#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)
/**
 * 
 */
UCLASS()
class MYPROJECT_API UGASAttributes_FlyingPawn : public UAttributeSet
{
	GENERATED_BODY()
	
public:
	UGASAttributes_FlyingPawn(); //default constructor

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

//Define generic attributes used by all FlyingPawn type actors 
//Note: assuming that gameplay effects that modify these stats are replicated to autonomous client, we might forgoe directly replicating these values and just letting local clients compute them manually. 
	//(Be careful about simulated clients running abilities with visual or positional effects that rely on these stats to be accurate)


	// THRUST
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|FlyingPawn", ReplicatedUsing = OnRep_Thrustpower)
	FGameplayAttributeData Thrustpower;
	ATTRIBUTE_ACCESSORS(UGASAttributes_FlyingPawn, Thrustpower)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|FlyingPawn", ReplicatedUsing = OnRep_Thrustpower_up_ratio)
	FGameplayAttributeData Thrustpower_up_ratio;
	ATTRIBUTE_ACCESSORS(UGASAttributes_FlyingPawn, Thrustpower_up_ratio)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|FlyingPawn", ReplicatedUsing = OnRep_Thrustpower_down_ratio)
	FGameplayAttributeData Thrustpower_down_ratio;
	ATTRIBUTE_ACCESSORS(UGASAttributes_FlyingPawn, Thrustpower_down_ratio)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|FlyingPawn", ReplicatedUsing = OnRep_Thrustpower_forward_ratio)
	FGameplayAttributeData Thrustpower_forward_ratio;
	ATTRIBUTE_ACCESSORS(UGASAttributes_FlyingPawn, Thrustpower_forward_ratio)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|FlyingPawn", ReplicatedUsing = OnRep_Thrustpower_brake_ratio)
	FGameplayAttributeData Thrustpower_brake_ratio;
	ATTRIBUTE_ACCESSORS(UGASAttributes_FlyingPawn, Thrustpower_brake_ratio)


	// SPECIAL THRUST (BOOSTING)
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|FlyingPawn", ReplicatedUsing = OnRep_Thrustpower_boost_ratio)
	FGameplayAttributeData Thrustpower_boost_ratio;
	ATTRIBUTE_ACCESSORS(UGASAttributes_FlyingPawn, Thrustpower_boost_ratio)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|FlyingPawn", ReplicatedUsing = OnRep_Impulsepower_boost)
	FGameplayAttributeData Impulsepower_boost;
	ATTRIBUTE_ACCESSORS(UGASAttributes_FlyingPawn, Impulsepower_boost)


	// ACCELERATION FACTOR
	/*
	* Modifier to increase acceleration amount without affecting velocity soft-cap thresholds of the pawn.
	*
	* Multiplied to thrust amount and drag amount to balance between the two forces
	* Makes the pawn acceleration (and deceleration) feel more responsive.
	*/
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|FlyingPawn", ReplicatedUsing = OnRep_Accelfactor)
	FGameplayAttributeData Accelfactor;
	ATTRIBUTE_ACCESSORS(UGASAttributes_FlyingPawn, Accelfactor)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|FlyingPawn", ReplicatedUsing = OnRep_Accelfactor_up)
	FGameplayAttributeData Accelfactor_up;
	ATTRIBUTE_ACCESSORS(UGASAttributes_FlyingPawn, Accelfactor_up)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|FlyingPawn", ReplicatedUsing = OnRep_Accelfactor_down)
	FGameplayAttributeData Accelfactor_down;
	ATTRIBUTE_ACCESSORS(UGASAttributes_FlyingPawn, Accelfactor_down)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|FlyingPawn", ReplicatedUsing = OnRep_Accelfactor_forward)
	FGameplayAttributeData Accelfactor_forward;
	ATTRIBUTE_ACCESSORS(UGASAttributes_FlyingPawn, Accelfactor_forward)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|FlyingPawn", ReplicatedUsing = OnRep_Accelfactor_backward)
	FGameplayAttributeData Accelfactor_backward;
	ATTRIBUTE_ACCESSORS(UGASAttributes_FlyingPawn, Accelfactor_backward)


	// ROTATION
	//Max amount of degrees the pawn can pitch per second
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|FlyingPawn", ReplicatedUsing = OnRep_Pitchspeed)
	FGameplayAttributeData Pitchspeed;
	ATTRIBUTE_ACCESSORS(UGASAttributes_FlyingPawn, Pitchspeed)

	//Max amount of degrees per second the pawn can yaw left or right
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|FlyingPawn", ReplicatedUsing = OnRep_Yawspeed)
	FGameplayAttributeData Yawspeed;
	ATTRIBUTE_ACCESSORS(UGASAttributes_FlyingPawn, Yawspeed)

	//How quickly the pawn reaches max yaw speed
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|FlyingPawn", ReplicatedUsing = OnRep_Yawspeed_accel)
	FGameplayAttributeData Yawspeed_accel;
	ATTRIBUTE_ACCESSORS(UGASAttributes_FlyingPawn, Yawspeed_accel)

	//Max amount of degrees the pawn can roll per second
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|FlyingPawn", ReplicatedUsing = OnRep_Rollspeed)
	FGameplayAttributeData Rollspeed;
	ATTRIBUTE_ACCESSORS(UGASAttributes_FlyingPawn, Rollspeed)


	// DRAG
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|FlyingPawn", ReplicatedUsing = OnRep_Dragpower)
	FGameplayAttributeData Dragpower;
	ATTRIBUTE_ACCESSORS(UGASAttributes_FlyingPawn, Dragpower)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|FlyingPawn", ReplicatedUsing = OnRep_Dragpower_yz)
	FGameplayAttributeData Dragpower_yz;
	ATTRIBUTE_ACCESSORS(UGASAttributes_FlyingPawn, Dragpower_yz)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|FlyingPawn", ReplicatedUsing = OnRep_Dragpower_forward)
	FGameplayAttributeData Dragpower_forward;
	ATTRIBUTE_ACCESSORS(UGASAttributes_FlyingPawn, Dragpower_forward)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|FlyingPawn", ReplicatedUsing = OnRep_Dragpower_backward)
	FGameplayAttributeData Dragpower_backward;
	ATTRIBUTE_ACCESSORS(UGASAttributes_FlyingPawn, Dragpower_backward)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|FlyingPawn", ReplicatedUsing = OnRep_Dragpower_offset_yz)
	FGameplayAttributeData Dragpower_offset_yz;
	ATTRIBUTE_ACCESSORS(UGASAttributes_FlyingPawn, Dragpower_offset_yz)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|FlyingPawn", ReplicatedUsing = OnRep_Dragpower_offset_forward)
	FGameplayAttributeData Dragpower_offset_forward;
	ATTRIBUTE_ACCESSORS(UGASAttributes_FlyingPawn, Dragpower_offset_forward)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|FlyingPawn", ReplicatedUsing = OnRep_Dragpower_backward_minimum)
	FGameplayAttributeData Dragpower_backward_minimum;
	ATTRIBUTE_ACCESSORS(UGASAttributes_FlyingPawn, Dragpower_backward_minimum)


	// OnRep functions
	UFUNCTION()
	virtual void OnRep_Thrustpower(const FGameplayAttributeData& OldThrustpower);

	UFUNCTION()
	virtual void OnRep_Thrustpower_up_ratio(const FGameplayAttributeData& OldThrustpower_up_ratio);

	UFUNCTION()
	virtual void OnRep_Thrustpower_down_ratio(const FGameplayAttributeData& OldThrustpower_down_ratio);

	UFUNCTION()
	virtual void OnRep_Thrustpower_forward_ratio(const FGameplayAttributeData& OldThrustpower_forward_ratio);

	UFUNCTION()
	virtual void OnRep_Thrustpower_brake_ratio(const FGameplayAttributeData& OldThrustpower_brake_ratio);

	UFUNCTION()
	virtual void OnRep_Thrustpower_boost_ratio(const FGameplayAttributeData& OldThrustpower_boost_ratio);

	UFUNCTION()
	virtual void OnRep_Impulsepower_boost(const FGameplayAttributeData& OldImpulsepower_boost);

	UFUNCTION()
	virtual void OnRep_Accelfactor(const FGameplayAttributeData& OldAccelfactor);

	UFUNCTION()
	virtual void OnRep_Accelfactor_up(const FGameplayAttributeData& OldAccelfactor_up);

	UFUNCTION()
	virtual void OnRep_Accelfactor_down(const FGameplayAttributeData& OldAccelfactor_down);

	UFUNCTION()
	virtual void OnRep_Accelfactor_forward(const FGameplayAttributeData& OldAccelfactor_forward);

	UFUNCTION()
	virtual void OnRep_Accelfactor_backward(const FGameplayAttributeData& OldAccelfactor_backward);

	UFUNCTION()
	virtual void OnRep_Pitchspeed(const FGameplayAttributeData& OldPitchspeed);

	UFUNCTION()
	virtual void OnRep_Yawspeed(const FGameplayAttributeData& OldYawspeed);

	UFUNCTION()
	virtual void OnRep_Yawspeed_accel(const FGameplayAttributeData& OldYawspeed_accel);

	UFUNCTION()
	virtual void OnRep_Rollspeed(const FGameplayAttributeData& OldRollspeed);

	UFUNCTION()
	virtual void OnRep_Dragpower(const FGameplayAttributeData& OldDragpower);

	UFUNCTION()
	virtual void OnRep_Dragpower_yz(const FGameplayAttributeData& OldDragpower_yz);

	UFUNCTION()
	virtual void OnRep_Dragpower_forward(const FGameplayAttributeData& OldDragpower_forward);

	UFUNCTION()
	virtual void OnRep_Dragpower_backward(const FGameplayAttributeData& OldDragpower_backward);

	UFUNCTION()
	virtual void OnRep_Dragpower_offset_yz(const FGameplayAttributeData& OldDragpower_offset_yz);

	UFUNCTION()
	virtual void OnRep_Dragpower_offset_forward(const FGameplayAttributeData& OldDragpower_offset_forward);

	UFUNCTION()
	virtual void OnRep_Dragpower_backward_minimum(const FGameplayAttributeData& OldDragpower_backward_minimum);


};
