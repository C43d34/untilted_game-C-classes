// Fill out your copyright notice in the Description page of Project Settings.


#include "FlyingPawnBase.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"


void AFlyingPawnBase::PossessedBy(AController* pawn_controller)
{
	Super::PossessedBy(pawn_controller);

	//Initialize Ability System Component On Server
	this->ASC->InitAbilityActorInfo(this, this);
	this->InitASCAttributes();
	this->InitASCAbilities();
}


void AFlyingPawnBase::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	//Initialize Ability System Component On Client
	this->ASC->InitAbilityActorInfo(this, this);
	this->InitASCAttributes();

	//Ability Input Bindings would go here & at the end of input setup if we have any
}


#if WITH_EDITOR
void AFlyingPawnBase::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	//FName PropertyName = (PropertyChangedEvent.Property != NULL) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	//FName Property_member_name = PropertyChangedEvent.MemberProperty ? PropertyChangedEvent.MemberProperty->GetFName() : NAME_None;

	//UE_LOG(LogTemp, Warning, TEXT("Property detected, maybe %s"), *PropertyName.ToString());
	//UE_LOG(LogTemp, Warning, TEXT("Property member detected, maybe %s"), *Property_member_name.ToString());


	//if ((PropertyName == GET_MEMBER_NAME_CHECKED(AFlyingPawnBase, FlyingPawn_attributes_default_table)))
	//{
	//	//implement something so that default attribute set is always present in ASC. :) 
	//	//might be best to make a custom ASC with a little variable that can be set externally that define default starting Attribute Sets. We can hide this variable from editor so its only define by C++ classes that explcitly implement the ASC. 
	//	//and lets be honest we probably will end up needing to override the ASC class to add custom functions later on anyways
	//}
	//Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif


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
	bDoMaxPitchDown = false;
	bDoMaxPitchUp = false;

//External Attribute Defaults
	base_upward_thrust = 100,000; //equivalent to 2500 velocity every frame (before delta_time scaling)
	base_downward_thrust = 60,000;
	base_forward_thrust = 200,000;
	base_brake_power = 80,000;
	base_yaw_amnt = 25;	//DEPRECATED
	yaw_acceleration = 10;
	max_yaw_speed = 70;
	max_pitch_speed = 190;
	max_roll_speed = 250;

	pitch_sensitivity = 0.5;
	roll_sensitivity = 15;
	yaw_sensitivity = 1.0;

	global_acceleration_scaling = 1.5f;
	upward_accel_scale = 1.0f;
	forward_accel_scale = 1.0f;
	backward_accel_scale = 1.0f;
	downward_accel_scale = 1.0f;

	YZaxis_vel_soft_cap = 1500;
	YZaxis_drag_min_soft_cap = 1.0;

	forward_drag_intensity = 3.0;
	forward_vel_soft_cap = 3500;
	forward_soft_cap_strength = 1.07;

	backward_drag_intensity = 3.0; 
	backward_drag_min_soft_cap = 300;

	yaw_momentum_dropoff = 10;

	roll_momentum_dropoff = 0.00001;

	initial_boost_strength = 300,000;
	initial_boost_cooldown = 0;
	base_boost_thrust = 100,000;

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
	//if (!HasAnyFlags(RF_ClassDefaultObject))
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
	//if (!HasAnyFlags(RF_ClassDefaultObject))
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


//Interpolation Component so Network Replication doesn't look ass
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

	
//GAS Technology
	//if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		this->ASC = CreateDefaultSubobject<UASC_Custom>("ASC");
		this->ASC->SetIsReplicated(true);
		this->ASC->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

		//Define attribute sets that the AbilitySystemComponent should always have defined no matter what
		TArray<TSubclassOf<UAttributeSet>> essential_FlyingPawn_attributesets = { UGASAttributes_FlyingPawn::StaticClass() };
		this->ASC->SetDefaultAttributeSets(essential_FlyingPawn_attributesets);
	}

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



void AFlyingPawnBase::UpdateBaseThrustFromDelegate(const FOnAttributeChangeData& Data)
{
	FString attribute_changed = Data.Attribute.AttributeName;
	if (attribute_changed.Contains(UGASAttributes_FlyingPawn::GetThrustpowerAttribute().GetName()))
	{
		//should safely be able to update all thrust bases regardless if it's the base attribute changing or just the directional ratio (aka lazy, but this function shouldn't be called very frequently)
		float base_thrust_power_attr = this->ASC->GetNumericAttribute(UGASAttributes_FlyingPawn::GetThrustpowerAttribute());
		this->base_forward_thrust = base_thrust_power_attr * this->ASC->GetNumericAttribute(UGASAttributes_FlyingPawn::GetThrustpower_forward_ratioAttribute());
		this->base_brake_power = base_thrust_power_attr * this->ASC->GetNumericAttribute(UGASAttributes_FlyingPawn::GetThrustpower_brake_ratioAttribute());
		this->base_upward_thrust = base_thrust_power_attr * this->ASC->GetNumericAttribute(UGASAttributes_FlyingPawn::GetThrustpower_up_ratioAttribute());
		this->base_downward_thrust = base_thrust_power_attr * this->ASC->GetNumericAttribute(UGASAttributes_FlyingPawn::GetThrustpower_down_ratioAttribute());
		this->base_boost_thrust = base_thrust_power_attr * this->ASC->GetNumericAttribute(UGASAttributes_FlyingPawn::GetThrustpower_boost_ratioAttribute());
	}
}



