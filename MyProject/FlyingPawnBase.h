// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Engine/EngineTypes.h"
#include "Components/StaticMeshComponent.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "SimulatedMovementInterpolator.h"

#include "FlyingPawnBase.generated.h"

UCLASS()
class MYPROJECT_API AFlyingPawnBase : public APawn
{
	GENERATED_BODY()

private:
	uint32 accum;
	float elapsed_time;
	float average_elapsed_time;
	float min_time;
	float max_time;
	float cur_time;

	//Used as accumulator to make rolling smooth out when slowing down
	float lingering_roll_velocity;

	//Accumulated rotator inputs every frame. Should be reset at the end of every tick. 
	FRotator incoming_input_rotation;
public:
	// Sets default values for this pawn's properties
	AFlyingPawnBase();

	
	//Sensitivity and input strength Settings
	UPROPERTY(EditAnywhere, Category = "Movement|Sensitivity")
	float pitch_sensitivity;

	//Sensitivity and input strength Settings
	UPROPERTY(EditAnywhere, Category = "Movement|Sensitivity")
	float roll_sensitivity;

	//Sensitivity and input strength Settings
	UPROPERTY(EditAnywhere, Category = "Movement|Sensitivity", meta=(UIMin = "0", UIMax= "1.0"))
	float yaw_sensitivity;


	UPROPERTY(EditAnywhere, Category = "Movement")
	float base_upward_thrust;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float base_downward_thrust;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float base_forward_thrust;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float base_brake_power;

	/*
	* Experimental modifier to increase acceleration amount without affecting soft-cap thresholds of the pawn.
	* 
	* Multiplies the thrust amount and drag amount (at soft cap thresholds) every frame. 
	* Makes the pawn acceleration (and deceleration) feel more responsive.
	*/
	UPROPERTY(EditAnywhere, Category = "Movement|Advanced")
	float global_acceleration_scaling;

	UPROPERTY(EditAnywhere, Category = "Movement|Deprecated")
	float upward_accel_scale;

	UPROPERTY(EditAnywhere, Category = "Movement|Deprecated")
	float forward_accel_scale;

	UPROPERTY(EditAnywhere, Category = "Movement|Deprecated")
	float backward_accel_scale;

	UPROPERTY(EditAnywhere, Category = "Movement|Deprecated")
	float downward_accel_scale;

	UPROPERTY(EditAnywhere, Category = "Movement|Deprecated")
	float sideways_accel_scale;

	//Amount of degrees per second the pawn will yaw left or right
	UPROPERTY(EditAnywhere, Category = "Movement")
	float base_yaw_amnt;

	//Max amount of degrees the pawn can pitch per second
	UPROPERTY(EditAnywhere, Category = "Movement")
	float max_pitch_speed;

	//Max amount of degrees the pawn can roll per second
	UPROPERTY(EditAnywhere, Category = "Movement")
	float max_roll_speed; 


	/*
	* experimental
	* soft cap offset when pawn is in decoupled flight
	*/
	UPROPERTY(EditAnywhere, Category = "Movement|Advanced")
	float YZAxis_decoupled_offset;

	/* DRAG: Point at which the pawn will start to be affected by drag with linear with respect to movement along the relative YZ plane.
	
	The smaller this number, the earlier linear drag scaling will take affect. Keep in mind that drag will reduce the actual acceleration of the pawn. 
	If looking to adjust maximum speed - try to adjust thrust (or mass) instead. */
	UPROPERTY(EditAnywhere, Category = "Movement|Advanced")
	float YZaxis_vel_soft_cap;


	/* Factor of how large the minimum speed is for speed in the YZ axis. As this parameter increases, the minimum speed allowed before fully stopping will be larger.
	Prevents the pawn from gliding around indefinitely at low speeds.*/
	UPROPERTY(EditAnywhere, Category = "Movement|Advanced")
	float YZaxis_drag_min_soft_cap;




	/* DRAG: Indirectly affects pawn maximum speed and acceleration in the forward direction. Also plays a role in decelerating the pawn.
	* Drag is increased as this parameter gets *smaller*.
	
	Consider adjusting thrust (or mass) in the forward direction - if only concerned about maximum speed and acceleration.*/
	UPROPERTY(EditAnywhere, Category = "Movement|Advanced")
	float forward_drag_intensity;

	/*
	* experimental
	* soft cap offset when pawn is in decoupled flight
	*/
	UPROPERTY(EditAnywhere, Category = "Movement|Advanced")
	float forward_decoupled_offset;

	/* DRAG: Point at which the pawn will start to be affected by drag with linear with respect to movement in the relative forward direction.
	
	The smaller this number, the earlier linear drag scaling will take affect. Keep in mind that drag will reduce the actual acceleration of the pawn. So the sooner this soft cap is reached, the quicker that effect will take place.*/
	UPROPERTY(EditAnywhere, Category = "Movement|Advanced")
	float forward_vel_soft_cap;

	/* Curve of the drag softcap gets steeper as this parameter increases. Reducing maximum speed.
	**very sensitive setting** */
	UPROPERTY(EditAnywhere, Category = "Movement|Advanced")
	float forward_soft_cap_strength;




