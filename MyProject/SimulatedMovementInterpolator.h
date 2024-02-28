// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/EngineTypes.h"
#include "GameFramework/FloatingPawnMovement.h"


#include "SimulatedMovementInterpolator.generated.h"

/*
* Interpolates movement for pawns controlled by simulated proxies.
* Requires a reference to the owning actor's movement component before construction (must be simulating physics on the simulated proxy side in order for interpolation to take effect. 
* If simulate physics is not enabled (atleast on simulated proxy version of actor), nothing will happen. 
* 
* Rotational changes are expressed on the root of the actor
*/
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MYPROJECT_API USimulatedMovementInterpolator : public UActorComponent
{
	GENERATED_BODY()

public:	
	//name of component that will be used to push physics on the simulated version of this actor
	UPROPERTY(EditAnywhere, Category="Movement|Simulated")
	FName owners_physics_body_name;

	//reference to component that is used to push physics on the simulated version of this actor
	UPROPERTY(Replicated, BlueprintReadOnly)
	UPrimitiveComponent* owners_physics_body;

	//default constructors
	USimulatedMovementInterpolator();

protected:
	//direct reference to the actor to interpolate movement with on simulated versions. 
	UPROPERTY(Replicated, BlueprintReadOnly)
	AActor* actor_to_simulate_for;

	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:

//STUFF FOR INTERPOLATING MOVEMENT ON SIMULATED CLIENTS
	
	//Translation & Rotation
	/*
	* use to pass pawn's next world position and rotation to server after having accounted for all local inputs
	*/
	UFUNCTION(Server, Unreliable, BlueprintCallable, Category = "Movement")
	void PassRotationToServer(FRotator rotational_velocity, FRotator newest_rotational_goal);

	/*
	* use to pass pawn's velocity to the server as owner of the pawn
	*/
	UFUNCTION(Server, Unreliable, BlueprintCallable, Category = "Movement")
	void PassTranslationToServer(FVector world_velocity, FVector reported_world_location);


	/*
	* Should only be executed on simulated clients. Should be called every tick
	* Handles the job of moving the pawn on other players screens
	* 
	* (can also be called on the server if wanting to handle replication to server that way, but if smoothness is not important, use a different method)
	*/
	UFUNCTION(BlueprintCallable, Category = "Movement|Simulated")
	void HandleSimulatingPosition(float delta_time);


	/*
	* Should only be executed on simulated clients. Should be called every tick
	* Handles the job of rotating the pawn on other players screens
	*
	* (can also be called on the server if wanting to handle replication to server that way, but if smoothness is not important, use a different method)
	*/
	UFUNCTION(BlueprintCallable, Category = "Movement|Simulated")
	void HandleSimualtingRotation(float delta_time);


	UPROPERTY(ReplicatedUsing = OnRep_SIM_last_rotational_goal, BlueprintReadOnly, Category = "Movement|Simulated")
	FRotator SIM_last_rotational_goal;
	float SIM_rotational_goal_interpolator_alpha; //Use this variable to keep track of how much time has passed since we started interpolating towards rotational goal. This value can exceed 1.0f which means >100% interpolation.

	//raw inputs from autonomous proxy passed over to simulated proxy. Before scaled by time or any post process effects
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Movement|Simulated")
	FRotator SIM_last_rotational_velocity;
	FTimerHandle SIM_begin_decay_rotator_handle; //when this timer has finished, bSIM_decay_input_rotator will be set to true (by calling EnableDecayRotation()), can invalidate early to make sure this doesn't happen. 


	UPROPERTY(ReplicatedUsing = OnRep_SIM_last_position_goal, BlueprintReadOnly, Category = "Movement|Simulated")
	FVector SIM_last_position_goal;

	UPROPERTY(ReplicatedUsing = OnRep_SIM_last_velocity_input, BlueprintReadOnly, Category = "Movement|Simulated")
	FVector SIM_last_velocity_input;
	bool bSIM_decay_input_velocity; //When true, SIM_last_velocity_input will decay slightly over time
	void SIMEnableDecayVelocity();
	FTimerHandle SIM_begin_decay_velocity_handle; //when this timer has finished, bSIM_decay_input_velocity will be set to true (by calling EnableDecayVelocity()), can invalidate early to make sure this doesnt happen.


	/*
	* OnRep functions must be manually called on server after the value is changed (only for C++) in order for replication to register.
	*/


	UFUNCTION()
	void OnRep_SIM_last_rotational_goal();


	UFUNCTION()
	void OnRep_SIM_last_position_goal();

	UFUNCTION()
	void OnRep_SIM_last_velocity_input();



};