void AFlyingPawnBase::InitInternalBaseThrustsUsingAttributeSet()
{
	float base_thrust_power_attr = this->ASC->GetNumericAttribute(UGASAttributes_FlyingPawn::GetThrustpowerAttribute());
	this->base_forward_thrust = base_thrust_power_attr * this->ASC->GetNumericAttribute(UGASAttributes_FlyingPawn::GetThrustpower_forward_ratioAttribute());
	this->base_brake_power = base_thrust_power_attr * this->ASC->GetNumericAttribute(UGASAttributes_FlyingPawn::GetThrustpower_brake_ratioAttribute());
	this->base_upward_thrust = base_thrust_power_attr * this->ASC->GetNumericAttribute(UGASAttributes_FlyingPawn::GetThrustpower_up_ratioAttribute());
	this->base_downward_thrust = base_thrust_power_attr * this->ASC->GetNumericAttribute(UGASAttributes_FlyingPawn::GetThrustpower_down_ratioAttribute());
	this->base_boost_thrust = base_thrust_power_attr * this->ASC->GetNumericAttribute(UGASAttributes_FlyingPawn::GetThrustpower_boost_ratioAttribute());

	this->ASC->GetGameplayAttributeValueChangeDelegate(UGASAttributes_FlyingPawn::GetThrustpowerAttribute()).AddUObject(this, &AFlyingPawnBase::UpdateBaseThrustFromDelegate);
	this->ASC->GetGameplayAttributeValueChangeDelegate(UGASAttributes_FlyingPawn::GetThrustpower_forward_ratioAttribute()).AddUObject(this, &AFlyingPawnBase::UpdateBaseThrustFromDelegate);
	this->ASC->GetGameplayAttributeValueChangeDelegate(UGASAttributes_FlyingPawn::GetThrustpower_brake_ratioAttribute()).AddUObject(this, &AFlyingPawnBase::UpdateBaseThrustFromDelegate);
	this->ASC->GetGameplayAttributeValueChangeDelegate(UGASAttributes_FlyingPawn::GetThrustpower_up_ratioAttribute()).AddUObject(this, &AFlyingPawnBase::UpdateBaseThrustFromDelegate);
	this->ASC->GetGameplayAttributeValueChangeDelegate(UGASAttributes_FlyingPawn::GetThrustpower_down_ratioAttribute()).AddUObject(this, &AFlyingPawnBase::UpdateBaseThrustFromDelegate);
	this->ASC->GetGameplayAttributeValueChangeDelegate(UGASAttributes_FlyingPawn::GetThrustpower_boost_ratioAttribute()).AddUObject(this, &AFlyingPawnBase::UpdateBaseThrustFromDelegate);
}



// Called every frame
void AFlyingPawnBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//GEngine->AddOnScreenDebugMessage(0, 100.0, FColor::Black, FString::Printf(TEXT("%d"), 
	//	/*(GEngine->GetNetMode(GWorld) == NM_Client) ? TEXT("Client")
	//	: (GEngine->GetNetMode(GWorld) == NM_ListenServer) ? TEXT("ListenServer")
	//	: (GEngine->GetNetMode(GWorld) == NM_DedicatedServer) ? TEXT("DedicatedServer")
	//	: TEXT("Standalone"))*/
	//	GEngine->GetNetMode(GWorld)
	//	));

	if (this->bDo_replay == false)
	{
		if (this->saved_positions_each_frame.Num() < 5000) {
			this->saved_positions_each_frame.Add(this->GetActorLocation());
			this->saved_rotations_each_frame.Add(this->GetActorRotation());
		}
	
	}
	else if (this->bDo_replay == true)
	{
		if (this->frame_counter >= this->saved_positions_each_frame.Num() -1) {
			this->frame_counter = -1;
		}
		frame_counter++;
	}


//If this actor is a simulated proxy - or NOT the original player, we will simulate its inputs 
	if (this->GetLocalRole() == ROLE_SimulatedProxy || !IsLocallyControlled())
	{
		if (this->bDo_replay)
		{
			this->SetActorLocation(this->saved_positions_each_frame[frame_counter]);
			this->SetActorRotation(this->saved_rotations_each_frame[frame_counter]);
		}
		else
		{
			this->SIM_movement_handler->HandleSimulatingPosition(DeltaTime);
			this->SIM_movement_handler->HandleSimualtingRotation(DeltaTime);

			//Get current physics state after applying the input
			FRotator local_axes_rotator = MainBody->GetRelativeRotation();
			FVector world_momentary_velocity = MainBody->GetPhysicsLinearVelocity(); //lingering velocity of the actor from previous frame
			FVector localized_momentary_velocity = local_axes_rotator.UnrotateVector(world_momentary_velocity); //lingering velocity now expressed using the pawn's local axis 

			//simulate how booster is moving
			this->ResolveBoosterModeThisTick(localized_momentary_velocity);


			GEngine->AddOnScreenDebugMessage(4, 1, FColor::Emerald, FString::Printf(TEXT("4 Their Pawn %s, physics mdoe %s"), *this->GetActorLocation().ToString(), MainBody->IsSimulatingPhysics() ? TEXT("True") : TEXT("false")));
			GEngine->AddOnScreenDebugMessage(44, 1, FColor::Emerald, FString::Printf(TEXT("44 Their Pawn's Rotation %s"), *this->GetActorRotation().ToString()));
			GEngine->AddOnScreenDebugMessage(444, 1, FColor::Emerald, FString::Printf(TEXT("444 Their Pawn's Rotation Vel %s"), *this->SIM_movement_handler->SIM_last_rotational_velocity.ToString()));


			/*debug*/	
			//draw a line where simulated pawn SHOULD be facing. 
			DrawDebugLine(GetWorld(), this->GetActorLocation(), this->GetActorLocation() + this->SIM_movement_handler->SIM_last_rotational_goal.Vector() * 1000, FColor::Emerald, false, 0.5f, 0U, 25.0f);

			//draw sphere to project error where the pawn should be located 
			//DrawDebugSphere(GetWorld(), this->SIM_movement_handler->SIM_last_position_goal, 50, 12, FColor::Cyan);
			/*debug*/
		}
	}