	/* DRAG: Affects the strength of how quickly the pawn decelerates while moving in the backward direction. 
	* Drag is increased as this parameter gets *smaller*.

	Assuming there is no way to add thrust in the backward direction. This parameter is the main way to control the upper limit of backwards velocity. .*/
	UPROPERTY(EditAnywhere, Category = "Movement|Advanced")
	float backward_drag_intensity;

	/* A flat value of deceleration to apply as drag to the pawn when moving backwards at slower speeds. 
	Unlike the parameter backward_drag_intensity, this is unaffected by momentary velocity. Therefore this value becomes relevant whenever the resulting drag calculated using momentary velocity is smaller than the specified value. */
	UPROPERTY(EditAnywhere, Category = "Movement|Advanced")
	float backward_drag_min_soft_cap;



	/* The smaller this parameter the quicker the roll momentum drops off, creating a snappier roll feeling. In otherwords simulating how strongly drag slows down the velocity of a roll. */
	UPROPERTY(EditAnywhere, Category = "Movement|Advanced")
	double roll_momentum_dropoff;



	//Strength of afterburner when initially fired (impulse) when not on cooldown.
	UPROPERTY(EditAnywhere, Category= "Movement|Afterburner")
	float initial_boost_strength;

	//Cooldown between when initial boosts can take affect
	UPROPERTY(EditAnywhere, Category = "Movement|Afterburner")
	float initial_boost_cooldown;

	//Remaining time before next initial_boost impulse can be fired again
	UPROPERTY(BlueprintReadOnly, Category = "Movement|Afterburner")
	float initial_boost_cd_counter;

	//Steady afterburner power that fires independently of initial boost (no cooldown). 
	UPROPERTY(EditAnywhere, Category = "Movement|Afterburner")
	float boost_strength;



	//Booster Pointing Direction Stuff stuff
	/*
	* Use speed ratio threshold of forward to vertical velocity to determine flight mode
	*/
	UPROPERTY(EditAnywhere, Category="Movement|BoosterMode")
	bool b_use_ratio_for_boostermode;

	/*
	* Determines speed ratio threshold of forward to vertical velocity that must be achieved to change flight modes
	*/
	UPROPERTY(EditAnywhere, Category = "Movement|BoosterMode")
	float jet_hover_ratio_threshold;

	/*
	* if b_determine_flight_mode_from_ratio = false
	* use flat velocity to determine flight mode instead of a ratio
	*/
	UPROPERTY(EditAnywhere, Category = "Movement|BoosterMode")
	float jet_speed_threshold;

	//Amount of artificial turning force to apply to booster based on assigned BoosterMode
	UPROPERTY(EditAnywhere, Category = "Movement|BoosterMode")
	float constraint_velocity_target;



protected:
	//Use this parameter to specify how much pitch should be input every frame (in degrees)
	UPROPERTY(BlueprintReadWrite, Category = "Movement|Inputs")
	float pitch_input;

	//Use this parameter to specify how much roll should be input every frame (in degrees)
	UPROPERTY(BlueprintReadWrite, Category = "Movement|Inputs")
	float roll_input;

	//value from -1 to 1 denoting the direction of roll (uses max roll speed to apply)
	int8 digital_roll_input; 

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

//STAPLE COMPONENTS
	//Root of the pawn. Where most things will be attached to 
		/*
		* Edit anywhere = Allow property editting of defaults (in editor not by blueprints) and after already been constructed (i think)
		* BlueprintReadWrite = Accessible via blueprints
		*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMeshComponent *MainBody;

	//Primary Booster
		/*
		* Use to simulate the booster's physics behaviour - can lay the actual booster's mesh ontop of this component
		* But important to acknowledge how the shape may affect the physics behaviour. (tbh not sure if it does).
		*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly) //may need to replicate this object so that the server can know it's position accurately when it's doing network stuff
	UPrimitiveComponent *SimulatedBooster;


	//Booster Attachment Point
		/*
		* Physics constraint that attaches SimulatedBooster to MainBody. Adjust this constraint to adjust SimulatedBooster's behaviour 
		*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UPhysicsConstraintComponent *BoosterHingeAttachement;



//MAKE MOVEMENTS
	//Translation
	/*
	* Inputs contributes to the current "MovementInput" Vector 
	*/

	UFUNCTION(BlueprintCallable, Category = "Movement|Inputs")
	void ApplyForwardThrust();

	UFUNCTION(BlueprintCallable, Category = "Movement|Inputs")
	void ApplyForwardBrake();

	UFUNCTION(BlueprintCallable, Category = "Movement|Inputs")
	void ApplyVertThrustUP();

	UFUNCTION(BlueprintCallable, Category = "Movement|Inputs")
	void ApplyVertThrustDOWN();

	/*
	* Experimental:
	*	Significantly weaken all thruster controls but allow pawn to retain momentum much easier
	*  not implemented
	*/
	//UFUNCTION(BlueprintCallable, Category = "Movement|Advanced")
	void SimDecoupledFlight(FVector localized_velocity_linger, float DeltaTime);

