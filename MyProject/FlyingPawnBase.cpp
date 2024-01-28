// Fill out your copyright notice in the Description page of Project Settings.


#include "FlyingPawnBase.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AFlyingPawnBase::AFlyingPawnBase()
{
	this->bReplicates = true;
	this->SetReplicateMovement(true);
	this->incoming_input_rotation = FRotator(0);

//TEST STUFF
	accum = 0;
	elapsed_time = 0;
	average_elapsed_time = 0;
	min_time = 0;
	max_time = 0;
	lingering_roll_velocity = 0;

//Attribute Defaults
	base_upward_thrust = 100,000; //equivalent to 2500 velocity every frame (before delta_time scaling)
	base_downward_thrust = 60,000;
	base_forward_thrust = 100,000;
	base_brake_power = 50,000;
	base_yaw_amnt = 25;	
	max_pitch_speed = 200;
	max_roll_speed = 300;

	pitch_sensitivity = 0.5;
	roll_sensitivity = 15;
	yaw_sensitivity = 1.0;

	YZaxis_vel_soft_cap = 1500;
	YZaxis_drag_min_soft_cap = 1.0;

	forward_drag_intensity = 3.0;
	forward_vel_soft_cap = 3500;
	forward_soft_cap_strength = 1.07;

	backward_drag_intensity = 3.0; 
	backward_drag_min_soft_cap = 300;

	roll_momentum_dropoff = 0.00001;

	initial_boost_strength = 8,000,000;
	initial_boost_cooldown = 1;
	boost_strength = 130,000;

	b_use_ratio_for_flightmode = true;
	jet_hover_ratio_threshold = 3.0f;
	jet_speed_threshold = 0;
	constraint_velocity_target = 0.2f;

// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

// Create a dummy root component we can attach things to.
	this->MainBody = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MainBody"));
	MainBody->SetSimulatePhysics(true);
	MainBody->bReplicatePhysicsToAutonomousProxy = false;
	MainBody->SetEnableGravity(false);
	MainBody->SetGenerateOverlapEvents(true);
	RootComponent = this->MainBody;

//Instantiate Hinge Booster Technology
	this->SimulatedBooster = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SimulatedBooster"));
	SimulatedBooster->SetupAttachment(MainBody);
	SimulatedBooster->SetSimulatePhysics(true); //this component exists purely for simulation so it should have no collision
	SimulatedBooster->SetEnableGravity(false); //...
	SimulatedBooster->SetCollisionObjectType(ECollisionChannel::ECC_PhysicsBody); //...
	SimulatedBooster->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics); //...
	SimulatedBooster->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore); //...
	SimulatedBooster->AddRelativeLocation(FVector(-103, 0, -12.5));
	SimulatedBooster->SetRelativeScale3D(FVector(1.0, 0.75, 0.25));

	this->BoosterHingeAttachement = CreateDefaultSubobject<UPhysicsConstraintComponent>(TEXT("BoosterHingeAttachment"));
	BoosterHingeAttachement->SetupAttachment(MainBody);
	BoosterHingeAttachement->AddRelativeLocation(FVector(-42, 0, -10));
	BoosterHingeAttachement->SetDisableCollision(true);
	BoosterHingeAttachement->SetAngularSwing1Limit(EAngularConstraintMotion::ACM_Locked, 0); //only allowing single axis swing
	BoosterHingeAttachement->SetAngularSwing2Limit(EAngularConstraintMotion::ACM_Limited, 45); //... 45degree 
	BoosterHingeAttachement->SetAngularTwistLimit(EAngularConstraintMotion::ACM_Locked, 0); //...
	BoosterHingeAttachement->ConstraintInstance.AngularRotationOffset = FRotator(45, 0, 0);
	BoosterHingeAttachement->ConstraintInstance.EnableParentDominates();
	BoosterHingeAttachement->SetAngularDriveMode(EAngularDriveMode::TwistAndSwing);
	BoosterHingeAttachement->SetAngularVelocityDriveTwistAndSwing(false, true);
	BoosterHingeAttachement->SetAngularDriveParams(0, 10, 0);

	//Define Child and Parent if constraint (the 1 and 2 order matters)
	this->BoosterHingeAttachement->SetConstrainedComponents(this->SimulatedBooster, this->SimulatedBooster->GetFName(), this->MainBody, this->MainBody->GetFName());
	//BoosterHingeAttachement->ComponentName2 = FConstrainComponentPropName(MainBody->GetFName());

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


	cur_time = cur_time + DeltaTime;
	this->initial_boost_cd_counter = FMath::Clamp((initial_boost_cd_counter - DeltaTime), 0, initial_boost_cooldown);
	GEngine->AddOnScreenDebugMessage(344, 5, FColor::Purple, FString::Printf(TEXT("344 %s"), *FString::SanitizeFloat(DeltaTime)));
	GEngine->AddOnScreenDebugMessage(343, 5, FColor::Purple, FString::Printf(TEXT("343 %f"), cur_time));


	GEngine->AddOnScreenDebugMessage(0, 100.0, FColor::Black, TEXT("Tick from C++ test"));