//If this actor is locally controlled, we will process user inputs as if standalone
	//may want to check if GEngine and GWorld are not null pointers when doing this but anyway.
	//run standard code if standalone or an autonomous proxy in a server mode world
	//else if (Controller && (Controller->GetLocalRole() == ROLE_AutonomousProxy || GEngine->GetNetMode(GWorld) == NM_ListenServer || GEngine->GetNetMode(GWorld) == NM_Standalone))
	else if (IsLocallyControlled())
	{
		cur_time = cur_time + DeltaTime;
		this->initial_boost_cd_counter = FMath::Clamp((initial_boost_cd_counter - DeltaTime), 0, initial_boost_cooldown);

		GEngine->AddOnScreenDebugMessage(344, 5, FColor::Purple, FString::Printf(TEXT("344 %s"), *FString::SanitizeFloat(DeltaTime)));
		GEngine->AddOnScreenDebugMessage(343, 5, FColor::Purple, FString::Printf(TEXT("343 %f"), cur_time));

		GEngine->AddOnScreenDebugMessage(11, 5, FColor::Cyan, FString::Printf(TEXT("11 Velocity Magnitude %f"), this->GetVelocity().Length()));

	//!!Process incomming rotation input
		FRotator momentary_rotational_velocity = this->ResolveRotationInputsThisTick(DeltaTime);

	//!!Process incoming movement input
		FVector local_momentary_acceleration_vector = this->ResolveThrustInputsThisTick(DeltaTime);
		
		//Apply some post-input movement processing 
			//Get current physics state after applying the input
		FRotator local_axes_rotator = MainBody->GetRelativeRotation();
		FVector world_momentary_velocity = MainBody->GetPhysicsLinearVelocity(); //lingering velocity of the actor from previous frame
		FVector localized_momentary_velocity = local_axes_rotator.UnrotateVector(world_momentary_velocity); //lingering velocity now expressed using the pawn's local axis 
		
		//Based on how the pawn is moving, determine what flight mode the pawn is in 
			//mostly just applies rotational velocity to simulated_booster for now
		this->ResolveBoosterModeThisTick(localized_momentary_velocity);

		//Based on how the pawn is trying to move this tick, counteract with some drag
		this->ResolveDragDecelThisTick(DeltaTime, local_momentary_acceleration_vector, localized_momentary_velocity);

		//Finally, correct velocity if the pawn is barely moving
		this->ZeroVelocityCorrection(MainBody->GetPhysicsLinearVelocity(), 10);



		//debug
		GEngine->AddOnScreenDebugMessage(7, 5, FColor::Blue, FString::Printf(TEXT("7 Local Velocity %s"), *local_axes_rotator.UnrotateVector(MainBody->GetPhysicsLinearVelocity()).ToString()));
		//debug


		//debug
		GEngine->AddOnScreenDebugMessage(12, 5, FColor::Purple, FString::Printf(TEXT("12 YZ Velocity Added due to force unscaled by Framerate: %f"),
			(localized_momentary_velocity.Length() - local_axes_rotator.UnrotateVector(MainBody->GetPhysicsLinearVelocity()).Length()) * (1 / DeltaTime)));
		//debug


		GEngine->AddOnScreenDebugMessage(1231237, 5, FColor::Blue, FString::Printf(TEXT("Attribute Check asc %f"), this->ASC->GetNumericAttribute(UGASAttributes_FlyingPawn::GetThrustpowerAttribute())));
		



	//!!Replicate Final Result to Server (only as the client) 
		//FTransform final_transform_this_frame = this->GetTransform(); //this is in terms of world coordiantes btw 
		//FVector final_vel_this_frame = MainBody->GetPhysicsLinearVelocity();


		GEngine->AddOnScreenDebugMessage(3, 1, FColor::Green, FString::Printf(TEXT("3 Our Pawn %s, phsycsi %s"), *this->GetActorLocation().ToString(), MainBody->IsSimulatingPhysics() ? TEXT("True") : TEXT("false")));
		GEngine->AddOnScreenDebugMessage(333, 1, FColor::Green, FString::Printf(TEXT("333 Our Pawn's rotation %s"), *this->GetActorRotation().ToString()));
	
		//only do if a serrver is running
		//if we are a client (including the listen server host): broadcast some things into server
		if ((GEngine->GetNetMode(GWorld) == NM_Client) || (GEngine->GetNetMode(GWorld) == NM_ListenServer)) 
		{
			//GEngine->AddOnScreenDebugMessage(6311, 20, FColor::Black, FString::Printf(TEXT("REPLICATING OUR SHIT TO SERVER")));
			this->SIM_movement_handler->PassRotationToServer(momentary_rotational_velocity, this->GetActorRotation());
			this->SIM_movement_handler->PassTranslationToServer(MainBody->GetPhysicsLinearVelocity(), this->GetActorLocation());

		}
	}

