// Fill out your copyright notice in the Description page of Project Settings.


#include "FlyingPawnBase.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"


// Sets default values
AFlyingPawnBase::AFlyingPawnBase()
{
	this->bReplicates = true;
	//this->SetReplicateMovement(true);
	this->incoming_input_rotation = FRotator(0);

//TEST STUFF
	accum = 0;
	elapsed_time = 0;
	average_elapsed_time = 0;
	min_time = 0;
	max_time = 0;

//Internal Attributes
	lingering_roll_velocity = 0;

//External Attribute Defaults
	base_upward_thrust = 100,000; //equivalent to 2500 velocity every frame (before delta_time scaling)
	base_downward_thrust = 60,000;
	base_forward_thrust = 200,000;
	base_brake_power = 80,000;
	base_yaw_amnt = 25;	
	max_pitch_speed = 190;
	max_roll_speed = 250;

	pitch_sensitivity = 0.5;
	roll_sensitivity = 15;
	yaw_sensitivity = 1.0;

	global_acceleration_scaling = 1.5f;

	YZaxis_vel_soft_cap = 1500;
	YZaxis_drag_min_soft_cap = 1.0;

	forward_drag_intensity = 3.0;
	forward_vel_soft_cap = 3500;
	forward_soft_cap_strength = 1.07;

	backward_drag_intensity = 3.0; 
	backward_drag_min_soft_cap = 300;

	roll_momentum_dropoff = 0.00001;

	initial_boost_strength = 300,000;
	initial_boost_cooldown = 1;
	boost_strength = 200,000;

	b_use_ratio_for_boostermode = true;
	jet_hover_ratio_threshold = 2.5f;
	jet_speed_threshold = 0;
	constraint_velocity_target = 0.9f;

// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

// Create a dummy root component we can attach things to.

	this->MainBody = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MainBody"));
	RootComponent = this->MainBody;

	MainBody->SetSimulatePhysics(true);
	MainBody->bReplicatePhysicsToAutonomousProxy = false;
	MainBody->SetEnableGravity(false);
	MainBody->SetGenerateOverlapEvents(true);
	MainBody->SetCollisionObjectType(ECollisionChannel::ECC_Pawn); 

//Instantiate Hinge Booster Technology
	this->SimulatedBooster = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SimulatedBooster"));
	SimulatedBooster->SetupAttachment(MainBody);
	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		SimulatedBooster->SetSimulatePhysics(true); //this component exists purely for simulation so it should have no collision
		SimulatedBooster->SetEnableGravity(false); //...
		SimulatedBooster->SetCollisionObjectType(ECollisionChannel::ECC_PhysicsBody); //...
		SimulatedBooster->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics); //...
		SimulatedBooster->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore); //...
		SimulatedBooster->AddRelativeLocation(FVector(-103, 0, -12.5));
		SimulatedBooster->SetRelativeScale3D(FVector(1.0, 0.75, 0.25));
		SimulatedBooster->SetCenterOfMass(FVector(-10, 0, 0));
	}


	this->BoosterHingeAttachement = CreateDefaultSubobject<UPhysicsConstraintComponent>(TEXT("BoosterHingeAttachment"));
	BoosterHingeAttachement->SetupAttachment(MainBody);
	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		BoosterHingeAttachement->SetRelativeLocation(FVector(-42, 0, -10));
		BoosterHingeAttachement->SetDisableCollision(true);
		BoosterHingeAttachement->SetAngularSwing1Limit(EAngularConstraintMotion::ACM_Locked, 0); //only allowing single axis swing
		BoosterHingeAttachement->SetAngularSwing2Limit(EAngularConstraintMotion::ACM_Limited, 45); //... 45degree 
		BoosterHingeAttachement->SetAngularTwistLimit(EAngularConstraintMotion::ACM_Locked, 0); //...
		BoosterHingeAttachement->ConstraintInstance.AngularRotationOffset = FRotator(45, 0, 0);
		BoosterHingeAttachement->ConstraintInstance.EnableParentDominates();
		BoosterHingeAttachement->SetAngularDriveMode(EAngularDriveMode::TwistAndSwing);
		BoosterHingeAttachement->SetAngularVelocityDriveTwistAndSwing(false, true);
		BoosterHingeAttachement->SetAngularDriveParams(0, 10, 0);
	}


	////Define Child and Parent if constraint (the 1 and 2 order matters)
	this->BoosterHingeAttachement->SetConstrainedComponents(this->SimulatedBooster, this->SimulatedBooster->GetFName(), this->MainBody, this->MainBody->GetFName());
	BoosterHingeAttachement->ComponentName2 = FConstrainComponentPropName(MainBody->GetFName());


