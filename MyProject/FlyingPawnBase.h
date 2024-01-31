// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Engine/EngineTypes.h"
#include "Components/StaticMeshComponent.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"


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

	float lingering_roll_velocity;

	FRotator incoming_input_rotation;


public:
	// Sets default values for this pawn's properties
	AFlyingPawnBase();

	
	//Sensitivity and input strength Settings

	UPROPERTY(EditAnywhere, Category = "Movement|Sensitivity")
	float pitch_sensitivity;

	UPROPERTY(EditAnywhere, Category = "Movement|Sensitivity")
	float roll_sensitivity;


	/*
	* Clamped to between 0 and 1
	*/
	UPROPERTY(EditAnywhere, Category = "Movement|Sensitivity")
	float yaw_sensitivity;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float base_upward_thrust;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float base_downward_thrust;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float base_forward_thrust;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float base_brake_power;


	//Pawn acceleration stuff
	/*
	* Scales how quickly the pawn accelerates (mostly by just multiplying by delta time)
	*/

	/*
	* Experimental modifier to increase acceleration amount without affecting soft-cap thresholds of the pawn.
	* 
	* Multiplies the thrust amount and drag amount (at soft cap thresholds) every frame. 
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


	UPROPERTY(EditAnywhere, Category = "Movement")
	float base_yaw_amnt;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float max_pitch_speed;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float max_roll_speed; 


	//Simulating Pawn Drag Stuff
	/* How quickly the pawn decelerates from drag and partially affects how quickly it accelerates to higher speeds
	
	Consider adjusting Drag if we really need to increase deceleration, but be wary that it affects acceleration and maximum speeds as well - so thrust may need to change to compensate. */
	UPROPERTY(EditAnywhere, Category = "Movement|Deprecated")
	float YZaxis_drag_intensity;

	/* Point at which the pawn will start to be affected by linear scaling drag
	
	Consider adjusting Drag if we really need to increase deceleration, but be wary that it affects acceleration and maximum speeds as well - so thrust may need to change to compensate. */
	UPROPERTY(EditAnywhere, Category = "Movement|Advanced")
	float YZaxis_vel_soft_cap;

	/*
	* experimental
	* soft cap offset when pawn is in decoupled flight
	*/
	UPROPERTY(EditAnywhere, Category = "Movement|Advanced")
	float YZAxis_decoupled_offset;

	/* Factor of how large the minimum speed is for speed in the YZ axis. As this parameter increases, the minimum speed allowed before fully stopping will be larger
	
	Consider adjusting Drag if we really need to increase deceleration, but be wary that it affects acceleration and maximum speeds as well - so thrust may need to change to compensate. */
	UPROPERTY(EditAnywhere, Category = "Movement|Advanced")
	float YZaxis_drag_min_soft_cap;


	/* How quickly the pawn decelerates from drag and partially affects how quickly it accelerates to higher speeds
	Less deceleration as this parameter increases
	
	Consider adjusting Drag if we really need to increase deceleration, but be wary that it affects acceleration and maximum speeds as well - so thrust may need to change to compensate. */
	UPROPERTY(EditAnywhere, Category = "Movement|Advanced")
	float forward_drag_intensity;

	/* Point at which the pawn will start to be affected by linear scaling drag
	
	Consider adjusting Drag if we really need to increase deceleration, but be wary that it affects acceleration and maximum speeds as well - so thrust may need to change to compensate. */
	UPROPERTY(EditAnywhere, Category = "Movement|Advanced")
	float forward_vel_soft_cap;

	/*
	* experimental
	* soft cap offset when pawn is in decoupled flight 
	*/
	UPROPERTY(EditAnywhere, Category = "Movement|Advanced")
	float forward_decoupled_offset;

	/* Strength of the softcap gets stronger as this parameter increases. Reducing maximum speed.
	Use for microadjustments to the curve of the drag.
	
	Consider adjusting Drag if we really need to increase deceleration, but be wary that it affects acceleration and maximum speeds as well - so thrust may need to change to compensate. */
	UPROPERTY(EditAnywhere, Category = "Movement|Advanced")
	float forward_soft_cap_strength;


	/* How quickly the pawn decelerates from drag and partially affects how quickly it accelerates to higher speeds
	
	Consider adjusting Drag if we really need to increase deceleration, but be wary that it affects acceleration and maximum speeds as well - so thrust may need to change to compensate. */
	UPROPERTY(EditAnywhere, Category = "Movement|Advanced")
	float backward_drag_intensity;

	/* this parameter * backward_drag_intensity = Point at which the pawn will start to be affected constant factor drag effect
	This drag is applied from velocity levels between this point and 0. Since this is backward velocity though it will specifically be in the -x => -0 
	
	
	Consider adjusting Drag if we really need to increase deceleration, but be wary that it affects acceleration and maximum speeds as well - so thrust may need to change to compensate. */
	UPROPERTY(EditAnywhere, Category = "Movement|Advanced")
	float backward_drag_min_soft_cap;

	/* Strength of the soft cap gets stronger as this parameter approaches zero. Reducing maximum speed*/
	UPROPERTY(EditAnywhere, Category = "Movement|Deprecated")
	float backward_soft_cap_strength;


	/* The smaller this parameter the quicker the roll momentum drops off */
	UPROPERTY(EditAnywhere, Category = "Movement|Advanced")
	double roll_momentum_dropoff;


	UPROPERTY(EditAnywhere, Category = "Movement|Deprecated")
	float vertical_momentum_dropoff;

	UPROPERTY(EditAnywhere, Category = "Movement|Deprecated")
	float vertical_momentum_dropoff_threshold;

	UPROPERTY(EditAnywhere, Category = "Movement|Deprecated")
	float vertical_momentum_dropoff_min_strength;

	UPROPERTY(EditAnywhere, Category = "Movement|Deprecated")
	float horizontal_momentum_dropoff;

	UPROPERTY(EditAnywhere, Category = "Movement|Deprecated")
	float horizontal_momentum_dropoff_threshold;

	UPROPERTY(EditAnywhere, Category = "Movement|Deprecated")
	float horizontal_momentum_dropoff_min_strength;

	UPROPERTY(EditAnywhere, Category = "Movement|Deprecated")
	float sideways_momentum_dropoff;

	UPROPERTY(EditAnywhere, Category = "Movement|Deprecated")
	float sideways_momentum_dropoff_threshold;

	UPROPERTY(EditAnywhere, Category = "Movement|Deprecated")
	float sideways_momentum_dropoff_min_strength;



	//Afterburner stuff
	UPROPERTY(EditAnywhere, Category= "Movement|Afterburner")
	float initial_boost_strength;

	UPROPERTY(EditAnywhere, Category = "Movement|Afterburner")
	float initial_boost_cooldown;

	UPROPERTY(BlueprintReadOnly, Category = "Movement|Afterburner")
	float initial_boost_cd_counter;

	UPROPERTY(EditAnywhere, Category = "Movement|Afterburner")
	float boost_strength;

	/*
	* Increases max speed threshold when boosting
	*/
	UPROPERTY(EditAnywhere, Category = "Movement|Deprecated")
	float max_speed_inc;


	//Flightmode stuff
	/*
	* Determines speed ratio threshold of forward and vertical velocity that must be achieved to change flight modes 
	*/
	UPROPERTY(EditAnywhere, Category="Movement|Flight Mode")
	bool b_use_ratio_for_boostermode;

	UPROPERTY(EditAnywhere, Category = "Movement|Flight Mode")
	float jet_hover_ratio_threshold;

	/*
	* if b_determine_flight_mode_from_ratio = false
	* use flat velocity to determine flight mode instead of a ratio
	*/
	UPROPERTY(EditAnywhere, Category = "Movement|Flight Mode")
	float jet_speed_threshold;

	UPROPERTY(EditAnywhere, Category = "Movement|Flight Mode")
	float constraint_velocity_target;

