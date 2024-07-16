// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Engine/EngineTypes.h"
#include "Components/StaticMeshComponent.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "SimulatedMovementInterpolator.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include <GameplayEffectTypes.h>
#include <GASAttributes_FlyingPawn.h>
#include <GASAttributes_Main.h>
#include <ASC_Custom.h>

#include "FlyingPawnBase.generated.h"

UCLASS()
class MYPROJECT_API AFlyingPawnBase : public APawn, public IAbilitySystemInterface
{
	GENERATED_BODY()

private:
	uint32 accum;
	float elapsed_time;
	float average_elapsed_time;
	float min_time;
	float max_time;
	float cur_time;

	virtual void PossessedBy(AController* pawn_controller) override;
	
	virtual void OnRep_PlayerState() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif


//INTERNAL MOVEMENT SPECIFIC VARIABLES
	//Used as accumulator to make rolling smooth out when slowing down
	float lingering_roll_velocity;

	//Used as accumulator to make yaw pick up speed if holding 1 direction long enough
	float lingering_yaw_velocity;

	//Accumulated rotator inputs every frame. Should be reset at the end of every tick. 
	FRotator incoming_input_rotation;


	//!!VARIABLES DEPENDENT ON ASC ATTRIBUTES
	//THRUST
	float pawn_upward_thrust;
	float pawn_downward_thrust;
	float pawn_forward_thrust;
	float pawn_brake_power;
	float pawn_boost_thrust; //Steady afterburner power that fires independently of initial boost (no cooldown). 


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

//STAPLE COMPONENTS
	//Root of the pawn. Where most things will be attached to 
		/*
		* Edit anywhere = Allow property editting of defaults (in editor not by blueprints) and after already been constructed (i think)
		* BlueprintReadWrite = Accessible via blueprints
		*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMeshComponent* MainBody;

	//Primary Booster
		/*
		* Use to simulate the booster's physics behaviour - can lay the actual booster's mesh ontop of this component
		* But important to acknowledge how the shape may affect the physics behaviour. (tbh not sure if it does).
		*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly) //may need to replicate this object so that the server can know it's position accurately when it's doing network stuff
		UPrimitiveComponent* SimulatedBooster;


	//Booster Attachment Point
		/*
		* Physics constraint that attaches SimulatedBooster to MainBody. Adjust this constraint to adjust SimulatedBooster's behaviour
		*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UPhysicsConstraintComponent* BoosterHingeAttachement;


	//Ability System Component
		/*
		* Contains FlyingPawn attributes and encapsulates Gameplay Ability System functionality on this actor
		*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UASC_Custom* ASC;

	//List of attribute set classes that will always be included on this actor's ability system component. (Only for use in C++, can add additional attribute sets within DefaultStartingData array in editor). 
	TArray<TSubclassOf<UAttributeSet>> essential_FlyingPawn_attribute_sets = { UGASAttributes_FlyingPawn::StaticClass() };

	//Default override necessary for ASC tech
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return ASC; }

	//Initialize any startup attributes on the ASC (runs on both server and client)
	UFUNCTION(Category = "AbilitySystem")
	void InitASCAttributes();
	//Uses attributes from UGASAttributes_FlyingPawn to initialize pawn thrust for movement of this actor 
	void InitInternalPawnThrustsUsingAttributeSet();
	//Uses attributes from UGASAttributes_FlyingPawn to initialzie drag asymptote and drag soft caps for this actor.
	void InitInternalDragValuesUsingAttributeSet();

	//Initialize any startup attributes on the ASC (runs on both server and client). Automatically called inside InitASCAttributes function (C++) 
	UFUNCTION(BlueprintNativeEvent, Category = "AbilitySystem")
	void InitASCAttributesBlueprint();

	//Initialize any startup abilities on the ASC (runs only on server)
	UFUNCTION(Category = "AbilitySystem")
	void InitASCAbilities();


	/* Function assigned to be called by a delegate that fires whenever any thrust oriented attribute is changed
	*  Updates internal pawn thrusts and any other attributes that internally rely on the pawn thrusts changing.
	*/
	void OnThrustAttributeChangeDelegate(const FOnAttributeChangeData& Data);

	/*Function assigned to be called by a delegate that fires whenever a DragOffset attribute is changed.*/
	void OnDragOffsetAttributeChangeDelegate(const FOnAttributeChangeData& Data);