//Interpolation Component To Network Replication doesn't look ass
	this->SIM_movement_handler = CreateDefaultSubobject<USimulatedMovementInterpolator>(TEXT("SimulatedMovementInterpolator"));
	if (SIM_movement_handler)
	{
		this->SIM_movement_handler->owners_physics_body_name = this->MainBody->GetFName();
		this->SIM_movement_handler->owners_physics_body = Cast<UPrimitiveComponent>(this->MainBody);
		this->SIM_movement_handler->SetNetAddressable();
		this->SIM_movement_handler->SetIsReplicated(true);
	}

	//if (!HasAnyFlags(RF_ClassDefaultObject))
	//{

	//}
	//this->AddOwnedComponent(this->SIM_movement_handler);
}



// Called when the game starts or when spawned
void AFlyingPawnBase::BeginPlay()
{
	Super::BeginPlay();

//!!Setup necessary replication protocol based on role
	/*
	* Should not affect how client interprets owned pawn. However it will affect how client interprets remote pawns. 
	*/
	if (GetLocalRole() == ROLE_SimulatedProxy)
	{
		//Intuition is to make it so the server replicates out raw positional data
		//  rather than trying to simulate physics which creates really bad stuttering.
		//	But we still want the owned client's object to simulate physics so we do this
		//	only on the aforementioned roles. 
		//We'll then apply our own smoothing function that will trigger on tick later. 
		//MainBody->SetSimulatePhysics(false);
		//this->SetPhysicsReplicationMode(EPhysicsReplicationMode::PredictiveInterpolation);
		//GEngine->AddOnScreenDebugMessage(911, 10, FColor::Purple, FString::Printf(TEXT("911 faggotry, %s"), this-> ? TEXT("True") : TEXT("False")));

		//this->BoosterHingeAttachement->UpdateConstraintFrames();

	}
	//this->BoosterHingeAttachement->UpdateConstraintFrames();


}



