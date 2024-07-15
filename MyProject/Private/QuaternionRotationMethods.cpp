// Fill out your copyright notice in the Description page of Project Settings.


#include "QuaternionRotationMethods.h"

void UQuaternionRotationMethods::SetRelativeRotationQUAT(UStaticMeshComponent* object_to_rotate, FQuat rotation)
{
	object_to_rotate->SetRelativeRotationExact(FRotator(rotation));
	//object_to_rotate->SetRelativeRotation(rotation);
}

void UQuaternionRotationMethods::AddRelativeRotationQUAT(UStaticMeshComponent* object_to_rotate, FQuat rotation)
{
	object_to_rotate->AddRelativeRotation(rotation);
}