//!!Process incomming rotation input
	//Handle Yaw
	this->incoming_input_rotation.Yaw = this->incoming_input_rotation.Yaw * DeltaTime;

	//Apply angular roll momentum if slowing down or changing direction
	this->incoming_input_rotation.Roll = HandleRoll(DeltaTime);

	//Handle Pitch
	this->incoming_input_rotation.Pitch = HandlePitch(DeltaTime);
	
	//Add all rotators together to the current rotation (input rotation is usually already scaled by time) 
	this->AddActorLocalRotation(this->incoming_input_rotation);

	//Clean Inputs
	this->incoming_input_rotation = FRotator(0); //is it possible to get inputs in the middle of this tick function? that would not really be that bad but kinda weird sideeffect 
	this->digital_roll_input = 0;

//!!Process incomming movement input

	FRotator local_axes_rotator = MainBody->GetRelativeRotation();
	
	FVector new_input_velocity_influence = this->ConsumeMovementInputVector(); 
	MainBody->AddForce(local_axes_rotator.RotateVector(new_input_velocity_influence)); //could be viable to use add velocity instead of add force. Since add velocity each frame is essentially just acceleration. But we have to be sure to add velocity scaled with time. The benefit of using add force is that it is affected by the objects mass which we can adjust. 


	FVector world_velocity_linger = MainBody->GetPhysicsLinearVelocity(); //lingering velocity of the actor from previous frame
	FVector localized_velocity_linger = local_axes_rotator.UnrotateVector(world_velocity_linger); //lingering velocity now expressed using the pawn's local axis 
	
	//Based on how the pawn is moving, determine what flight mode the pawn is in 
		//mostly just applies rotational velocity to simulated_booster for now
	this->ResolveFlightMode(localized_velocity_linger);


	GEngine->AddOnScreenDebugMessage(11, 5, FColor::Cyan, FString::Printf(TEXT("11 Velocity Magnitude %f"), this->GetVelocity().Length()));
	GEngine->AddOnScreenDebugMessage(7, 5, FColor::Blue, FString::Printf(TEXT("7 Local Velocity %s"), *local_axes_rotator.UnrotateVector(MainBody->GetPhysicsLinearVelocity()).ToString()));


	//Add counter velocity to simulate drag on the pawn
	FVector X_drag_velocity = GetXDrag(localized_velocity_linger.X, DeltaTime);
	MainBody->SetPhysicsLinearVelocity(X_drag_velocity, /*addtocurrent*/ true);

	FVector YZ_drag_velocity = GetYZDrag(FVector(0, localized_velocity_linger.Y, localized_velocity_linger.Z), DeltaTime);
	MainBody->SetPhysicsLinearVelocity(YZ_drag_velocity, /*addtocurrent*/ true);


	GEngine->AddOnScreenDebugMessage(12, 5, FColor::Purple, FString::Printf(TEXT("12 YZ Velocity Added due to force unscaled by Framerate: %f"),
		(localized_velocity_linger.Length() - local_axes_rotator.UnrotateVector(MainBody->GetPhysicsLinearVelocity()).Length()) * (1 / DeltaTime)));

	
		

	

	//FVector localized_drag_velocity = FVector(
	//	0,
	//	//GetXVelAfterDrag(new_input_velocity_influence.X, localized_velocity_linger.X, DeltaTime),
	//	YZ_drag_velocity.Y,
	//	YZ_drag_velocity.Z
	//	//GetYVelAfterDrag(new_input_velocity_influence.Y, localized_velocity_linger.Y, DeltaTime),
	//	//GetZVelAfterDrag(new_input_velocity_influence.Z, localized_velocity_linger.Z, DeltaTime)
	//);

	//Un-localize velocity_final: point it in the direction of the pawn. The direction of the pawn is relative to the world axis which is why we say it is un-localized. 
	//MainBody->SetPhysicsLinearVelocity(local_axes_rotator.RotateVector(localized_drag_velocity), /*addtocurrent*/ true);
	//MainBody->SetPhysicsLinearVelocity(local_axes_rotator.RotateVector(new_input_velocity_influence) * DeltaTime, true);



	//FVector just_z_stuff = FVector(0, 0 /*GetYDrag(new_input_velocity_influence.Y, localized_velocity_linger.Y, DeltaTime)*/, GetZVelAfterDrag(new_input_velocity_influence.Z, localized_velocity_linger.Z, DeltaTime));
	//new_input_velocity_influence.Z = 0.0f;
	//new_input_velocity_influence.Y = 0.0f;
	//MainBody->SetPhysicsLinearVelocity(local_axes_rotator.RotateVector(just_z_stuff), /*addtocurrent*/ false);


	//if (localized_velocity_linger.X >= 0) {
	//	localized_velocity_linger.X = localized_velocity_linger.X - ((FMath::Clamp(FMath::Abs(localized_velocity_linger.X) / 10000, 0, 1)) * localized_velocity_linger.X);
	//	//reduces forward X velocity based on how large it is currently with a linear scaling
	//	//it's not actually linear afterall
	//}
	//else {
	//	localized_velocity_linger.X = localized_velocity_linger.X + ((FMath::Clamp(
	//		(FMath::Abs(localized_velocity_linger.X) / 10000), 0, 1))
	//		* FMath::Abs(localized_velocity_linger.X));
	//	//reduces backward X velocity based on how large it is currently with a linear scaling
	//}
	//GEngine->AddOnScreenDebugMessage(6, 1, FColor::Blue, FString::Printf(TEXT("6 Lingering Velocity %s"), *localized_velocity_linger.ToString()));




	//FVector final_velocity = FVector(0);

	//We are trying to move forward right now
	//if (new_input_velocity_influence.X > 0)
	//{
	//	final_velocity.X = ComputeVelocityWithAcceleration(FVector(new_input_velocity_influence.X,0,0), FVector(localized_velocity_linger.X, 0, 0), .002f);
	//}
	////We are breaking from forward movement or trying to move backwards
	//else if (new_input_velocity_influence.X <= 0)
	//{
	//	final_velocity.X = ComputeVelocityWithDeceleration(FVector(new_input_velocity_influence.X, 0, 0), FVector(localized_velocity_linger.X, 0, 0), 0.0001f);
	//}



	//Add two velocities together, then re-express in terms of local facing direction and apply. 
		//idea to take a portion of what lingers and put it in the direction of facing rather than the direction it was originally pointing
	//FVector final_velocity = localized_velocity_linger + new_input_velocity_influence;


	//FVector last_position = this->GetLastMovementInputVector(); //make sure these are in terms of local axis, not world coordiantes
	//FVector pending_position = this->GetPendingMovementInputVector(); //...

	//Get intended local velocity based on previous movement vector

		//check for limits 

	//Apply drag forces/simulate acceleration or deceleration based on orientation

	//Apply Result Scaled By Frametime And Clean Inputs
	//FVector debug = this->ConsumeMovementInputVector();
	//this->AddActorLocalOffset(debug * DeltaTime);
	//GEngine->AddOnScreenDebugMessage(1, 1, FColor::Green, FString::Printf(TEXT("Velocity Input: %s"), *debug.ToString()));