// Called every frame
void AFlyingPawnBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//GEngine->AddOnScreenDebugMessage(0, 100.0, FColor::Black, FString::Printf(TEXT("%d"), 
		/*(GEngine->GetNetMode(GWorld) == NM_Client) ? TEXT("Client")
		: (GEngine->GetNetMode(GWorld) == NM_ListenServer) ? TEXT("ListenServer")
		: (GEngine->GetNetMode(GWorld) == NM_DedicatedServer) ? TEXT("DedicatedServer")
		: TEXT("Standalone"))*/
		//GEngine->GetNetMode(GWorld)
		//));

	GEngine->AddOnScreenDebugMessage(121111324, 100, FColor::Red, FString::Printf(TEXT("asdfhjk ufkc u")));
	
	//may want to check if GEngine and GWorld are not null pointers when doing this but anyway.
	//run standard code if standalone or an autonomous proxy in a server mode world
	if (Controller && (Controller->GetLocalRole() == ROLE_AutonomousProxy || GEngine->GetNetMode(GWorld) == NM_ListenServer || GEngine->GetNetMode(GWorld) == NM_Standalone))
	{
		cur_time = cur_time + DeltaTime;
		this->initial_boost_cd_counter = FMath::Clamp((initial_boost_cd_counter - DeltaTime), 0, initial_boost_cooldown);

		GEngine->AddOnScreenDebugMessage(344, 5, FColor::Purple, FString::Printf(TEXT("344 %s"), *FString::SanitizeFloat(DeltaTime)));
		GEngine->AddOnScreenDebugMessage(343, 5, FColor::Purple, FString::Printf(TEXT("343 %f"), cur_time));


	//!!Process incomming rotation input

		//Handle Yaw
		this->incoming_input_rotation.Yaw = this->incoming_input_rotation.Yaw * DeltaTime;

		//Apply angular roll momentum if slowing down or changing direction
		this->incoming_input_rotation.Roll = bcoupled_flight_enabled? HandleRoll(DeltaTime) : 0.5f * HandleRoll(DeltaTime);

		//Handle Pitch
		this->incoming_input_rotation.Pitch = bcoupled_flight_enabled? HandlePitch(DeltaTime) : 0.5f * HandlePitch(DeltaTime);
	
		//Add all rotators together to the current rotation (input rotation is usually already scaled by time) 

		this->AddActorLocalRotation(this->incoming_input_rotation);
	
		//velocity to pass over to simulated proxies (we don't actually use this variable inside autonmous proxy
		FRotator latest_rotational_velocity = this->incoming_input_rotation * (1 / DeltaTime); //unscale by delta time to get value as time independent velocity

		GEngine->AddOnScreenDebugMessage(2277, 1, FColor::Green, FString::Printf(TEXT("2277 world rotation % s"), *this->GetActorRotation().ToString()));

		//Flush Inputs
		this->incoming_input_rotation = FRotator(0); //is it possible to get inputs in the middle of this tick function? that would not really be that bad but kinda weird sideeffect 
		this->digital_roll_input = 0;


	//!!Process incoming movement input
		FRotator local_axes_rotator = MainBody->GetRelativeRotation();
		FVector new_input_velocity_influence = this->ConsumeMovementInputVector() * this->global_acceleration_scaling; //get acceleration to apply this frame * scaling factor
	
		/*EXPERIMENTAL increase thrust in all directions from flight mode*/
		if (!bcoupled_flight_enabled)
		{
			new_input_velocity_influence = new_input_velocity_influence * 1.5;
		}

		//Add input to the pawn
		/*EXPERIMENTAL (can't really tell a difference between velocity and thrust overall)*/
		if (bthrust_as_velocity)
		{
			new_input_velocity_influence = DeltaTime* (new_input_velocity_influence / MainBody->GetMass());
			MainBody->SetAllPhysicsLinearVelocity(local_axes_rotator.RotateVector(new_input_velocity_influence), /*add to current*/ true); //could be viable to use add velocity instead of add force. Since add velocity each frame is essentially just acceleration. But we have to be sure to add velocity scaled with time. The benefit of using add force is that it is affected by the objects mass which we can adjust. 
		}
		else
		{
			MainBody->AddForce(local_axes_rotator.RotateVector(new_input_velocity_influence)); //could be viable to use add velocity instead of add force. Since add velocity each frame is essentially just acceleration. But we have to be sure to add velocity scaled with time. The benefit of using add force is that it is affected by the objects mass which we can adjust. 
		}

		//Get current physics state after applying the input
		FVector world_velocity_linger = MainBody->GetPhysicsLinearVelocity(); //lingering velocity of the actor from previous frame
		FVector localized_velocity_linger = local_axes_rotator.UnrotateVector(world_velocity_linger); //lingering velocity now expressed using the pawn's local axis 
	
		//Based on how the pawn is moving, determine what flight mode the pawn is in 
			//mostly just applies rotational velocity to simulated_booster for now
		this->ResolveBoosterMode(localized_velocity_linger);

		//debug
		GEngine->AddOnScreenDebugMessage(11, 5, FColor::Cyan, FString::Printf(TEXT("11 Velocity Magnitude %f"), this->GetVelocity().Length()));
		GEngine->AddOnScreenDebugMessage(7, 5, FColor::Blue, FString::Printf(TEXT("7 Local Velocity %s"), *local_axes_rotator.UnrotateVector(MainBody->GetPhysicsLinearVelocity()).ToString()));
		//debug


		//Add counter velocity based on physics state to simulate drag on the pawn
		FVector X_drag_velocity = GetXDrag(localized_velocity_linger.X, DeltaTime);
		MainBody->SetPhysicsLinearVelocity(X_drag_velocity, /*addtocurrent*/ true);

		FVector YZ_drag_velocity = GetYZDrag(FVector(0, localized_velocity_linger.Y, localized_velocity_linger.Z), DeltaTime);
		MainBody->SetPhysicsLinearVelocity(YZ_drag_velocity, /*addtocurrent*/ true);

		//debug
		GEngine->AddOnScreenDebugMessage(12, 5, FColor::Purple, FString::Printf(TEXT("12 YZ Velocity Added due to force unscaled by Framerate: %f"),
			(localized_velocity_linger.Length() - local_axes_rotator.UnrotateVector(MainBody->GetPhysicsLinearVelocity()).Length()) * (1 / DeltaTime)));
		//debug

		//Finally, correct velocity if the pawn is barely moving
		ZeroVelocityCorrection(MainBody->GetPhysicsLinearVelocity(), 10);


	//!!Replicate Final Result to Server (only as the client) 
		//FTransform final_transform_this_frame = this->GetTransform(); //this is in terms of world coordiantes btw 
		FVector final_vel_this_frame = MainBody->GetPhysicsLinearVelocity();


		GEngine->AddOnScreenDebugMessage(3, 1, FColor::Green, FString::Printf(TEXT("3 Our Pawn %s, phsycsi %s"), *this->GetActorLocation().ToString(), MainBody->IsSimulatingPhysics() ? TEXT("True") : TEXT("false")));
		GEngine->AddOnScreenDebugMessage(333, 1, FColor::Green, FString::Printf(TEXT("333 Our Pawn's rotation %s"), *this->GetActorRotation().ToString()));
	
		//only do if in serrver mode fr
		if ((GEngine->GetNetMode(GWorld) == NM_Client))
		{
			this->SIM_movement_handler->PassRotationToServer(latest_rotational_velocity, this->GetActorRotation());
			this->SIM_movement_handler->PassTranslationToServer(MainBody->GetPhysicsLinearVelocity(), this->GetActorLocation());
		}
	}

	else if (this->GetLocalRole() == ROLE_SimulatedProxy)
	{
		this->SIM_movement_handler->HandleMovementOnSimulatedClient(DeltaTime);

		GEngine->AddOnScreenDebugMessage(4, 1, FColor::Emerald, FString::Printf(TEXT("4 Their Pawn %s, physics mdoe %s"), *this->GetActorLocation().ToString(), MainBody->IsSimulatingPhysics()? TEXT("True") : TEXT("false")));
		GEngine->AddOnScreenDebugMessage(44, 1, FColor::Emerald, FString::Printf(TEXT("44 Their Pawn's Rotation %s"), *this->GetActorRotation().ToString()));
	}
}