//If this version of the actor is not a listen server, but is strictly a separate dedicated server
	/*
	* - Simulate actors movement on the server 
	*/
	else if (this->GetLocalRole() == ROLE_Authority) 
	{
		//simulate movement on server as well so server entities can interact with the pawn (such as AI)
		this->SIM_movement_handler->HandleSimulatingPosition(DeltaTime);
		this->SIM_movement_handler->HandleSimualtingRotation(DeltaTime);
	}
}



// Called to bind functionality to input
void AFlyingPawnBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	//Ability Input Bindings would go here & at the end of input setup if we have any

}


void AFlyingPawnBase::replay_last_movements()
{
	this->bDo_replay = true; 
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


  
void AFlyingPawnBase::InitASCAttributes()
{
	//Any intial Gameplay Effects apply here 

	//Goodplace to setup delegate listeners here since we expect ASC to be valid inside this function
	InitInternalBaseThrustsUsingAttributeSet(); 

	InitASCAttributesBlueprint();
}



void AFlyingPawnBase::InitASCAttributesBlueprint_Implementation()
{
	//TODO Override by blueprint
	return;
}



void AFlyingPawnBase::InitASCAbilities()
{
	return;
}



FVector AFlyingPawnBase::ResolveThrustInputsThisTick(float delta_time)
{
	FRotator local_axes_rotator = MainBody->GetRelativeRotation();
	FVector force_from_incoming_input = this->ConsumeMovementInputVector() * (double)this->ASC->GetNumericAttribute(UGASAttributes_FlyingPawn::GetAccelfactorAttribute()); //get acceleration to apply this frame * scaling factor

	/*EXPERIMENTAL increase thrust in all directions from flight mode*/
	if (!this->bcoupled_flight_enabled)
	{
		force_from_incoming_input = force_from_incoming_input * 1.5;
	}

	//Add input to the pawn
	/*EXPERIMENTAL (can't really tell a difference between velocity and thrust overall)*/
	if (this->bthrust_as_velocity)
	{
		force_from_incoming_input = delta_time * (force_from_incoming_input / MainBody->GetMass());
		MainBody->SetAllPhysicsLinearVelocity(local_axes_rotator.RotateVector(force_from_incoming_input), /*add to current*/ true); //could be viable to use add velocity instead of add force. Since add velocity each frame is essentially just acceleration. But we have to be sure to add velocity scaled with time. The benefit of using add force is that it is affected by the objects mass which we can adjust. 
	}
	else
	{
		MainBody->AddForce(local_axes_rotator.RotateVector(force_from_incoming_input)); //could be viable to use add velocity instead of add force. Since add velocity each frame is essentially just acceleration. But we have to be sure to add velocity scaled with time. The benefit of using add force is that it is affected by the objects mass which we can adjust. 
	}

	return force_from_incoming_input;
}



FVector AFlyingPawnBase::GetLastMovementInputVectorUNSCALED()
{
	FVector raw_movement_input = this->GetLastMovementInputVector();
	float x_input = raw_movement_input.X >= 0 ? raw_movement_input.X / this->ASC->GetNumericAttribute(UGASAttributes_FlyingPawn::GetAccelfactor_forwardAttribute()) : raw_movement_input.X / this->ASC->GetNumericAttribute(UGASAttributes_FlyingPawn::GetAccelfactor_backwardAttribute());
	float y_input = raw_movement_input.Y;
	float z_input = raw_movement_input.Z >= 0 ? raw_movement_input.Z / this->ASC->GetNumericAttribute(UGASAttributes_FlyingPawn::GetAccelfactor_upAttribute()) : raw_movement_input.Z / this->ASC->GetNumericAttribute(UGASAttributes_FlyingPawn::GetAccelfactor_downAttribute());
	return FVector(x_input, y_input, z_input);
}



void AFlyingPawnBase::ApplyForwardThrust()
{
	FVector desired_movement = FVector(base_forward_thrust, 0, 0);
	this->AddMovementInput(desired_movement * this->ASC->GetNumericAttribute(UGASAttributes_FlyingPawn::GetAccelfactor_forwardAttribute()));
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
		this->AddMovementInput(desired_movement * this->ASC->GetNumericAttribute(UGASAttributes_FlyingPawn::GetAccelfactor_backwardAttribute()));
	}

}



void AFlyingPawnBase::ApplyVertThrustUP()
{
	FVector desired_movement = FVector(0, 0, base_upward_thrust);
	this->AddMovementInput(desired_movement * this->ASC->GetNumericAttribute(UGASAttributes_FlyingPawn::GetAccelfactor_upAttribute()));
}



