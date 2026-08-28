#include "MundusGranumGameplayTags.h"
#include "Engine/EngineTypes.h"
#include "GameplayTagsManager.h"

namespace MundusGranumGameplayTags
{
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
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(CharacterState_OneHandedEquipped, "CharacterState.OneHandedEquipped", "单手持武器");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(CharacterState_TwoHandedEquipped, "CharacterState.TwoHandedEquipped", "双手持武器");
	
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(ActionState_Unoccupied, "ActionState.Unoccupied", "可主动打断当前动作");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(ActionState_Occupied, "ActionState.Occupied", "只可被动打断当前动作");
	
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(ItemCategory_Block, "ItemCategory.Block", "方块物品");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(ItemCategory_Tool, "ItemCategory.Tool", "工具物品");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(ItemCategory_Melee, "ItemCategory.Melee", "近战武器");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(ItemCategory_Consumable, "ItemCategory.Consumable", "消耗物品");
	
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InitState_Spawned, "InitState.Spawned", "1: Actor/component has initially spawned and can be extended");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InitState_DataAvailable, "InitState.DataAvailable", "2: All required data has been loaded/replicated and is ready for initialization");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InitState_DataInitialized, "InitState.DataInitialized", "3: The available data has been initialized for this actor/component, but it is not ready for full gameplay");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InitState_GameplayReady, "InitState.GameplayReady", "4: The actor/component is fully ready for active gameplay");
	
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