// Called to bind functionality to input
void AFlyingPawnBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}



//Only relevant to simulated proxy actors & simulate physics is turned off
//Override: capture reported location from server to be used for interpolation on client machine 
void AFlyingPawnBase::PostNetReceiveLocationAndRotation()
{
	const FRepMovement& ConstRepMovement = GetReplicatedMovement();

	// always consider Location as changed if we were spawned this tick as in that case our replicated Location was set as part of spawning, before PreNetReceive()
	if ((FRepMovement::RebaseOntoLocalOrigin(ConstRepMovement.Location, this) == GetActorLocation()
		&& ConstRepMovement.Rotation == GetActorRotation()) && (CreationTime != GetWorld()->TimeSeconds))
	{
		return;
	}

	if (GetLocalRole() == ROLE_SimulatedProxy)
	{
		// Correction to make sure pawn doesn't penetrate floor after replication rounding
		FRepMovement& MutableRepMovement = GetReplicatedMovement_Mutable();
		MutableRepMovement.Location.Z += 0.01f;

		const FVector OldLocation = GetActorLocation();
		const FQuat OldRotation = GetActorQuat();
		const FVector NewLocation = FRepMovement::RebaseOntoLocalOrigin(MutableRepMovement.Location, this);

		SetActorLocationAndRotation(NewLocation, MutableRepMovement.Rotation, /*bSweep=*/ false);

		//CUSTOM
		//GEngine->AddOnScreenDebugMessage(34522, 5, FColor::Purple, FString::Printf(TEXT("test")));
		accum += 1;
		float time_since_last_func_call = cur_time - elapsed_time;
		elapsed_time = cur_time;
		average_elapsed_time = ((average_elapsed_time * (accum - 1)) + time_since_last_func_call) / accum;

		//GEngine->AddOnScreenDebugMessage(344, 5, FColor::Purple, FString::Printf(TEXT("%d"), accum));
		GEngine->AddOnScreenDebugMessage(345, 5, FColor::Purple, FString::Printf(TEXT("345 %d"), elapsed_time));
		GEngine->AddOnScreenDebugMessage(346, 5, FColor::Purple, FString::Printf(TEXT("346 avg %d"), average_elapsed_time));
		//GEngine->AddOnScreenDebugMessage(3, 1, FColor::Purple, FString::Printf(TEXT("max %d"), average_elapsed_time));
		//GEngine->AddOnScreenDebugMessage(3, 1, FColor::Purple, FString::Printf(TEXT("min %d"), average_elapsed_time));
		//CUSTOM

	}
}