void AFlyingPawnBase::ApplyVertThrustDOWN()
{
	FVector desired_movement = FVector(0, 0, -1 * base_downward_thrust);
	this->AddMovementInput(desired_movement * this->ASC->GetNumericAttribute(UGASAttributes_FlyingPawn::GetAccelfactor_downAttribute()));
}




FRotator AFlyingPawnBase::ResolveRotationInputsThisTick(float delta_time)
{
	FRotator latest_rotational_velocity;

	//Handle Yaw
	this->incoming_input_rotation.Yaw = HandleYaw(delta_time);

	//Apply angular roll momentum if slowing down or changing direction
	this->incoming_input_rotation.Roll = bcoupled_flight_enabled ? HandleRoll(delta_time) : 0.5f * HandleRoll(delta_time);

	//Handle Pitch
	this->incoming_input_rotation.Pitch = bcoupled_flight_enabled ? HandlePitch(delta_time) : 0.5f * HandlePitch(delta_time);

	//Add all rotators together to the current rotation (input rotation is usually already scaled by time) 
	this->AddActorLocalRotation(this->incoming_input_rotation);

	//velocity to pass over to simulated proxies (we don't actually use this variable inside autonmous proxy
	latest_rotational_velocity = this->incoming_input_rotation * (1 / delta_time); //unscale by delta time to get value as time independent velocity

	//Flush Inputs
	this->incoming_input_rotation = FRotator(0); //is it possible to get inputs in the middle of this tick function? that would not really be that bad but kinda weird sideeffect 
	this->digital_roll_input = 0;


	return latest_rotational_velocity;
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
	float scaled_max_pitch_speed = delta_time * this->ASC->GetNumericAttribute(UGASAttributes_FlyingPawn::GetPitchspeedAttribute());
	float analog_pitch_input = this->pitch_input * this->pitch_sensitivity;
	float digital_pitch_input = 0;
	if (bDoMaxPitchUp){
		digital_pitch_input += scaled_max_pitch_speed;
	}

	if(bDoMaxPitchDown){
		digital_pitch_input -= scaled_max_pitch_speed;
	}

	return FMath::Clamp(analog_pitch_input + digital_pitch_input, -scaled_max_pitch_speed, scaled_max_pitch_speed);

}



void AFlyingPawnBase::ApplyYaw(bool bIsNegative)
{
	if (bIsNegative) {
		this->yaw_input = -1 * this->ASC->GetNumericAttribute(UGASAttributes_FlyingPawn::GetYawspeed_accelAttribute());
		//this->incoming_input_rotation.Add(0, this->base_yaw_amnt * -1 * FMath::Clamp(this->yaw_sensitivity,0 , 1), 0);
	}
	else {
		this->yaw_input = this->ASC->GetNumericAttribute(UGASAttributes_FlyingPawn::GetYawspeed_accelAttribute());
		//this->incoming_input_rotation.Add(0, this->base_yaw_amnt * FMath::Clamp(this->yaw_sensitivity, 0, 1), 0);
	}
}



float AFlyingPawnBase::HandleYaw(float delta_time)
{ 
	if (this->bUse_yaw_roll_input) //special analog yaw input (with mouse)
	{
		float scaled_max_yaw_speed = this->special_yaw_speed > 0 ? delta_time * this->special_yaw_speed : delta_time * this->ASC->GetNumericAttribute(UGASAttributes_FlyingPawn::GetPitchspeedAttribute()); //use max pitch speed as base if we didn't set a unique max speed for yaw 
		float yaw_roll_ratio = 1 - (this->special_yaw_roll_ratio / 1); //if special ratio is 0, apply 100% of incoming yaw input
		float analog_yaw_input = this->yaw_roll_input * this->pitch_sensitivity * yaw_roll_ratio; 

		return FMath::Clamp(analog_yaw_input, -scaled_max_yaw_speed, scaled_max_yaw_speed);
	}
	else //original digital yaw input method
	{
		//if no yaw input, decay from lingering yaw velocity by flat amount
		if (this->yaw_input == 0)
		{
			int yaw_direction = FMath::Sign(this->lingering_yaw_velocity);
			float abs_yaw_velocity = yaw_direction * (this->lingering_yaw_velocity - yaw_direction * (this->yaw_momentum_dropoff * delta_time));
			this->lingering_yaw_velocity = yaw_direction * FMath::Clamp(abs_yaw_velocity, 0, this->max_yaw_speed); //make sure when reducing yaw velocity, it doesn't go past 0. 
			return this->lingering_yaw_velocity * delta_time;
		}
		else
		{
			//check if yaw input direction and current lingering direction are going same way
			if ((this->yaw_input >= 0 && this->lingering_yaw_velocity >= 0) || (this->yaw_input <= 0 && this->lingering_yaw_velocity <= 0))
			{
				this->lingering_yaw_velocity += this->yaw_input * delta_time;
			}
			else
			{
				//if opposite input and lingering, reset lingering to 0 to "start from scratch" 
				this->lingering_yaw_velocity = this->yaw_input * delta_time;
			}

			this->lingering_yaw_velocity = FMath::Clamp(this->lingering_yaw_velocity, -this->max_yaw_speed, this->max_yaw_speed);
			this->yaw_input = 0; //reset yaw input for next tick 
		}

		return this->lingering_yaw_velocity * delta_time;
	}
}



