#include "AetherGameplayEffect_HealingField.h"
#include "Aether/AbilitySystem/AttributeSet/AetherBaseAttributeSet.h"

const FName UAetherGameplayEffect_HealingField::AmountName(TEXT("HealingField.Amount"));

UAetherGameplayEffect_HealingField::UAetherGameplayEffect_HealingField()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;
	FGameplayModifierInfo Modifier;
	Modifier.Attribute = UAetherBaseAttributeSet::GetHealthAttribute();
	Modifier.ModifierOp = EGameplayModOp::Additive;
	FSetByCallerFloat Amount;
	Amount.DataName = AmountName;
	Modifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(Amount);
	Modifiers.Add(Modifier);
}