//!!Replicate Final Result to Server (only as the client) 
	//FTransform final_transform_this_frame = this->GetTransform(); //this is in terms of world coordiantes btw 
	FVector final_vel_this_frame = MainBody->GetPhysicsLinearVelocity();

	if (Controller && Controller->GetLocalRole() == ROLE_AutonomousProxy)
	{
		GEngine->AddOnScreenDebugMessage(3, 1, FColor::Green, FString::Printf(TEXT("3 Our Pawn %s, phsycsi %s"), *this->GetActorLocation().ToString(), MainBody->IsSimulatingPhysics() ? TEXT("True") : TEXT("false")));
		//this->PassTransformToServer(this->GetActorTransform());
		this->PassVelToServer(final_vel_this_frame);
	}

	else if (GetLocalRole() == ROLE_SimulatedProxy)
	{
		GEngine->AddOnScreenDebugMessage(4, 1, FColor::Emerald, FString::Printf(TEXT("4 Their Pawn %s, physics mdoe %s"), *this->GetActorLocation().ToString(), MainBody->IsSimulatingPhysics()? TEXT("True") : TEXT("false")));
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



//Server handling of reported location from autonomous proxys actors 
void AFlyingPawnBase::PassTransformToServer_Implementation(FTransform final_world_transform)
{
	//MainBody->SetSimulatePhysics(false);
	//Intuition is to make it so the server replicates out raw positional data
	//  rather than trying to simulate physics which creates really bad stuttering.
	//	But we still want the owned client's object to simulate physics so we do this
	//	only on the aforementioned roles. 
	//We'll then apply our own smoothing function that will trigger on tick later. 
	//But annoying to have to reset this every time we talk to the server instead of once at start... 
	//Maybe the simualtephysics variable is being replicated to server. Maybe we can override this somehow, but not sure if it's worth it


	this->SetActorTransform(final_world_transform, false, (FHitResult *)nullptr, ETeleportType::None); //sets the world location and rotation verbatum of this object on the server
	//FVector stuff = this->GetActorLocation();
	//UE_LOG(LogCore, Display, TEXT("incoming position: %s"), *final_world_transform.ToString());
}


void AFlyingPawnBase::PassVelToServer_Implementation(FVector final_world_velocity)
{
	MainBody->SetAllPhysicsLinearVelocity(final_world_velocity);
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



void AFlyingPawnBase::DecoupleEngines()
{
}



void AFlyingPawnBase::RecoupleEngines()
{
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
	FVector boost_direction = this->SimulatedBooster->GetForwardVector();
	//FVector boost_direction = local_axis_rotator.UnrotateVector(this->SimulatedBooster->GetForwardVector()); //for some reason, trying to get relative rotation of simulated booster is borked -- so we will grab the world rotation and then align it with the local axis and apply velocity that way.
	MainBody->AddForce(boost_direction * this->boost_strength);
	//this->AddMovementInput(boost_direction * this->boost_strength);
	GEngine->AddOnScreenDebugMessage(552, 1, FColor::Green, FString::Printf(TEXT("552 Boost Direction From Local %s"), *boost_direction.ToString()));
}



bool AFlyingPawnBase::InitialBoost()
{
	if (this->initial_boost_cd_counter <= 0) {
		FRotator local_axis_rotator = MainBody->GetRelativeRotation(); //world rotation since is root component
		FVector boost_direction = local_axis_rotator.UnrotateVector(this->SimulatedBooster->GetForwardVector()); //for some reason, trying to get relative rotation of simulated booster is borked -- so we will grab the world rotation and then align it with the local axis and apply velocity that way. 
		this->AddMovementInput(boost_direction * this->initial_boost_strength);
		this->initial_boost_cd_counter = this->initial_boost_cooldown;
		return true;
	}
	else {
		return false;
	}

}




//For softcap math (Acceleration) 
	/*
	* Two options:
	* 1. Use y = (x+z)/x^const; x: lingering velocity, z: input velocity, y = set velocity that frame
	*	Where the growth of velocity due to lingering velocity (x) grows incresingly weaker on the resulting final velocity
	*
	* 2. Use z0+z1 - x^2; x: drag, z0: accumulated/lingering velocity, z1: additional input velocity
	*	Where drag is a portion of last frame's compounding input velocity
	*	Drag increasing exponentially until compounding velocity from adding z1 and x^2 cancel out
	*
	* Method 1 will probably have more lingering velocity so the actor will not slow down very quickly at all.
	* Method 2 has the opposite problem where it will slow down incredibly fast
	* For slowing down (decceleration) we can fix with just a branch statement to check if deccelerating or not
	* But I think method 2 will reach peak speed much faster, where method 1 has a lot more room for different speeds which could be more interesting gameplay wise.
	*/

	//For softcap math (Decceleration)
	/*
	* y = (-1)z + abs(x)^const; x: lingering velocity, y = set velocity that frame, z: input velocity when current velocity (x) is in opposite direction, const: some constant value to scale drop off speed
	*	Where the lingering velocity (x) is retained but drops off faster as actor gets slower.
	*	Input velocity (z) should be zero when no thrust is being applied, however if thrust is being applied and it is in the opposite direction of where the actor is currently moving, it would be relatively negative to lingering velocity (x).
		Allows very high speeds to be maintained but slower speeds drop off quicker.
	*/

//Always returns correct negative or positive float
float AFlyingPawnBase::VelocityFromEXPOFastDrag(float desired_vel_increase, float lingering_vel, float momentum_dropoff_strength, float delta_time, float accel_scaling)
{
	//use if drag is y = (x)^c 
	// good for strong initial drag with a lot of room at high incoming velocities 
	//float drag_velocity_fast = (this->horizontal_momentum_dropoff_threshold * target_z_velocity); //(z * x)
	//drag_velocity_fast = FMath::Pow(drag_velocity_fast, this->horizontal_momentum_dropoff_strength); //(x^c)

	//use if drag is y = (c)^x
	// usually more tempered initial drag but tightens up at higher incoming velocities quite steeply 
	float target_velocity = FMath::Abs(desired_vel_increase) + FMath::Abs(lingering_vel);  
		//side effect of abs on these two parameters is that the drag from changing directions will be stronger than if there was no abs value: since drag = c^x and x = desired + lingering. However this might be a benefit because switching direction will feel more snappy perhaps. 
		//also it's kind of necessary because the c^x function only work on positive target velocities. 

	int8 desired_direction = FMath::Sign(desired_vel_increase + lingering_vel);

	//(c^x) function
	//float drag_velocity_fast = FMath::Pow(momentum_dropoff_strength, target_velocity); 
	float drag_velocity_fast = FMath::Pow((momentum_dropoff_strength - 1) * target_velocity, 3);
	drag_velocity_fast = drag_velocity_fast * desired_direction;
	//target = neg, then drag_vel = pos 
	//target = pos, then drag_vel = neg (remember we are subtracting drag so we are already flipping the sign that way)
	
	drag_velocity_fast = FMath::Clamp(drag_velocity_fast, -FMath::Abs(lingering_vel), FMath::Abs(lingering_vel));

	float final_target_velocity_fast = ((lingering_vel) + (desired_vel_increase - drag_velocity_fast) 
		* delta_time * accel_scaling);
	
	return final_target_velocity_fast;
}



//Always returns correct positive or negative float
float AFlyingPawnBase::VelocityFromLOGSlowDrag(float desired_vel_increase, float lingering_vel, float momentum_dropoff_strength, float delta_time, float accel_scaling)
{
	int8 lingering_direction = FMath::Sign(lingering_vel);


	float x = FMath::Max(10.0f, FMath::Abs(lingering_vel));

	float drag_velocity_slow = momentum_dropoff_strength / FMath::LogX(10, x);
	
	//ling = pos, drag = neg
	//ling = neg, drag = pos 
	drag_velocity_slow = lingering_direction * drag_velocity_slow;
	float final_target_velocity_slow = lingering_vel - ((desired_vel_increase - drag_velocity_slow) * delta_time * accel_scaling);
	return final_target_velocity_slow;
}




float AFlyingPawnBase::VelocityFromSoftCapDrag(float desired_vel_increase, float lingering_vel, float soft_cap_threshold, float delta_time)
{
	return 0.0f;
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



float AFlyingPawnBase::GetXVelAfterDrag(float desired_x_velocity, float lingering_x_velocity, float delta_time)
{
	int8 acceleration_direction = FMath::Sign(desired_x_velocity);

	//accelerate forward
	if (acceleration_direction == 1)
	{
		//Unfortuantely everything about the math is the same here except for the very last parameter, which is an optional modifier. Costing us 1 branch statement D:
		return VelocityFromEXPOFastDrag(desired_x_velocity, lingering_x_velocity, this->horizontal_momentum_dropoff, delta_time, this->forward_accel_scale);
	}
	//accelerate backward
	else if (acceleration_direction == -1)
	{
		return VelocityFromEXPOFastDrag(desired_x_velocity, lingering_x_velocity, this->horizontal_momentum_dropoff, delta_time, this->backward_accel_scale);
	}
	// == 0 means no acceleration OR only passive deceleration
	else
	{
		return lingering_x_velocity*.99999999;
		//if (!FMath::IsNearlyEqual(lingering_x_velocity, 0, 0.0001f))
		//{
		//	int8 deceleration_direction = FMath::Sign(lingering_x_velocity); //direction we are going passively should be maintained

		//	//decelerating from forward momentum
		//	if (deceleration_direction == 1)
		//	{
		//		//Get the velocity from the function which produces the stronger drag on "x" (in this case "lingering_velocity") 
		//			//same thing as returning the velocity which is the smallest
		//		float velocity_from_fast_drag = VelocityFromEXPOFastDrag(0, lingering_x_velocity, this->horizontal_momentum_dropoff, delta_time, this->forward_accel_scale);

		//		float velocity_from_slow_drag = VelocityFromLOGSlowDrag(0, lingering_x_velocity, this->horizontal_momentum_dropoff_min_strength, delta_time, this->forward_accel_scale);

		//		return FMath::Min(velocity_from_fast_drag, velocity_from_slow_drag);
		//	}
		//	//decelerating from backward momentm
		//	else
		//	{
		//		//Get the velocity from the function which produces the stronger drag on "x" (in this case "lingering_velocity") 
		//			//same thing as returning the velocity which is the smallest
		//		float velocity_from_fast_drag = VelocityFromEXPOFastDrag(0, lingering_x_velocity, this->horizontal_momentum_dropoff, delta_time, this->backward_accel_scale);


		//		float velocity_from_slow_drag = VelocityFromLOGSlowDrag(0, lingering_x_velocity, this->horizontal_momentum_dropoff_min_strength, delta_time, this->backward_accel_scale);
		//		

		//		return FMath::Min(velocity_from_fast_drag, velocity_from_slow_drag);
		//	}
		//}
		//else {
		//	return 0; //we are not moving 
		//}
	}
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
		if (lingering_x_velocity >= this->forward_vel_soft_cap)
		{
			float drag_porportion = lingering_x_velocity / this->forward_drag_intensity; 
			float drag_proportion_after_exp = FMath::Pow(drag_porportion, this->forward_soft_cap_strength);

			return -1.0f * MainBody->GetForwardVector() * drag_proportion_after_exp * delta_time;
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



float AFlyingPawnBase::GetXDrag(float desired_x_velocity, float lingering_x_velocity, float delta_time)
{
	//first determine if we are going backwards or forwards
	if (lingering_x_velocity > 0) //currently moving forwards
	{

		return 0;

		//if (lingering_x_velocity >= this->forward_vel_soft_cap)
		//{
		//	return DragFromLINEAR(lingering_x_velocity, this->forward_soft_cap_strength, delta_time, this->forward_drag_intensity);
		//}
		//else
		//{
		//	return 0;
		//	//return DragFromLOG(lingering_x_velocity, delta_time, this->forward_drag_intensity);
		//}
	}
	else //currently moving backwards or not moving at all 
	{
		float drag_proportion = FMath::Clamp(lingering_x_velocity / this->backward_drag_intensity, this->backward_soft_cap_strength, 100000000);

		return drag_proportion * delta_time;
		
		
		//float backward_min_strength = 200 * delta_time;
		//return FMath::Max(backward_min_strength, DragFromLINEAR(lingering_x_velocity, this->backward_soft_cap_strength, delta_time, this->backward_drag_intensity));
		//if (FMath::Abs(lingering_x_velocity) >= this->backward_vel_soft_cap)
		//{
		//	return DragFromLINEAR(lingering_x_velocity, this->backward_soft_cap_strength, delta_time, this->backward_drag_intensity);
		//}
		//else
		//{
		//	return DragFromLOG(lingering_x_velocity, delta_time, this->backward_drag_intensity);
		//}
	}


}



float AFlyingPawnBase::GetYVelAfterDrag(float desired_y_velocity, float lingering_y_velocity, float delta_time)
{

	int8 acceleration_direction = FMath::Sign(desired_y_velocity);

	//We assume left and right drag work similarly, so we can express the directional drag purely mathematically without any additional logic

	//drag to be applied when accelerating
	if (acceleration_direction != 0) 
	{
		return VelocityFromEXPOFastDrag(desired_y_velocity, lingering_y_velocity, this->sideways_momentum_dropoff, delta_time, this->sideways_accel_scale);
	}
	//drag to be applied when decelerating 
	else
	{
		if (!FMath::IsNearlyEqual(lingering_y_velocity, 0, 0.0001f))
		{
			//use velocity obtained from the stronger drag force 
			float velocity_from_fast_drag = VelocityFromEXPOFastDrag(0, lingering_y_velocity, this->sideways_momentum_dropoff, delta_time, this->sideways_accel_scale);

			float velocity_from_slow_drag = VelocityFromLOGSlowDrag(0, lingering_y_velocity, this->sideways_momentum_dropoff_min_strength, delta_time, this->sideways_accel_scale);

			return FMath::Min(velocity_from_fast_drag, velocity_from_slow_drag);
		}
		//not moving at all
		else
		{
			return 0;
		}
	}
}


/*
* Z Drag behaves in a U shape.
* 
* Faster speeds = more drag (a sort of speed hardcap limit)
* Medium speeds = some drag
* Slower speeds = more drag (slows to a halt pretty quickly)
*/
float AFlyingPawnBase::GetZVelAfterDrag(float desired_z_velocity, float lingering_z_velocity, float delta_time)
{
	int8 acceleration_direction = FMath::Sign(desired_z_velocity);

	//accelerate upwards
	if (acceleration_direction == 1)
	{
		//Unfortuantely everything about the math is the same here except for the very last parameter, which is an optional modifier. Costing us 1 branch statement D:
		return VelocityFromEXPOFastDrag(desired_z_velocity, lingering_z_velocity, this->vertical_momentum_dropoff, delta_time, this->upward_accel_scale);
	}
	//accelerate downwards
	else if (acceleration_direction == -1)
	{
		return VelocityFromEXPOFastDrag(desired_z_velocity, lingering_z_velocity, this->vertical_momentum_dropoff, delta_time, this->downward_accel_scale);
	}
	// == 0 means no acceleration OR only passive deceleration
	else 
	{
		if (!FMath::IsNearlyEqual(lingering_z_velocity, 0, 0.0001f))
		{
			int8 deceleration_direction = FMath::Sign(lingering_z_velocity); //direction we are going passively should be maintained

			//decelerating from upward direction
			if (deceleration_direction == 1)
			{
				//Get the velocity from the function which produces the stronger drag on "x" (in this case "lingering_velocity") 
					//same thing as returning the velocity which is the smallest
				float velocity_from_fast_drag = VelocityFromEXPOFastDrag(0, lingering_z_velocity, this->vertical_momentum_dropoff, delta_time, this->upward_accel_scale);

				float velocity_from_slow_drag = VelocityFromLOGSlowDrag(0, lingering_z_velocity, this->vertical_momentum_dropoff_min_strength, delta_time, this->upward_accel_scale);

				return FMath::Min(velocity_from_fast_drag, velocity_from_slow_drag);
			}
			//decelerating from downward direction
			else
			{
				//Get the velocity from the function which produces the stronger drag on "x" (in this case "lingering_velocity") 
					//same thing as returning the velocity which is the smallest
				float velocity_from_fast_drag = VelocityFromEXPOFastDrag(0, lingering_z_velocity, this->vertical_momentum_dropoff, delta_time, this->downward_accel_scale);

				float velocity_from_slow_drag = VelocityFromLOGSlowDrag(0, lingering_z_velocity, this->vertical_momentum_dropoff_min_strength, delta_time, this->downward_accel_scale);

				return FMath::Min(velocity_from_fast_drag, velocity_from_slow_drag);
			}
		}
		else
		{
			return 0; //we aren't moving and not accelerating so continue not moving
		}
	}
}


/*
* Experimental, treat Y and Z velocity together 
* 
Easier to do this when working with vector. But could technically do with this separate Y and Z function by ensuring that applied drag in Y and Z are proportional to the magnitude of the combined vector. 

* This is mostly proof of concept
*/
FVector AFlyingPawnBase::GetYZDrag(FVector lingering_yz_velocity, float delta_time)
{
	/*
	* Decelerate an amount of velocity equal to current velocity (resulting velocity change over time is 1/x scaling I think technically. Because as time increases, effect of acceleration is reduced approaching 0)
	* 
	* final_vel = ling_vel + accel - drag(ling_vel)
	* 
	* Steady state exists when added velocity each frame (acceleration) equals the drag which is equal to the actual current velocity (lingering velocity)
	*/
	if (lingering_yz_velocity.Length() > this->YZaxis_vel_soft_cap)
	{
		FRotator local_axis_rotator = MainBody->GetRelativeRotation();
		return -1.0f * local_axis_rotator.RotateVector(lingering_yz_velocity) * delta_time;
	
	}
	/*
	* Dampen currenty velocity faster as it gets slower
	* Weaker dampening as velocity gets faster. 
	*/
	else
	{
		FRotator local_axis_rotator = MainBody->GetRelativeRotation();
		FVector drag_amount = DragFromLOG(lingering_yz_velocity, delta_time, this->YZaxis_drag_min_soft_cap);

		return local_axis_rotator.RotateVector(drag_amount); //convert the drag expressed along the local axis to world coordinates
	}
}



void AFlyingPawnBase::ResolveFlightMode(FVector current_local_velocity)
{
	float local_forward_magnitude = current_local_velocity.X;
	float local_vertical_magnitude = FVector2D(current_local_velocity.Y, current_local_velocity.Z).Length();

	if (this->b_use_ratio_for_flightmode)
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