double AFlyingPawnBase::HandleRoll(float delta_time)
{
	if (this->bUse_yaw_roll_input) //special analog roll that combines with analog yaw from mouse
	{
		float scaled_max_roll_speed = this->special_roll_speed > 0 ? delta_time * this->special_roll_speed : delta_time * this->ASC->GetNumericAttribute(UGASAttributes_FlyingPawn::GetRollspeedAttribute()); //use normal max roll speed as base if we didn't set a unique max speed for roll 
		float yaw_roll_ratio = this->special_yaw_roll_ratio; //if special ratio is 1, apply 100% of incoming roll input
		float analog_roll_input = this->yaw_roll_input * this->roll_sensitivity * yaw_roll_ratio;

		//return FMath::Clamp(analog_roll_input, -scaled_max_roll_speed, scaled_max_roll_speed);




		//reduce lingering velocity proportionally to each timestep
		double lingering_velocity_after_losses = this->lingering_roll_velocity * FMath::Pow(this->roll_momentum_dropoff, 2 * delta_time);

		//Set velocity of roll this frame 
		this->lingering_roll_velocity = FMath::Clamp(analog_roll_input + lingering_velocity_after_losses, -scaled_max_roll_speed/delta_time, scaled_max_roll_speed/delta_time);

		//settle at a speed of 0 if we are pretty much there
		if (FMath::IsNearlyEqual(this->lingering_roll_velocity, 0, 0.001)) {
			this->lingering_roll_velocity = 0;
		}

		return this->lingering_roll_velocity * delta_time;
	}
	else //normal analog roll without yaw input
	{
		double scaled_roll_input = (this->roll_input * this->roll_sensitivity) + (this->digital_roll_input * this->ASC->GetNumericAttribute(UGASAttributes_FlyingPawn::GetRollspeedAttribute())); //get roll input from sum of different types of inputs. 

		//reduce lingering velocity proportionally to each timestep
		double lingering_velocity_after_losses = this->lingering_roll_velocity * FMath::Pow(this->roll_momentum_dropoff, 2 * delta_time);

		//Set velocity of roll this frame 
		this->lingering_roll_velocity = FMath::Clamp(scaled_roll_input + lingering_velocity_after_losses, -this->ASC->GetNumericAttribute(UGASAttributes_FlyingPawn::GetRollspeedAttribute()), this->ASC->GetNumericAttribute(UGASAttributes_FlyingPawn::GetRollspeedAttribute()));

		//settle at a speed of 0 if we are pretty much there
		if (FMath::IsNearlyEqual(this->lingering_roll_velocity, 0, 0.001)) {
			this->lingering_roll_velocity = 0;
		}

		return this->lingering_roll_velocity * delta_time;
	}
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
	//MainBody->AddForce(boost_direction * this->ASC->GetNumericAttribute(UGASAttributes_FlyingPawn::GetThrustpower_boost_ratioAttribute()));


	//we know the booster is only going to be affecting some combination of forward and upward movement, so make sure it's applied force is scaled appropriately 
	boost_direction.X *= this->ASC->GetNumericAttribute(UGASAttributes_FlyingPawn::GetAccelfactor_forwardAttribute());
	boost_direction.Z *= this->ASC->GetNumericAttribute(UGASAttributes_FlyingPawn::GetAccelfactor_upAttribute()); 



	this->AddMovementInput(boost_direction * this->base_boost_thrust);
	GEngine->AddOnScreenDebugMessage(552, 1, FColor::Green, FString::Printf(TEXT("552 Boost Direction From Local %s"), *boost_direction.ToString()));
}



bool AFlyingPawnBase::ImpulseBoost(float boost_strength_scaling)
{
	if (this->initial_boost_cd_counter <= 0) {
		FRotator local_axis_rotator = MainBody->GetRelativeRotation(); //world rotation since is root component
		//FVector boost_direction = local_axis_rotator.UnrotateVector(this->SimulatedBooster->GetForwardVector()); //for some reason, trying to get relative rotation of simulated booster is borked -- so we will grab the world rotation and then align it with the local axis and apply velocity that way. 
		//this->AddMovementInput(boost_direction * this->ASC->GetNumericAttribute(UGASAttributes_FlyingPawn::GetImpulsepower_boostAttribute()));
		FVector boost_direction = this->SimulatedBooster->GetForwardVector();
		
		MainBody->AddImpulse(boost_direction * this->ASC->GetNumericAttribute(UGASAttributes_FlyingPawn::GetImpulsepower_boostAttribute()) * boost_strength_scaling);
		this->initial_boost_cd_counter = this->initial_boost_cooldown;
		return true;
	}
	else {
		return false;
	}

}



