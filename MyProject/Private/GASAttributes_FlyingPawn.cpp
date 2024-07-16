// Fill out your copyright notice in the Description page of Project Settings.

#include "GASAttributes_FlyingPawn.h"
#include "Net/UnrealNetwork.h"


UGASAttributes_FlyingPawn::UGASAttributes_FlyingPawn()
{
}

void UGASAttributes_FlyingPawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    //some of these might be only necessary to set initial only such as the sub attributes Thrustpower_up_ratio, _down, ....

    DOREPLIFETIME_CONDITION_NOTIFY(UGASAttributes_FlyingPawn, Thrustpower, COND_AutonomousOnly, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UGASAttributes_FlyingPawn, Thrustpower_up_ratio, COND_AutonomousOnly, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UGASAttributes_FlyingPawn, Thrustpower_down_ratio, COND_AutonomousOnly, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UGASAttributes_FlyingPawn, Thrustpower_forward_ratio, COND_AutonomousOnly, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UGASAttributes_FlyingPawn, Thrustpower_brake_ratio, COND_AutonomousOnly, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UGASAttributes_FlyingPawn, Thrustpower_boost_ratio, COND_AutonomousOnly, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UGASAttributes_FlyingPawn, Impulsepower_boost, COND_AutonomousOnly, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UGASAttributes_FlyingPawn, Accelfactor, COND_AutonomousOnly, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UGASAttributes_FlyingPawn, Accelfactor_up, COND_AutonomousOnly, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UGASAttributes_FlyingPawn, Accelfactor_down, COND_AutonomousOnly, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UGASAttributes_FlyingPawn, Accelfactor_forward, COND_AutonomousOnly, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UGASAttributes_FlyingPawn, Accelfactor_backward, COND_AutonomousOnly, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UGASAttributes_FlyingPawn, Pitchspeed, COND_AutonomousOnly, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UGASAttributes_FlyingPawn, Yawspeed, COND_AutonomousOnly, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UGASAttributes_FlyingPawn, Yawspeed_accel, COND_AutonomousOnly, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UGASAttributes_FlyingPawn, Rollspeed, COND_AutonomousOnly, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UGASAttributes_FlyingPawn, Dragpower, COND_AutonomousOnly, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UGASAttributes_FlyingPawn, Dragpower_yz, COND_AutonomousOnly, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UGASAttributes_FlyingPawn, Dragpower_forward, COND_AutonomousOnly, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UGASAttributes_FlyingPawn, Dragpower_backward, COND_AutonomousOnly, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UGASAttributes_FlyingPawn, Dragpower_offset_yz, COND_AutonomousOnly, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UGASAttributes_FlyingPawn, Dragpower_offset_forward, COND_AutonomousOnly, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UGASAttributes_FlyingPawn, Dragpower_backward_minimum, COND_AutonomousOnly, REPNOTIFY_Always);
}

void UGASAttributes_FlyingPawn::OnRep_Thrustpower(const FGameplayAttributeData& OldThrustpower)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UGASAttributes_FlyingPawn, Thrustpower, OldThrustpower);
}

void UGASAttributes_FlyingPawn::OnRep_Thrustpower_up_ratio(const FGameplayAttributeData& OldThrustpower_up_ratio)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UGASAttributes_FlyingPawn, Thrustpower_up_ratio, OldThrustpower_up_ratio);
}

void UGASAttributes_FlyingPawn::OnRep_Thrustpower_down_ratio(const FGameplayAttributeData& OldThrustpower_down_ratio)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UGASAttributes_FlyingPawn, Thrustpower_down_ratio, OldThrustpower_down_ratio);
}

void UGASAttributes_FlyingPawn::OnRep_Thrustpower_forward_ratio(const FGameplayAttributeData& OldThrustpower_forward_ratio)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UGASAttributes_FlyingPawn, Thrustpower_forward_ratio, OldThrustpower_forward_ratio);
}

void UGASAttributes_FlyingPawn::OnRep_Thrustpower_brake_ratio(const FGameplayAttributeData& OldThrustpower_brake_ratio)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UGASAttributes_FlyingPawn, Thrustpower_brake_ratio, OldThrustpower_brake_ratio);
}