	/*
	* not implemented
	*/
	//UFUNCTION(BlueprintCallable, Category = "Movement|Advanced")
	void SimCoupledFlight(FVector localized_velocity_linger, float DeltaTime);

	/*
	* Experimental:
	*	A flag denoting unique pawn handling. When disabled, the pawn will be considered in decoupled flight, which has faster acceleration but worse handling. 
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Advanced")
	bool bcoupled_flight_enabled;

	/*
	* Experimental:
	*	Whether to treat thrust as Force to be applied against the Pawn as a physics object with mass, or to treat thrust as Adding velocity/mass over time. Both theoretically result in the same behavior. So mostly for debugging.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Advanced")
	bool bthrust_as_velocity;


	//Rotation
	/*
	* Gets pitch amount to apply on tick
	* Should be called within tick function.
	* To assign pitch or roll, look at setting pitch_input and roll_input attributes
	*/
	float HandlePitch(float delta_time);

	//Yaw can only be applied as a flat amount of degrees/time currently. 
	UFUNCTION(BlueprintCallable, Category = "Movement|Inputs")
	void ApplyYaw(bool bIsNegative);

	//roll
	/*
	* Gets roll amount to apply on tick
	* Should be called within tick function.
	* To assign pitch or roll, look at setting pitch_input and roll_input attributes
	*/
	double HandleRoll(float delta_time);

	//Call this node if wanting to digitally handle roll inputs. Will always send the maximum amount of roll degrees to the pawn. 
	UFUNCTION(BlueprintCallable, Category = "Movement|Inputs")
	void ApplyMaxRoll(bool roll_right);

	//Special Movement
	
	//apply boost
	UFUNCTION(BlueprintCallable, Category = "Movement|Afterburner")
	void ApplyBoost();

	//apply initial boost as impulse. Only applies when the internal cooldown is <= 0.
	UFUNCTION(BlueprintCallable, Category = "Movement|Afterburner")
	bool InitialBoost();



//AFTER MOVEMENT INPUTS, POST PROCESS MOVEMNENT EVERY FRAME 
	//Translation
	//Generic Drag functions in charge of getting the amount of drag to apply counter to the lingering_velocity 
	/*
	* linear functions use ratio (x/c) onto lingering_velocity to find out how much drag to apply. 
	* Decent to use when trying to find a maximum speed cap for the pawn. As velocity (x) increases, the amount of drag increases linearly. Steady state occurs when drag == incoming acceleration (or velocity added each frame). 
	*/
	FVector2D DragFromLINEAR(FVector2D lingering_velocity, float drag_strength, float delta_time, float drag_scaling);
	float DragFromLINEAR(float lingering_velocity, float drag_strength, float delta_time, float drag_scaling);

	/*
	* logarithmic functions use ratio (c/logx) onto lingering_velocity to find out how much drag to apply.
	* Naturally these functions apply more drag as the velocity gets smaller. So they aren't good for capping out the pawn's maximum speeds. 
	*/
	FVector DragFromLOG(FVector lingering_velocity, float delta_time, float drag_scaling);
	float DragFromLOG(float lingering_velocity, float delta_time, float drag_scaling);


	/*Specific Axis Drag function. Currently ticked every frame and applied against velocity in that tick*/
	FVector GetXDrag(float lingering_x_velocity, float delta_time);

	/*Specific Axis Drag function. Currently ticked every frame and applied against velocity in that tick*/
	FVector GetYZDrag(FVector lingering_yz_velocity, float delta_time);
	
	/*
	* Checks if pawn is moving at speeds close to zero in all axis and will set the speed to actually zero if true
	* Useful when velocity is small and a sort of bouncing between positive and negative vales occur due to incoming drag velocity overcorrecting. 
	*/
	void ZeroVelocityCorrection(FVector lingering_velocity, float zero_speed_threshold);
	
	//Rotation

	//Other
	/*
	* Determine and apply subtle rotational influence on SimulatedBooster component
	* Based on assumed flight mode of the pawn (Hovering or Jet Flight)
	* 
	* May be one day useful to make this function return or set an ENUM which represents the flight mode types, so that way we can access the flight mode and do additional logic outside this class or in BP.
	*/
	void ResolveBoosterMode(FVector current_local_velocity);



//NETWORKING AND SERVER REPLICATION
	USimulatedMovementInterpolator *SIM_movement_handler;

	//Translation & Rotation
	/*
	* use to pass pawn's next world position and rotation to server after having accounted for all local inputs
	*/
	UFUNCTION(Server, Unreliable)
	void PassRotationToServer(FRotator rotational_velocity, FRotator newest_rotational_goal);

	/*
	* use to pass pawn's velocity to the server as owner of the pawn
	*/
	UFUNCTION(Server, Unreliable)
	void PassTranslationToServer(FVector world_velocity, FVector reported_world_location);

	// Overriden to utilize custom network smoothing upon position update (simulated proxies only)
	void PostNetReceiveLocationAndRotation() override;

	/*
	* Should only be executed on simulated clients. Should be called every tick
	* Handles the job of moving the pawn on other players screens 
	*/
	void HandleMovementOnSimulatedClient(float delta_time);


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
	





public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


};
