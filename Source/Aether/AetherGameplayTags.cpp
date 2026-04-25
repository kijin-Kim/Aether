#include "AetherGameplayTags.h"

namespace AetherGameplayTags
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Test, "InputTag.Jump", "Jump Input");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Move, "InputTag.Move", "Move Input");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Look, "InputTag.Look", "Look Input");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_SwitchPartySlot1, "InputTag.SwitchPartySlot1", "Switch Party Slot 1 Input");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_SwitchPartySlot2, "InputTag.SwitchPartySlot2", "Switch Party Slot 2 Input");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_SwitchPartySlot3, "InputTag.SwitchPartySlot3", "Switch Party Slot 3 Input");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_SwitchPartySlot4, "InputTag.SwitchPartySlot4", "Switch Party Slot 4 Input");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_NormalAttack, "InputTag.NormalAttack", "Normal Attack Input");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Data_Damage, "Data.Damage", "Damage Data Tag");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Data_AuraGauge, "Data.AuraGauge", "Aura Gauge Data Tag");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Element_Pyro, "Element.Pyro", "Pyro Element Tag");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Element_Hydro, "Element.Hydro", "Hydro Element Tag");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Element_Electro, "Element.Electro", "Electro Element Tag");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Element_Cryo, "Element.Cryo", "Cryo Element Tag");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Element_Anemo, "Element.Anemo", "Anemo Element Tag");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Element_Geo, "Element.Geo", "Geo Element Tag");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Cooldown_Ability_SwitchPartySlot, "Cooldown.Ability.SwitchPartySlot", "Cooldown for Switch Party Slot Ability");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(NormalAttack_ComboWindow_Opened, "NormalAttack.ComboWindow.Opened", "Normal Attack Combo Window Opened");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(NormalAttack_ComboWindow_Closed, "NormalAttack.ComboWindow.Closed", "Normal Attack Combo Window Closed");
}
