// Fill out your copyright notice in the Description page of Project Settings.


#include "SimulatedMovementInterpolator.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"



// Sets default values for this component's properties
USimulatedMovementInterpolator::USimulatedMovementInterpolator()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	owners_physics_body_name = TEXT("Root");
	owners_physics_body = nullptr;
	//SetIsReplicated(true); //this crashes my game when it's enabled by default, thank you leaguye of legends.

	bSIM_decay_input_velocity = false;
	SIM_last_velocity_input = FVector(0);
	SIM_last_position_goal = FVector(0);

	SIM_rotational_goal_interpolator_alpha = 0;
	SIM_last_rotational_velocity = FRotator(0);
	SIM_last_rotational_goal = FRotator(0);
}



// Called when the game starts
void USimulatedMovementInterpolator::BeginPlay()
{
	Super::BeginPlay();

	// ...
	this->actor_to_simulate_for = this->GetOwner();

	//owner hasn't already set a reference to the physics body so we need to look for it now. 
	if (this->owners_physics_body == nullptr)
	{
		this->owners_physics_body = Cast<UPrimitiveComponent>(this->actor_to_simulate_for->GetDefaultSubobjectByName(this->owners_physics_body_name));
	}

	//debugging
	//if (this->actor_to_simulate_for->IsValidLowLevel())
	//{
	//	GEngine->AddOnScreenDebugMessage(1235, 100, FColor::Red, FString::Printf(TEXT("father %s"), *this->actor_to_simulate_for->GetName()));
	//	if (this->owners_physics_body->IsValidLowLevel())
	//	{
	//		GEngine->AddOnScreenDebugMessage(1234, 100, FColor::Red, FString::Printf(TEXT("assigned physics body to be interpolated when simulated proxy blahg blah %s"), *this->owners_physics_body->GetName()));
	//	}
	//	else
	//	{
	//		GEngine->AddOnScreenDebugMessage(1234, 100, FColor::Red, FString::Printf(TEXT("babdabdabdabdabdaa %s"), *this->owners_physics_body_name.ToString()));
	//	}
	//}
	//else
	//{
	//	GEngine->AddOnScreenDebugMessage(1234, 100, FColor::Red, FString::Printf(TEXT("owner not real")));
	//}
}



// Called every frame
void USimulatedMovementInterpolator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}



void USimulatedMovementInterpolator::PassRotationToServer_Implementation(FRotator rotational_velocity, FRotator newest_rotational_goal)
{
	this->SIM_last_rotational_velocity = rotational_velocity;

	this->SIM_last_rotational_goal = newest_rotational_goal;
	OnRep_SIM_last_rotational_goal();
}


//Server handling of reported location from autonomous proxys actors 
void USimulatedMovementInterpolator::PassTranslationToServer_Implementation(FVector world_velocity, FVector reported_world_location)
{
	this->SIM_last_position_goal = reported_world_location;
	OnRep_SIM_last_position_goal();

	this->SIM_last_velocity_input = world_velocity;
	OnRep_SIM_last_velocity_input();
}



void USimulatedMovementInterpolator::HandleSimulatingPosition(float delta_time)
{
	this->owners_physics_body->SetPhysicsLinearVelocity(this->SIM_last_velocity_input);

	//try to ease off velocity a little bit so we dont end up overshooting.
	if (this->bSIM_decay_input_velocity)
	{	//this is not frame independent btw (issue with long frametimes like on server which is 30tick rate)
		this->SIM_last_velocity_input = this->SIM_last_velocity_input *
			(this->SIM_last_velocity_input.Normalize() - (this->SIM_last_velocity_input.Normalize() * delta_time));
	}

	//find out how far we are from desired position and append extra velocity to compensate 
	this->owners_physics_body->SetPhysicsLinearVelocity((this->SIM_last_position_goal - this->actor_to_simulate_for->GetActorLocation()), true);

	/*debug*/
	//GEngine->AddOnScreenDebugMessage(411, 1, FColor::Emerald, FString::Printf(TEXT("411 Latest Goal Position %s"), *this->SIM_last_position_goal.ToString()));
	//GEngine->AddOnScreenDebugMessage(412, 1, FColor::Emerald, FString::Printf(TEXT("412 Goal Mag %.2f, Goal Error %s: "), (this->SIM_last_position_goal.Length() - this->actor_to_simulate_for->GetActorLocation().Length()), *(this->SIM_last_position_goal - this->actor_to_simulate_for->GetActorLocation()).ToString()));
	/*debug*/
}