	//NETWORKING AND SERVER REPLICATION COMPONENT
	USimulatedMovementInterpolator* SIM_movement_handler;


//MAKING MOVEMENTS
	//!!Translation
	/*
	* Inputs contributes to the current "MovementInput" Vector
	*/

	/*
	* Function should be called every tick
	*
	* Given any thrust inputs made by the user between now and the last tick, processes them and apply them to the pawn
	*
	* Returns localized force vector that was applied to the pawn (localized meaning it is with respect to local pawn axes not world axes)
	*/
	FVector ResolveThrustInputsThisTick(float delta_time);

	//Function similar to GetLastMovementInputVector but returns the vector unscaled by any acceleration factors that may be applied on input directions. 
	UFUNCTION(BlueprintCallable, Category = "Movement|Inputs", BlueprintPure)
	FVector GetLastMovementInputVectorUNSCALED();

	UFUNCTION(BlueprintCallable, Category = "Movement|Inputs")
	void ApplyForwardThrust();

	UFUNCTION(BlueprintCallable, Category = "Movement|Inputs")
	void ApplyForwardBrake();

	UFUNCTION(BlueprintCallable, Category = "Movement|Inputs")
	void ApplyVertThrustUP();

	UFUNCTION(BlueprintCallable, Category = "Movement|Inputs")
	void ApplyVertThrustDOWN();


	//!!Rotation
	/*
	* Function should be called every tick
	*
	* Given any rotational inputs made by the user between now and the last tick, processes them and apply them to the pawn
	*
	* Returns the raw unscaled rotation that was applied on the pawn (raw = independent of delta_time)
	*/
	FRotator ResolveRotationInputsThisTick(float delta_time);


	//PITCH//
	
	//Use this parameter to specify how much pitch should be input every frame (in degrees)
	UPROPERTY(BlueprintReadWrite, Category = "Movement|Inputs")
	float pitch_input;

	UPROPERTY(BlueprintReadWrite, Category = "Movement|Inputs")
	bool bDoMaxPitchUp;

	UPROPERTY(BlueprintReadWrite, Category = "Movement|Inputs")
	bool bDoMaxPitchDown;

	/*
	* Gets pitch amount to apply on tick
	* Should be called within tick function.
	* To assign pitch or roll, look at setting pitch_input and roll_input attributes
	*/
	float HandlePitch(float delta_time);


	//YAW//

	float yaw_input; //is set by ApplyYaw function. Is checked every tick to see how it will affect yaw rotation.

	//Yaw can only be applied as a flat amount of degrees/time currently. 
	UFUNCTION(BlueprintCallable, Category = "Movement|Inputs")
	void ApplyYaw(bool bIsNegative);

	float HandleYaw(float delta_time);


	//ROLL//
	
	//Use this parameter to specify how much roll should be input every frame (in degrees)
	UPROPERTY(BlueprintReadWrite, Category = "Movement|Inputs")
	float roll_input;

	//value from -1 to 1 denoting the direction of roll (uses max roll speed to apply)
	int8 digital_roll_input;
	/*
	* Gets roll amount to apply on tick
	* Should be called within tick function.
	* To assign pitch or roll, look at setting pitch_input and roll_input attributes
	*/
	double HandleRoll(float delta_time);

	//Call this node if wanting to digitally handle roll inputs. Will always send the maximum amount of roll degrees to the pawn. 
	UFUNCTION(BlueprintCallable, Category = "Movement|Inputs")
	void ApplyMaxRoll(bool roll_right);


	//SPECIAL MOVEMENT//

	//apply boost
	UFUNCTION(BlueprintCallable, Category = "Movement|Afterburner")
	void ApplyBoost();

	//apply impulse boost. Only applies when the internal cooldown is <= 0.
	UFUNCTION(BlueprintCallable, Category = "Movement|Afterburner")
	bool ImpulseBoost(float boost_strength_scaling);



	//Experimental//
	/*
	* Experimental:
	*	Whether to treat thrust as Force to be applied against the Pawn as a physics object with mass, or to treat thrust as Adding velocity/mass over time. Both theoretically result in the same behavior. So mostly for debugging.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Advanced")
	bool bthrust_as_velocity;



	//AFTER MOVEMENT INPUTS, POST PROCESS MOVEMNENT EVERY FRAME 
		//Translation

		/*
		* Function should be called every tick
		*
		* Given pawns current movement (specified as parameters), apply some drag to the pawn by directly adding velocity
		*
		* Returns the combined change in velocity that was applied to the pawn as a world vector (not localized to the pawn)
		*/
	FVector ResolveDragDecelThisTick(float delta_time, FVector input_forces_this_tick, FVector localized_current_velocity);

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
	FVector GetYZDrag(FVector lingering_yz_velocity, float delta_time);

