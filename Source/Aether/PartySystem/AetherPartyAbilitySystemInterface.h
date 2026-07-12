// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "AetherPartyAbilitySystemInterface.generated.h"

class UAetherAbilitySystemComponent;
// This class does not need to be modified.
UINTERFACE()
class UAetherPartyAbilitySystemInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class AETHER_API IAetherPartyAbilitySystemInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual UAetherAbilitySystemComponent* GetPartyAbilitySystemComponent() const = 0;
	virtual UAetherAbilitySystemComponent* GetActiveAbilitySystemComponent() const = 0;
	
};
