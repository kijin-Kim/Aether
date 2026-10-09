// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "WeaponHolder.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UWeaponHolder : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class AETHER_API IWeaponHolder
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual UStaticMeshComponent* GetWeaponMesh() const = 0;
};