protected:

	UPROPERTY(BlueprintReadWrite, Category = "Movement|Inputs")
	float pitch_input;

	UPROPERTY(BlueprintReadWrite, Category = "Movement|Inputs")
	float roll_input;

	int8 digital_roll_input; //value from -1 to 1 denoting the direction of roll (uses max roll speed to apply)

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
	* 
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Advanced")
	bool bcoupled_flight_enabled;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Advanced")
	bool bthrust_as_velocity;


	//Rotation
	/*
	* Since there is no "AddMovementInput" function for rotation, will have to do with manual rotation
	* aside from yaw, these should be called within tick function.
	* To assign pitch or roll, look at setting pitch_input and roll_input attributes
	*/

	/*
	* Gets pitch amount to apply on tick
	* Should be called within tick function.
	* To assign pitch or roll, look at setting pitch_input and roll_input attributes
	*/
	float HandlePitch(float delta_time);

	//apply yaw
	UFUNCTION(BlueprintCallable, Category = "Movement|Inputs")
	void ApplyYaw(bool bIsNegative);

	//roll
	/*
	* Gets roll amount to apply on tick
	* Should be called within tick function.
	* To assign pitch or roll, look at setting pitch_input and roll_input attributes
	*/
	double HandleRoll(float delta_time);

	UFUNCTION(BlueprintCallable, Category = "Movement|Inputs")
	void ApplyMaxRoll(bool roll_right);

	//Special Movement
	
	//apply boost
	UFUNCTION(BlueprintCallable, Category = "Movement|Afterburner")
	void ApplyBoost();

	UFUNCTION(BlueprintCallable, Category = "Movement|Afterburner")
	bool InitialBoost();


//HANDLE MOVEMENT EVERY FRAME
	//Deprecated or not in use or just trash code 
	float VelocityFromEXPOFastDrag(float desired_vel_increase, float lingering_vel, float momentum_dropoff_strength, float delta_time, float accel_scaling);
	/*
	* Uses logarithmic function c/logx to calculate change in velocity for next frame
	*/
	float VelocityFromLOGSlowDrag(float desired_vel_increase, float lingering_vel, float momentum_dropoff_strength, float delta_time, float accel_scaling);
	float VelocityFromSoftCapDrag(float desired_vel_increase, float lingering_vel, float soft_cap_threshold, float delta_time);
	float GetXVelAfterDrag(float desired_x_velocity, float lingering_x_velocity, float delta_time);
	float GetXDrag(float desired_x_velocity, float lingering_x_velocity, float delta_time);
	float GetYVelAfterDrag(float desired_y_velocity, float lingering_y_velocity, float delta_time);
	float GetZVelAfterDrag(float desired_z_velocity, float lingering_z_velocity, float delta_time);
	//

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


	/*Specific Axis Drag function.Currently ticked every frame and applied against velocity in that tick*/
	FVector GetXDrag(float lingering_x_velocity, float delta_time);

	/*Specific Axis Drag function. Currently ticked every frame and applied against velocity in that tick*/
	FVector GetYZDrag(FVector lingering_yz_velocity, float delta_time);
	
	
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
	//Translation & Rotation
	/*
	* use to pass pawn's next world position and rotation to server after having accounted for all local inputs
	*/
	UFUNCTION(Server, Unreliable)
	void PassTransformToServer(FTransform final_world_transform);

	UFUNCTION(Server, Unreliable)
	void PassVelToServer(FVector final_world_velocity);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Overriden to utilize custom network smoothing upon position update (simulated proxies only)
	void PostNetReceiveLocationAndRotation() override;
};