		/* Point at which the pawn will start to be affected by drag scaling with respect to movement along the relative YZ plane.
		The smaller this number, the earlier linear drag scaling will take affect. Keep in mind that drag will reduce the actual acceleration of the pawn.
		Variable directly used by its respective Drag calculation function */
		float YZaxis_vel_soft_cap;
		const double YZaxis_drag_func_curvature = 5 * 1e6; //Variable directly used in the above Drag calculation function
		const float YZaxis_drag_func_asymptote_base = 9000;
		const float YZaxis_drag_func_magnitude_shift = 1700; //Variable directly used in the above Drag calculation function
		/* Influences the YZaxis drag function
		* Essentially defines the velocity hard cap (asymptote) in its given direction
		* Variable directly used in the above Drag calculation function
		*/
		float YZaxis_drag_asymptote; //point of maximum drag (velocity hard cap) 
		void SetYZDragAsymptote(float asymptote_scaling_factor);

	/*Specific Axis Drag function. Currently ticked every frame and applied against velocity in that tick*/
	FVector GetXDrag(float lingering_x_velocity, float delta_time);

		/* Point at which the pawn will start to be affected by drag scaling with respect to movement in the relative forward direction.
		The smaller this number, the earlier linear drag scaling will take affect. Keep in mind that drag will reduce the actual acceleration of the pawn.
		Variable directly used by its respective Drag calculation function*/
		float forward_vel_soft_cap;
		const double forward_drag_func_curvature = 15 * 1e6; //Variable directly used in the above Drag calculation function
		const float forward_drag_func_asymptote_base = 22000;
		const float foward_drag_func_magnitude_shift = 1300; //Variable directly used in the above Drag calculation function
		/* Influences the Forward drag function
		* Essentially defines the velocity hard cap (asymptote) in its given direction
		Variable directly used in the above Drag calculation function*/
		float forward_drag_asymptote; //point of maximum drag (velocity hard cap)
		void SetForwardDragAsymptote(float asymptote_scaling_factor);

	
	/*
	* Will update YZ and Forward velocity soft caps. Should be called whenever attributes that those two properties rely on to be calculated change.
	* Can optionally pass in OnAttributeChangeData if being called from a delegate
	* Can call function with initialize = to true incase bingobnog
	*/
	void UpdatePawnVelSoftCaps(const FOnAttributeChangeData& Data, bool initialize);


	/*
	* Checks if pawn is moving at speeds close to zero in all axis and will set the speed to actually zero if true
	* Useful when velocity is small and a sort of bouncing between positive and negative vales occur due to incoming drag velocity overcorrecting.
	*/
	void ZeroVelocityCorrection(FVector lingering_velocity, float zero_speed_threshold);

	//Rotation

	//Other
	/*
	* Determine and apply subtle rotational influence on SimulatedBooster component
	* Based on assumed flight mode of the pawn (Hovering or Jet Flight) during the given tick
	*
	* May be one day useful to make this function return or set an ENUM which represents the flight mode types, so that way we can access the flight mode and do additional logic outside this class or in BP.
	*/
	void ResolveBoosterModeThisTick(FVector current_local_velocity);


	// Overriden to utilize custom network smoothing upon position update (simulated proxies only)
	void PostNetReceiveLocationAndRotation() override;



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
	UPROPERTY(EditAnywhere, Category = "Movement|Sensitivity|Deprecated", meta=(UIMin = "0", UIMax= "1.0"))
	float yaw_sensitivity;



	//How quickly the pawn reaches max yaw speed (MOVED TO UGASATTRIBUTES_FLYINGPAWN ATTRIBUTE SET)
	UPROPERTY(EditAnywhere, Category = "Movement|Deprecated")
	float yaw_acceleration;

	//Max amount of degrees per second the pawn can yaw left or right (MOVED TO UGASATTRIBUTES_FLYINGPAWN ATTRIBUTE SET)
	UPROPERTY(EditAnywhere, Category = "Movement|Deprecated")
	float max_yaw_speed;