void AFlyingPawnBase::ApplyForwardThrust()
{
	FVector desired_movement = FVector(base_forward_thrust, 0, 0);
	this->AddMovementInput(desired_movement);
}



void AFlyingPawnBase::ApplyForwardBrake()
{
	//conditionally apply backward thrust when moving forward to simulate braking
	FRotator local_axis_rotator = MainBody->GetRelativeRotation();
	FVector lingering_velocity = MainBody->GetPhysicsLinearVelocity();
	float lingering_x_velocity = local_axis_rotator.UnrotateVector(lingering_velocity).X; //getting relative forward direction magnitude 

	if (lingering_x_velocity > 0)
	{
		FVector desired_movement = FVector(-1 * base_brake_power, 0, 0);
		this->AddMovementInput(desired_movement);
	}

}



void AFlyingPawnBase::ApplyVertThrustUP()
{
	FVector desired_movement = FVector(0, 0, base_upward_thrust);
	this->AddMovementInput(desired_movement);
}



void AFlyingPawnBase::ApplyVertThrustDOWN()
{
	FVector desired_movement = FVector(0, 0, -1 * base_downward_thrust);
	this->AddMovementInput(desired_movement);
}



void AFlyingPawnBase::SimDecoupledFlight(FVector localized_velocity_linger, float DeltaTime)
{
	FVector X_drag_velocity = GetXDrag(localized_velocity_linger.X, DeltaTime);
	MainBody->SetPhysicsLinearVelocity(X_drag_velocity, /*addtocurrent*/ true);

	FVector YZ_drag_velocity = GetYZDrag(FVector(0, localized_velocity_linger.Y, localized_velocity_linger.Z), DeltaTime);
	MainBody->SetPhysicsLinearVelocity(YZ_drag_velocity, /*addtocurrent*/ true);
}