FVector AFlyingPawnBase::ResolveDragDecelThisTick(float delta_time, FVector input_forces_this_tick, FVector localized_current_velocity)
{
	//Get current velocity of pawn (after all other input forces were applied) 
	double x_vel = localized_current_velocity.X; 
	double y_vel = localized_current_velocity.Y;
	double z_vel = localized_current_velocity.Z; 




	//Add counter velocity based on physics state to simulate drag on the pawn
		//Scale velocities by appropriate acceleration values ONLY IF acceleration was indeed applied to those directions within this frame
		/*
		* Drag each frame is represented by applying some change in velocity to the pawn. This means we are effectively counteracting the acceleration provided by input forces. Therefore we need to make sure the proportion of such acceleration is appropriately scaled with same direction it is oppossed to.
		* But we only need to apply this scaling factor if those input forces are actively changing the velocity. If they are not, drag is merely applied on lingering velocity.
		*
		* Desired side effect is also the velocity soft-caps are maintained because acceleration from thrust and drag maintain proportionality
		* (Unfortuantely this isn't the case for some reason velocity soft-caps get slightly shifted but VERY minorly so currently not investigating this) 
		*/
	FVector X_drag_velocity = GetXDrag(x_vel, delta_time);

	if (input_forces_this_tick.X > 0) {
		X_drag_velocity *= this->ASC->GetNumericAttribute(UGASAttributes_FlyingPawn::GetAccelfactor_forwardAttribute());
	}
	else if (input_forces_this_tick.X < 0) {
		X_drag_velocity *= this->ASC->GetNumericAttribute(UGASAttributes_FlyingPawn::GetAccelfactor_backwardAttribute());
	}

	FVector YZ_drag_velocity = GetYZDrag(FVector(0, y_vel, z_vel), delta_time);
	FRotator local_axis_rotator = MainBody->GetRelativeRotation(); 

	if (input_forces_this_tick.Z > 0) {
		FVector localized_yz_drag = local_axis_rotator.UnrotateVector(YZ_drag_velocity);
		localized_yz_drag.Z *= this->ASC->GetNumericAttribute(UGASAttributes_FlyingPawn::GetAccelfactor_upAttribute()); //apply acceleration scaling only along parts of drag vector that deal with the Z direction
		YZ_drag_velocity = local_axis_rotator.RotateVector(localized_yz_drag);
	}
	else {
		FVector localized_yz_drag = local_axis_rotator.UnrotateVector(YZ_drag_velocity);
		localized_yz_drag.Z *= this->ASC->GetNumericAttribute(UGASAttributes_FlyingPawn::GetAccelfactor_downAttribute());  //apply acceleration scaling only along parts of drag vector that deal with the Z direction
		YZ_drag_velocity = local_axis_rotator.RotateVector(localized_yz_drag);
	}



	MainBody->SetPhysicsLinearVelocity(X_drag_velocity, /*addtocurrent*/ true);
	MainBody->SetPhysicsLinearVelocity(YZ_drag_velocity, /*addtocurrent*/ true);


	return (X_drag_velocity + YZ_drag_velocity);
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
	if (FMath::IsNearlyEqual(lingering_x_velocity, 0, 1)) //dont do drag if we aren't moving 
	{
		return FVector(0);
	}

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
				//OG DRAG I GUESS
				//float drag_proportion = lingering_x_velocity / this->forward_drag_intensity;
				//float drag_proportion_after_exp = FMath::Pow(drag_proportion, this->forward_soft_cap_strength);
				//return -1.0f * MainBody->GetForwardVector() * drag_proportion_after_exp * delta_time * this->ASC->GetNumericAttribute(UGASAttributes_FlyingPawn::GetAccelfactorAttribute());
				
				//TANGENT FUNCTION DRAG ZAMN
				//double period_scale = 10000;
				//double shift_phase = 9000;	
				//double function_curvature = 1000;
				//double magnitude_shift = 2200;
				////check if velocity surpassed (or atleast close to) asymptote due to impulse or other shenangains?
				//double x = lingering_x_velocity >= 22000 ? 22000 : lingering_x_velocity;
				//double math = (function_curvature / (FMath::Tan((lingering_x_velocity + shift_phase) / period_scale))) - magnitude_shift;
				//return math * MainBody->GetForwardVector() * delta_time * this->ASC->GetNumericAttribute(UGASAttributes_FlyingPawn::GetAccelfactorAttribute()); //dont need to multiply by -1 for this 

				//1/x-c FUNCTION DRAG BOINGOZOINGO
				double curvature = 15 * 1e6; //millions ._. 15
				double asymptote = 22000; 
				double magnitude_shift = 1300; //0
				//check if velocity surpassed (or atleast close to) asymptote due to impulse or other shenangains?
				double x = lingering_x_velocity >= asymptote - 500 ? asymptote - 500 : lingering_x_velocity;
				double math = (curvature / (x - asymptote)) - magnitude_shift;
				return math * MainBody->GetForwardVector() * delta_time * this->ASC->GetNumericAttribute(UGASAttributes_FlyingPawn::GetAccelfactorAttribute());


			}
		}
		else
		{
			if (lingering_x_velocity >= this->forward_vel_soft_cap + this->forward_decoupled_offset)
			{
				float drag_proportion = lingering_x_velocity / this->forward_drag_intensity;
				float drag_proportion_after_exp = FMath::Pow(drag_proportion, this->forward_soft_cap_strength);

				return -1.0f * MainBody->GetForwardVector() * drag_proportion_after_exp * delta_time * this->ASC->GetNumericAttribute(UGASAttributes_FlyingPawn::GetAccelfactorAttribute());
			}
		}


		//drag to apply when less than softcap
		return -1.0f * MainBody->GetForwardVector() * 100 /*could be modified by minimum forward drag*/ * delta_time * this->ASC->GetNumericAttribute(UGASAttributes_FlyingPawn::GetAccelfactorAttribute()); 

	}
	else //currently moving backwards
	{
		/*
		* Uses similar drag function as forward drag, but with a min-softcap for when velocity gets really small so that we can decelerate at a linear speed
		*/
		//drag proportion = x / c 
		float drag_proportion = FMath::Max(FMath::Abs(lingering_x_velocity) * this->ASC->GetNumericAttribute(UGASAttributes_FlyingPawn::GetDragpower_backwardAttribute()), this->ASC->GetNumericAttribute(UGASAttributes_FlyingPawn::GetDragpower_backward_minimumAttribute()));

		//unit vector * drag amount
		return MainBody->GetForwardVector() * drag_proportion * delta_time * this->ASC->GetNumericAttribute(UGASAttributes_FlyingPawn::GetAccelfactorAttribute());
	}
}



