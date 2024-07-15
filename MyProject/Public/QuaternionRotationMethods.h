// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "QuaternionRotationMethods.generated.h"

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class MYPROJECT_API UQuaternionRotationMethods : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:

	/*
	* because quaternions are jsut better
	*/
	UFUNCTION(BlueprintCallable, Category="Transformation")
	static void SetRelativeRotationQUAT(UStaticMeshComponent* object_to_rotate, FQuat rotation);



	/*
	* because quaternions are jsut better
	*/
	UFUNCTION(BlueprintCallable, Category = "Transformation")
	static void AddRelativeRotationQUAT(UStaticMeshComponent* object_to_rotate, FQuat rotation);

	
};