void AFlyingPawnBase::SimCoupledFlight(FVector localized_velocity_linger, float DeltaTime)
{
	FVector X_drag_velocity = GetXDrag(localized_velocity_linger.X, DeltaTime);
	MainBody->SetPhysicsLinearVelocity(X_drag_velocity, /*addtocurrent*/ true);

	FVector YZ_drag_velocity = GetYZDrag(FVector(0, localized_velocity_linger.Y, localized_velocity_linger.Z), DeltaTime);
	MainBody->SetPhysicsLinearVelocity(YZ_drag_velocity, /*addtocurrent*/ true);
}



float AFlyingPawnBase::HandlePitch(float delta_time)
{
	float scaled_pitch_speed = delta_time * this->max_pitch_speed;
	return FMath::Clamp(this->pitch_input * this->pitch_sensitivity, -scaled_pitch_speed, scaled_pitch_speed);
}



void AFlyingPawnBase::ApplyYaw(bool bIsNegative)
{
	if (bIsNegative) {
		this->incoming_input_rotation.Add(0, this->base_yaw_amnt * -1 * FMath::Clamp(this->yaw_sensitivity,0 , 1), 0);
	}
	else {
		this->incoming_input_rotation.Add(0, this->base_yaw_amnt * FMath::Clamp(this->yaw_sensitivity, 0, 1), 0);
	}
}



double AFlyingPawnBase::HandleRoll(float delta_time)
{
	double scaled_roll_input = (this->roll_input * this->roll_sensitivity) + (this->digital_roll_input * this->max_roll_speed); //get roll input from sum of different types of inputs. 

	//reduce lingering velocity proportionally to each timestep
	double lingering_velocity_after_losses = this->lingering_roll_velocity * FMath::Pow(this->roll_momentum_dropoff, 2 * delta_time);

	//Set velocity of roll this frame 
	this->lingering_roll_velocity = FMath::Clamp(scaled_roll_input + lingering_velocity_after_losses, -this->max_roll_speed, this->max_roll_speed);

	//settle at a speed of 0 if we are pretty much there
	if (FMath::IsNearlyEqual(this->lingering_roll_velocity, 0, 0.001)) {
		this->lingering_roll_velocity = 0;
	}

	return this->lingering_roll_velocity * delta_time;
}



void AFlyingPawnBase::ApplyMaxRoll(bool roll_right)
{
	if (roll_right) {
		this->digital_roll_input = 1;
	}
	else {
		this->digital_roll_input = -1;
	}
}



void AFlyingPawnBase::ApplyBoost()
{
	FRotator local_axis_rotator = MainBody->GetRelativeRotation(); //world rotation since is root component
	//FVector boost_direction = this->SimulatedBooster->GetForwardVector();
	FVector boost_direction = local_axis_rotator.UnrotateVector(this->SimulatedBooster->GetForwardVector()); //for some reason, trying to get relative rotation of simulated booster is borked -- so we will grab the world rotation and then align it with the local axis and apply velocity that way.
	//MainBody->AddForce(boost_direction * this->boost_strength);
	this->AddMovementInput(boost_direction * this->boost_strength);
	GEngine->AddOnScreenDebugMessage(552, 1, FColor::Green, FString::Printf(TEXT("552 Boost Direction From Local %s"), *boost_direction.ToString()));
}



bool AFlyingPawnBase::InitialBoost()
{
	if (this->initial_boost_cd_counter <= 0) {
		FRotator local_axis_rotator = MainBody->GetRelativeRotation(); //world rotation since is root component
		//FVector boost_direction = local_axis_rotator.UnrotateVector(this->SimulatedBooster->GetForwardVector()); //for some reason, trying to get relative rotation of simulated booster is borked -- so we will grab the world rotation and then align it with the local axis and apply velocity that way. 
		//this->AddMovementInput(boost_direction * this->initial_boost_strength);
		FVector boost_direction = this->SimulatedBooster->GetForwardVector();
		MainBody->AddImpulse(boost_direction * this->initial_boost_strength);
		this->initial_boost_cd_counter = this->initial_boost_cooldown;
		return true;
	}
	else {
		return false;
	}

}