FVector AFlyingPawnBase::GetYZDrag(FVector lingering_yz_velocity, float delta_time)
{
	if (FMath::IsNearlyEqual(lingering_yz_velocity.Length(), 0, 1)) //dont do drag if we aren't moving 
	{
		return FVector(0);
	}

	/*
	* Decelerate an amount of velocity equal to current velocity (resulting velocity change over time is 1/x scaling I think technically. Because as time increases, effect of acceleration is reduced approaching 0)
	* 
	* final_vel = ling_vel + accel - drag(c*ling_vel)^exp
	* 
	* Steady state exists when added velocity each frame (acceleration) equals the drag which is equal to the actual current velocity (lingering velocity)
	*/
	if (this->bcoupled_flight_enabled)
	{
		if (lingering_yz_velocity.Length() > this->YZaxis_vel_soft_cap)
		{
			FRotator local_axis_rotator = MainBody->GetRelativeRotation();

			//double math = FMath::Pow(.001 * (lingering_yz_velocity.Length()) + 6, 3);
			//FVector thing = math * lingering_yz_velocity.GetSafeNormal();
			//return -1.0f * local_axis_rotator.RotateVector(thing) * delta_time * this->ASC->GetNumericAttribute(UGASAttributes_FlyingPawn::GetAccelfactorAttribute());

			//TANGENT FUNCTION DRAG ZAMN
			//double period_scale = 5000;
			//double shift_phase = 3000;	
			//double function_curvature = 1000;
			//double magnitude_shift = 3000;
			//double math = (function_curvature / (FMath::Tan((lingering_yz_velocity.Length() + shift_phase) / period_scale))) - magnitude_shift;
			//FVector thing = math * lingering_yz_velocity.GetSafeNormal();
			//return local_axis_rotator.RotateVector(thing) * delta_time * this->ASC->GetNumericAttribute(UGASAttributes_FlyingPawn::GetAccelfactorAttribute()); //dont need to multiply by -1 for this 

			//1/x-c FUNCTION DRAG BOINGOZOINGO
			double curvature = 5 * 1e6; //millions ._.
			double asymptote = 9000;
			double magnitude_shift = 1700;
			//check if velocity surpassed (or atleast close to) asymptote due to impulse or other shenangains?
			double x = lingering_yz_velocity.Length() >= asymptote - 500 ? asymptote - 500 : lingering_yz_velocity.Length();
			double math = (curvature / (x - asymptote)) - magnitude_shift;
			FVector thing = math * lingering_yz_velocity.GetSafeNormal();
			return local_axis_rotator.RotateVector(thing) * delta_time * this->ASC->GetNumericAttribute(UGASAttributes_FlyingPawn::GetAccelfactorAttribute());



			//OG bingobongop
			//return -1.0f * local_axis_rotator.RotateVector(lingering_yz_velocity) * delta_time * this->ASC->GetNumericAttribute(UGASAttributes_FlyingPawn::GetAccelfactorAttribute());
		}
	}
	else
	{
		if (lingering_yz_velocity.Length() > this->YZaxis_vel_soft_cap + this->YZAxis_decoupled_offset)
		{
			FRotator local_axis_rotator = MainBody->GetRelativeRotation();
			return -1.0f * local_axis_rotator.RotateVector(lingering_yz_velocity) * delta_time * this->ASC->GetNumericAttribute(UGASAttributes_FlyingPawn::GetAccelfactorAttribute());
		}
	}

	/*
	* Dampen currenty velocity faster as it gets slower
	* Weaker dampening as velocity gets faster.
	*/
	FRotator local_axis_rotator = MainBody->GetRelativeRotation();
	FVector drag_amount = DragFromLOG(lingering_yz_velocity, delta_time, 1 /*could be modified by minimum yz drag*/);

	return local_axis_rotator.RotateVector(drag_amount); //convert the drag expressed along the local axis to world coordinates
}



void AFlyingPawnBase::ZeroVelocityCorrection(FVector lingering_velocity, float zero_speed_threshold)
{
	if (lingering_velocity.Length() <= zero_speed_threshold)
	{
		MainBody->SetPhysicsLinearVelocity(FVector(0, 0, 0));
	}
}



void AFlyingPawnBase::ResolveBoosterModeThisTick(FVector current_local_velocity)
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