	//Max amount of degrees the pawn can pitch per second (MOVED TO UGASATTRIBUTES_FLYINGPAWN ATTRIBUTE SET)
	UPROPERTY(EditAnywhere, Category = "Movement|Deprecated")
	float max_pitch_speed;

	//Max amount of degrees the pawn can roll per second (MOVED TO UGASATTRIBUTES_FLYINGPAWN ATTRIBUTE SET)
	UPROPERTY(EditAnywhere, Category = "Movement|Deprecated")
	float max_roll_speed; 







	/* A flat value that dampens yaw lingering momentum when no yaw input is given. The larger this value the snappier the yaw movement will feel when slowing down (not when speeding up - see yaw acceleration) */
	UPROPERTY(EditAnywhere, Category="Movement|Advanced")
	double yaw_momentum_dropoff;


	/* The smaller this parameter the quicker the roll momentum drops off, creating a snappier roll feeling. In otherwords simulating how strongly drag slows down the velocity of a roll. */
	UPROPERTY(EditAnywhere, Category = "Movement|Advanced")
	double roll_momentum_dropoff;


	//Cooldown between when initial boosts can take affect (TODO: HANDLED BY GAMEPLAY ABILITY SYSTEM)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Afterburner")
	float initial_boost_cooldown;

	//Remaining time before next initial_boost impulse can be fired again  (TODO: HANDLED BY GAMEPLAY ABILITY SYSTEM)
	UPROPERTY(BlueprintReadOnly, Category = "Movement|Afterburner")
	float initial_boost_cd_counter;





	//Booster Pointing Direction Stuff stuff
	/*
	* Use speed ratio threshold of forward to vertical velocity to determine flight mode
	*/
	UPROPERTY(EditAnywhere, Category="Movement|BoosterMode")
	bool b_use_ratio_for_boostermode;

	/*
	* Determines speed ratio threshold of forward to vertical velocity that must be achieved to change flight modes
	*/
	UPROPERTY(EditAnywhere, Category = "Movement|BoosterMode", meta = (EditCondition = "b_use_ratio_for_boostermode"))
	float jet_hover_ratio_threshold;

	/*
	* if b_determine_flight_mode_from_ratio = false
	* use flat velocity to determine flight mode instead of a ratio
	*/
	UPROPERTY(EditAnywhere, Category = "Movement|BoosterMode", meta = (EditCondition = "!b_use_ratio_for_boostermode"))
	float jet_speed_threshold;

	//Amount of artificial turning force to apply to booster based on assigned BoosterMode
	UPROPERTY(EditAnywhere, Category = "Movement|BoosterMode")
	float constraint_velocity_target;



public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;



	//Rudimentary replay system so I can test multiplayer against myself 
	UFUNCTION(BlueprintCallable)
	void replay_last_movements();

	bool bDo_replay = false; 
	int frame_counter = 0;
	TArray<FVector> saved_positions_each_frame;
	TArray<FRotator> saved_rotations_each_frame;



	/* 
	* Experimental aiming system 
	* variable contributes to both yaw and roll at the same time.
	* (yaw speed & sensiticity are constrained to max pitch speed & sensitivty)
	* (roll speed & sensitivity are constrained to normal roll speed & sensitivity) 
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool bUse_yaw_roll_input;

	/*
	* Experimental aiming system
	* variable contributes to both yaw and roll at the same time.
	* (yaw speed & sensiticity are constrained to max pitch speed & sensitivty)
	* (roll speed & sensitivity are constrained to normal roll speed & sensitivity)
	*/
	UPROPERTY(BlueprintReadWrite, Category = "Movement")
	float yaw_roll_input;

	//Special override to make roll influence stronger
	UPROPERTY(EditAnywhere, Category = "Movement", Meta = (EditCondition = "bUse_yaw_roll_input"))
	float special_roll_speed;

	//Special override to make yaw influence stronger
	UPROPERTY(EditAnywhere, Category = "Movement", Meta = (EditCondition = "bUse_yaw_roll_input"))
	float special_yaw_speed;

	/*
	* When value is 0, only yaw is applied by yaw_roll_input (will behave like pitch but sideways)
	* When value is 1, only roll is applied by yaw_roll_input (will behave like normal roll)
	*/
	UPROPERTY(EditAnywhere, Category = "Movement",  Meta=(ClampMin="0", ClampMax="1", EditCondition = "bUse_yaw_roll_input"))
	float special_yaw_roll_ratio;
};