FVector2D AFlyingPawnBase::DragFromLINEAR(FVector2D lingering_velocity, float drag_strength, float delta_time, float drag_scaling)
{
	float lingering_vel_scalar = lingering_velocity.Length();
	float drag_proportion = (lingering_vel_scalar / drag_strength); //note: drag_strength => inf, makes the drag_proportion smaller - not larger. 

	//Grab a portion of lingering velocity and use it's inverse as drag 
	return -1.0f * lingering_velocity * drag_proportion * delta_time * drag_scaling;
}



float AFlyingPawnBase::DragFromLINEAR(float lingering_velocity, float drag_strength, float delta_time, float drag_scaling)
{
	//int8 drag_direction = -1 * FMath::Sign(lingering_velocity);
	float drag_proportion = (FMath::Abs(lingering_velocity) / drag_strength); //note: drag_strength => inf, makes the drag_proportion smaller - not larger. 

	//Grab a portion of lingering velocity and use it's inverse as drag 
	return -1.0f * lingering_velocity * drag_proportion * delta_time * drag_scaling;
}



FVector AFlyingPawnBase::DragFromLOG(FVector lingering_velocity, float delta_time, float drag_scaling)
{
	float lingering_vel_scalar = lingering_velocity.Length();
	float x = FMath::Max(lingering_vel_scalar, FMath::Pow(10, drag_scaling));/*
													Don't let the value "x" placed inside the logarthmic function be smaller than 10^c; where f(x) = c / log10(x).  
													otherwise c/log will produce drag proportion that is stronger than the lingering_velocity 
													TLDR: don't let f(x) > 1 by controlling x */ 
	float drag_porportion = 1 / FMath::LogX(10, x);

	//Grab a portion of lingering velocity and use it's inverse as drag 
	return -1.0f * lingering_velocity * drag_porportion * delta_time * drag_scaling;
}



float AFlyingPawnBase::DragFromLOG(float lingering_velocity, float delta_time, float drag_scaling)
{

	float x = FMath::Max(FMath::Abs(lingering_velocity), FMath::Pow(10, drag_scaling)); /*
													Don't let the value "x" placed inside the logarthmic function be smaller than 10^c; where f(x) = c / log10(x).  
													otherwise c/log will produce drag proportion that is stronger than the lingering_velocity 
													TLDR: don't let f(x) > 1 by controlling x */ 
	float drag_proportion = 1 / FMath::LogX(10, x);

	//Grab a portion of lingering velocity and use it's inverse as drag 
	return -1.0f * lingering_velocity * drag_proportion * delta_time * drag_scaling;
}



FVector AFlyingPawnBase::GetXDrag(float lingering_x_velocity, float delta_time)
{
	//first determine if we are going backwards or forwards
	if (lingering_x_velocity > 0) //currently moving forwards
	{
		/*
		* Decelerate an amount of velocity equal to current velocity (resulting velocity change over time is 1/x scaling I think technically. Because as time increases, effect of acceleration is reduced approaching 0)
		*
		* final_vel = ling_vel + accel - drag(ling_vel/c)
		*
		* Steady state exists when added velocity each frame (acceleration) equals the drag, which is equal to the actual current velocity (lingering velocity) divided by c. This only holds true when soft_cap_strength is 1 though; otherwise it would be non-linear. 
		*/
		if (this->bcoupled_flight_enabled)
		{
			if (lingering_x_velocity >= this->forward_vel_soft_cap)
			{
				float drag_proportion = lingering_x_velocity / this->forward_drag_intensity;
				float drag_proportion_after_exp = FMath::Pow(drag_proportion, this->forward_soft_cap_strength);

				return -1.0f * MainBody->GetForwardVector() * drag_proportion_after_exp * delta_time * this->global_acceleration_scaling; 
			}
		}
		else
		{
			if (lingering_x_velocity >= this->forward_vel_soft_cap + this->forward_decoupled_offset)
			{
				float drag_proportion = lingering_x_velocity / this->forward_drag_intensity;
				float drag_proportion_after_exp = FMath::Pow(drag_proportion, this->forward_soft_cap_strength);

				return -1.0f * MainBody->GetForwardVector() * drag_proportion_after_exp * delta_time * this->global_acceleration_scaling;
			}
		}


		//drag to apply when less than softcap
		return FVector(0); 

	}
	else //currently moving backwards
	{
		/*
		* Uses similar drag function as forward drag, but with a min-softcap for when velocity gets really small so that we can decelerate at a linear speed
		*/
		//drag proportion = x / c 
		float drag_proportion = FMath::Max(FMath::Abs(lingering_x_velocity) / this->backward_drag_intensity, this->backward_drag_min_soft_cap);

		//unit vector * drag amount
		return MainBody->GetForwardVector() * drag_proportion * delta_time;
	}
}