void UGASAttributes_FlyingPawn::OnRep_Thrustpower_boost_ratio(const FGameplayAttributeData& OldThrustpower_boost_ratio)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UGASAttributes_FlyingPawn, Thrustpower_boost_ratio, OldThrustpower_boost_ratio);
}

void UGASAttributes_FlyingPawn::OnRep_Impulsepower_boost(const FGameplayAttributeData& OldImpulsepower_boost)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UGASAttributes_FlyingPawn, Impulsepower_boost, OldImpulsepower_boost);
}

void UGASAttributes_FlyingPawn::OnRep_Accelfactor(const FGameplayAttributeData& OldAccelfactor)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UGASAttributes_FlyingPawn, Accelfactor, OldAccelfactor);
}

void UGASAttributes_FlyingPawn::OnRep_Accelfactor_up(const FGameplayAttributeData& OldAccelfactor_up)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UGASAttributes_FlyingPawn, Accelfactor_up, OldAccelfactor_up);
}

void UGASAttributes_FlyingPawn::OnRep_Accelfactor_down(const FGameplayAttributeData& OldAccelfactor_down)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UGASAttributes_FlyingPawn, Accelfactor_down, OldAccelfactor_down);
}

void UGASAttributes_FlyingPawn::OnRep_Accelfactor_forward(const FGameplayAttributeData& OldAccelfactor_forward)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UGASAttributes_FlyingPawn, Accelfactor_forward, OldAccelfactor_forward);
}

void UGASAttributes_FlyingPawn::OnRep_Accelfactor_backward(const FGameplayAttributeData& OldAccelfactor_backward)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UGASAttributes_FlyingPawn, Accelfactor_backward, OldAccelfactor_backward);
}

void UGASAttributes_FlyingPawn::OnRep_Pitchspeed(const FGameplayAttributeData& OldPitchspeed)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UGASAttributes_FlyingPawn, Pitchspeed, OldPitchspeed);
}

void UGASAttributes_FlyingPawn::OnRep_Yawspeed(const FGameplayAttributeData& OldYawspeed)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UGASAttributes_FlyingPawn, Yawspeed, OldYawspeed);
}

void UGASAttributes_FlyingPawn::OnRep_Yawspeed_accel(const FGameplayAttributeData& OldYawspeed_accel)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UGASAttributes_FlyingPawn, Yawspeed_accel, OldYawspeed_accel);
}

void UGASAttributes_FlyingPawn::OnRep_Rollspeed(const FGameplayAttributeData& OldRollspeed)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UGASAttributes_FlyingPawn, Rollspeed, OldRollspeed);
}

void UGASAttributes_FlyingPawn::OnRep_Dragpower(const FGameplayAttributeData& OldDragpower)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UGASAttributes_FlyingPawn, Dragpower, OldDragpower);
}

void UGASAttributes_FlyingPawn::OnRep_Dragpower_yz(const FGameplayAttributeData& OldDragpower_yz)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UGASAttributes_FlyingPawn, Dragpower_yz, OldDragpower_yz);
}

void UGASAttributes_FlyingPawn::OnRep_Dragpower_forward(const FGameplayAttributeData& OldDragpower_forward)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UGASAttributes_FlyingPawn, Dragpower_forward, OldDragpower_forward);
}

void UGASAttributes_FlyingPawn::OnRep_Dragpower_backward(const FGameplayAttributeData& OldDragpower_backward)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UGASAttributes_FlyingPawn, Dragpower_backward, OldDragpower_backward);
}

void UGASAttributes_FlyingPawn::OnRep_Dragpower_offset_yz(const FGameplayAttributeData& OldDragpower_offset_yz)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UGASAttributes_FlyingPawn, Dragpower_offset_yz, OldDragpower_offset_yz);
}

void UGASAttributes_FlyingPawn::OnRep_Dragpower_offset_forward(const FGameplayAttributeData& OldDragpower_offset_forward)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UGASAttributes_FlyingPawn, Dragpower_offset_forward, OldDragpower_offset_forward);
}

void UGASAttributes_FlyingPawn::OnRep_Dragpower_backward_minimum(const FGameplayAttributeData& OldDragpower_backward_minimum)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UGASAttributes_FlyingPawn, Dragpower_backward_minimum, OldDragpower_backward_minimum);
}