void USimulatedMovementInterpolator::HandleSimualtingRotation(float delta_time)
{
	//Simulate Rotation
	FQuat SIM_interpolated_rotation_quat;

	// if reported rotational velocity is pretty much zero, then only interpolate to last reported goal
	if (this->SIM_last_rotational_velocity.Equals(FRotator(0), 0.0001f))
	{

		//do nothing if the current rotation is close to rotator goal and rotational velocity is also close to zero
		if (this->actor_to_simulate_for->GetActorRotation().Equals(this->SIM_last_rotational_goal, 0.01f)) //could make this more efficient by expressing rotational velocity as a scalar value because we are only using it for it's magnitude in this case 
		{
		}

		//we are not at our goal and accumulator is interpolating us past the goal, so we need to go back
		else if (this->SIM_rotational_goal_interpolator_alpha > 1.0)
		{
			this->SIM_rotational_goal_interpolator_alpha = FMath::Clamp(this->SIM_rotational_goal_interpolator_alpha - delta_time, 1, INT32_MAX); //rewind interpolation until we reach our goal (1)

			SIM_interpolated_rotation_quat = FMath::Lerp(this->actor_to_simulate_for->GetActorRotation().Quaternion(), this->SIM_last_rotational_goal.Quaternion(), this->SIM_rotational_goal_interpolator_alpha);
			//SIM_interpolated_rotation_quat = FQuat::Slerp(this->actor_to_simulate_for->GetActorRotation().Quaternion(), this->SIM_last_rotational_goal.Quaternion(), this->SIM_rotational_goal_interpolator_alpha);
			this->actor_to_simulate_for->SetActorRotation(SIM_interpolated_rotation_quat);
		}

		//interpolate towards our goal 
		else
		{
			this->SIM_rotational_goal_interpolator_alpha = FMath::Clamp(this->SIM_rotational_goal_interpolator_alpha + delta_time, 0, 1); //step forward interpolation up to our goal

			SIM_interpolated_rotation_quat = FMath::Lerp(this->actor_to_simulate_for->GetActorRotation().Quaternion(), this->SIM_last_rotational_goal.Quaternion(), this->SIM_rotational_goal_interpolator_alpha);
			//SIM_interpolated_rotation_quat = FQuat::Slerp(this->actor_to_simulate_for->GetActorRotation().Quaternion(), this->SIM_last_rotational_goal.Quaternion(), this->SIM_rotational_goal_interpolator_alpha);
			this->actor_to_simulate_for->SetActorRotation(SIM_interpolated_rotation_quat);
		}
	}

	//reported rotational velocity is non-zero, we are free to interpolate (predict future) rotational position beyond last recieved goal 
	else
	{
		this->SIM_rotational_goal_interpolator_alpha += delta_time;


		SIM_interpolated_rotation_quat = FQuat::Slerp(this->actor_to_simulate_for->GetActorRotation().Quaternion(), this->SIM_last_rotational_goal.Quaternion(), this->SIM_rotational_goal_interpolator_alpha);
		//SIM_interpolated_rotation_quat = FMath::Lerp(this->actor_to_simulate_for->GetActorRotation().Quaternion(), this->SIM_last_rotational_goal.Quaternion(), this->SIM_rotational_goal_interpolator_alpha);
		this->actor_to_simulate_for->SetActorRotation(SIM_interpolated_rotation_quat);
	}
}



void USimulatedMovementInterpolator::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//Variables to share only with simulated actors
	//DOREPLIFETIME_CONDITION(USimulatedMovementInterpolator, SIM_last_rotational_velocity, COND_SimulatedOnly);
	//DOREPLIFETIME_CONDITION(USimulatedMovementInterpolator, SIM_last_rotational_goal, COND_SimulatedOnly);

	//DOREPLIFETIME_CONDITION(USimulatedMovementInterpolator, SIM_last_position_goal, COND_SimulatedOnly);
	//DOREPLIFETIME_CONDITION(USimulatedMovementInterpolator, SIM_last_velocity_input, COND_SimulatedOnly);

	DOREPLIFETIME_CONDITION(USimulatedMovementInterpolator, SIM_last_rotational_velocity, COND_SkipOwner); 
	DOREPLIFETIME_CONDITION(USimulatedMovementInterpolator, SIM_last_rotational_goal, COND_SkipOwner);

	DOREPLIFETIME_CONDITION(USimulatedMovementInterpolator, SIM_last_position_goal, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(USimulatedMovementInterpolator, SIM_last_velocity_input, COND_SkipOwner);

	DOREPLIFETIME_CONDITION(USimulatedMovementInterpolator, actor_to_simulate_for, COND_InitialOnly);
	DOREPLIFETIME_CONDITION(USimulatedMovementInterpolator, owners_physics_body, COND_InitialOnly);

}



void USimulatedMovementInterpolator::OnRep_SIM_last_rotational_goal()
{
	this->SIM_rotational_goal_interpolator_alpha = 0.0f; //reset interpolation to zero now that we have recieved a new goal/
	this->SIM_last_rotational_goal.Normalize();
}



void USimulatedMovementInterpolator::OnRep_SIM_last_position_goal()
{
	//find difference between actual position (the one on this simulated client) and position goal (the one reported by the server), add this vector to last simulated velocity input to start approaching the corrected position 
	//FVector simulated_location = this->actor_to_simulate_for->GetActorLocation();
	//FVector distance_away_from_goal = this->SIM_last_position_goal - simulated_location;
	//this->SIM_last_velocity_input += distance_away_from_goal;

}



void USimulatedMovementInterpolator::OnRep_SIM_last_rotational_velocity()
{
}

void USimulatedMovementInterpolator::OnRep_SIM_last_velocity_input()
{
	//new input velocity recieved as per this OnRep call, so we will refrain from decaying input velocity until some arbitrary time has passed
	this->bSIM_decay_input_velocity = false;
	//this->SIM_begin_decay_velocity_handle.Invalidate(); //technically we should invalidate the previous timer but it kind of is working fine without. we will just decay a little bit in between onReps I guess. 
	GetWorld()->GetTimerManager().SetTimer(this->SIM_begin_decay_velocity_handle, this, &USimulatedMovementInterpolator::SIMEnableDecayVelocity, 0.05f, false); //set a non-looping timer to set decay velocity flag to true
}



void USimulatedMovementInterpolator::SIMEnableDecayVelocity() {
	this->bSIM_decay_input_velocity = true;
}

