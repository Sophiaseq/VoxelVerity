#include "MundusGranumGameplayTags.h"
#include "Engine/EngineTypes.h"
#include "GameplayTagsManager.h"

namespace MundusGranumGameplayTags
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_Behavior_SurvivesDeath, "Ability.Behavior.SurvivesDeath", "An ability with this type tag should not be canceled due to death.");
	
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Move, "InputTag.Move", "角色移动轴输入");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Look_Mouse, "InputTag.Look.Mouse", "鼠标视角转动");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Jump, "InputTag.Jump", "角色跳跃");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Sprint, "InputTag.Sprint", "移动加速");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Pickup, "InputTag.Pickup", "拾取放置的物品");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_UseLeftHandItem, "InputTag.UseLeftHandItem", "使用左手的物品");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_UseRightHandItem, "InputTag.UseRightHandItem", "使用右手的物品");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_SelectItem, "InputTag.SelectItem", "滚轮切换快捷栏");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_SlowWalk, "InputTag.SlowWalk", "角色慢走");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Movement_Mode_Walking, "Movement.Mode.Walking", "行走状态");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Movement_Mode_Falling, "Movement.Mode.Falling", "下落状态");
	
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(CharacterState_Unequipped, "CharacterState.Unequipped", "未手持物品");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(CharacterState_Equipped_LeftHand, "CharacterState.Equipped.LeftHand", "左手持");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(CharacterState_Equipped_RightHand_Sword, "CharacterState.Equipped.RightHand.Sword", "右手持剑");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(CharacterState_Rigidity_SelfAction, "CharacterState.Rigidity.SelfAction", "僵直状态，动作时附加，只能被受击动作打断");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(CharacterState_Rigidity_Hit, "CharacterState.Rigidity.Hit", "被攻击时施加的状态");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(CharacterState_Block, "CharacterState.Block", "普通格挡");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(CharacterState_Block_Parry, "CharacterState.Block.Parry", "弹刀");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(CharacterState_Death, "Status.Death", "Target has the death status.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(CharacterState_Death_Dying, "Status.Death.Dying", "Target has begun the death process.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(CharacterState_Death_Dead, "Status.Death.Dead", "Target has finished the death process.");
	
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameplayEvent_Death, "GameplayEvent.Death", "Event that fires on death. This event only fires on the server.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameplayEvent_Reset, "GameplayEvent.Reset", "Event that fires once a player reset is executed.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameplayEvent_RequestReset, "GameplayEvent.RequestReset", "Event to request a player's pawn to be instantly replaced with a new one at a valid spawn location.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameplayEvent_HitReact, "GameplayEvent.HitReact", "角色的AbilitySystemComponent被添加CharacterState.Rigidity.Hit时触发的事件")
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(HitReact_Direction_Front, "HitReact.Direction.Front", "受击方向：正面");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(HitReact_Direction_Back, "HitReact.Direction.Back", "受击方向：背面");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(HitReact_Direction_Left, "HitReact.Direction.Left", "受击方向：左侧");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(HitReact_Direction_Right, "HitReact.Direction.Right", "受击方向：右侧");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Damage_Melee_Sharpness, "Damage.Melee.Sharpness", "近战武器的锋利度属性");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Damage_Melee_Quality, "Damage.Melee.Quality", "近战武器的质量属性")
	
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(ItemCategory_Block, "ItemCategory.Block", "方块物品");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(ItemCategory_Tool, "ItemCategory.Tool", "工具物品");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(ItemCategory_Weapon, "ItemCategory.Weapon", "近战武器");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(ItemCategory_Consumable, "ItemCategory.Consumable", "消耗物品");
	
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InitState_Spawned, "InitState.Spawned", "1: Actor/component has initially spawned and can be extended");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InitState_DataAvailable, "InitState.DataAvailable", "2: All required data has been loaded/replicated and is ready for initialization");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InitState_DataInitialized, "InitState.DataInitialized", "3: The available data has been initialized for this actor/component, but it is not ready for full gameplay");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InitState_GameplayReady, "InitState.GameplayReady", "4: The actor/component is fully ready for active gameplay");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Component_Mesh_Weapon, "Component.Mesh.Weapon", "武器网格的组件标签")
	
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Attribute_Primary_Strength, "Attributes.Primary.Strength", "力量");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Attribute_Primary_Dexterity, "Attributes.Primary.Dexterity", "敏捷");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Attribute_Primary_Constitution, "Attributes.Primary.Constitution", "体质");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Attribute_Primary_Intelligence, "Attributes.Primary.Intelligence", "智力");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Attribute_Primary_Perception, "Attributes.Primary.Perception", "感知");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Attribute_Primary_Luck, "Attributes.Primary.Luck", "运气");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Attribute_Vital_Health, "Attributes.Vital.Health", "当前生命值");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Attribute_Vital_MaxHealth, "Attributes.Vital.MaxHealth", "最大生命值");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Attribute_Vital_Mana, "Attributes.Vital.Mana", "当前法力值");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Attribute_Vital_MaxMana, "Attributes.Vital.MaxMana", "最大法力值");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Attribute_Vital_Stamina, "Attributes.Vital.Stamina", "当前体力值");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Attribute_Vital_MaxStamina, "Attributes.Vital.MaxStamina", "最大体力值");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Attribute_Combat_AttackPower, "Attributes.Combat.AttackPower", "攻击力");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Attribute_Combat_DefensePower, "Attributes.Combat.DefensePower", "防御力");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Attribute_Combat_CriticalRate, "Attributes.Combat.CriticalRate", "暴击率（0~1）");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Attribute_Combat_CriticalDamage, "Attributes.Combat.CriticalDamage", "暴击伤害倍率");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Attribute_Combat_IncomingDamage, "Attributes.Meta.IncomingDamage", "本次受到的伤害（meta 缓冲）");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Attribute_Combat_IncomingHealing, "Attributes.Meta.IncomingHealing", "本次受到的治疗（meta 缓冲）");
	
	const TMap<uint8, FGameplayTag> MovementModeTagMap =
	{
		{ MOVE_Walking, Movement_Mode_Walking },
		{ MOVE_Falling, Movement_Mode_Falling },
	};
	
	FGameplayTag FindTagByString(const FString& TagString, bool bMatchPartialString)
    	{
    		const UGameplayTagsManager& Manager = UGameplayTagsManager::Get();
    		FGameplayTag Tag = Manager.RequestGameplayTag(FName(*TagString), false);
    
    		if (!Tag.IsValid() && bMatchPartialString)
    		{
    			FGameplayTagContainer AllTags;
    			Manager.RequestAllGameplayTags(AllTags, true);
    
    			for (const FGameplayTag& TestTag : AllTags)
    			{
    				if (TestTag.ToString().Contains(TagString))
    				{
    					UE_LOG(LogTemp, Display, TEXT("Could not find exact match for tag [%s] but found partial match on tag [%s]."), *TagString, *TestTag.ToString());
    					Tag = TestTag;
    					break;
    				}
    			}
    		}
    
    		return Tag;
    	}
}