FVector AFlyingPawnBase::GetYZDrag(FVector lingering_yz_velocity, float delta_time)
{
	/*
	* Decelerate an amount of velocity equal to current velocity (resulting velocity change over time is 1/x scaling I think technically. Because as time increases, effect of acceleration is reduced approaching 0)
	* 
	* final_vel = ling_vel + accel - drag(ling_vel)
	* 
	* Steady state exists when added velocity each frame (acceleration) equals the drag which is equal to the actual current velocity (lingering velocity)
	*/
	if (this->bcoupled_flight_enabled)
	{
		if (lingering_yz_velocity.Length() > this->YZaxis_vel_soft_cap)
		{
			FRotator local_axis_rotator = MainBody->GetRelativeRotation();
			return -1.0f * local_axis_rotator.RotateVector(lingering_yz_velocity) * delta_time * this->global_acceleration_scaling;
		}
	}
	else
	{
		if (lingering_yz_velocity.Length() > this->YZaxis_vel_soft_cap + this->YZAxis_decoupled_offset)
		{
			FRotator local_axis_rotator = MainBody->GetRelativeRotation();
			return -1.0f * local_axis_rotator.RotateVector(lingering_yz_velocity) * delta_time * this->global_acceleration_scaling;
		}
	}
	
	/*
	* Dampen currenty velocity faster as it gets slower
	* Weaker dampening as velocity gets faster. 
	*/
	FRotator local_axis_rotator = MainBody->GetRelativeRotation();
	FVector drag_amount = DragFromLOG(lingering_yz_velocity, delta_time, this->YZaxis_drag_min_soft_cap);

	return local_axis_rotator.RotateVector(drag_amount); //convert the drag expressed along the local axis to world coordinates
}



void AFlyingPawnBase::ZeroVelocityCorrection(FVector lingering_velocity, float zero_speed_threshold)
{
	if (lingering_velocity.Length() <= zero_speed_threshold)
	{
		MainBody->SetPhysicsLinearVelocity(FVector(0, 0, 0));
	}
}



void AFlyingPawnBase::ResolveBoosterMode(FVector current_local_velocity)
{
	float local_forward_magnitude = current_local_velocity.X;
	float local_vertical_magnitude = FVector2D(current_local_velocity.Y, current_local_velocity.Z).Length();

	if (this->b_use_ratio_for_boostermode)
	{
		if (local_forward_magnitude * this->jet_hover_ratio_threshold > local_vertical_magnitude){
			this->BoosterHingeAttachement->SetAngularVelocityTarget(FVector(0,this->constraint_velocity_target,0));
		}
		else{
			this->BoosterHingeAttachement->SetAngularVelocityTarget(FVector(0,-this->constraint_velocity_target, 0));
		}
	}
	else
	{
		if (local_forward_magnitude >= this->jet_speed_threshold){
			this->BoosterHingeAttachement->SetAngularVelocityTarget(FVector(0, this->constraint_velocity_target, 0));
		}
		else {
			this->BoosterHingeAttachement->SetAngularVelocityTarget(FVector(0, -this->constraint_velocity_target, 0));
		}
	}
}





