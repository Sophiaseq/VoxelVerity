# 在UE5中实现类似于我的世界一样的可生成可破坏的世界。
1.让方块更小一开始设置为原本方块的1/8，根据性能在项目后期可设置为更小
2.通过不同的函数将方块之间的面渲染成曲面
3.可自定义方块模板，比如一个原版大小的方块就是基础模板

# 物品

**物品有三种显示方式：在物品栏中，作为掉落物，被放置在世界中**
**物品可左手持，可右手持**

1.FItemBaseData 物品的用于显示在物品栏的基本信息
2.FEquipDisplayData 手持时的装备显示数据
3.UMGItemDefinition

```cpp
class MUNDUSGRANUM_API UMGItemDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<AMGItemBehavior> ItemBehavior;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FItemBaseData BaseData;

	// 丢在地上时显示的 3D 网格体（静态模型，如石头、木头）
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UStaticMesh> DropMesh;
	
	// 手持时的显示数据（静态或骨骼网格 + 偏移 + 动画类）
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FEquipDisplayData EquipDisplayData;

	// 放置到世界（如果Category是Block）——不是网格体，而是Actor类（因为方块可能有交互逻辑）
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(EditConditionExpression="ItemTags.HasTag(ItemCategory.Block)"))
	TSubclassOf<AActor> PlacedActorClass;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tags")
	FGameplayTagContainer ItemTags;
```

## 武器

1.FBoxCollisionInfo 用于存Boxtrace在不同武器上的偏移大小信息
2.FWeaponAttributes 武器的属性：锐利度(降低目标防御，最终伤害与目标防御和来源力量有关)，质量(攻击造成的僵直)
3.UMGWeaponItemDefinition 

```cpp
class MUNDUSGRANUM_API UMGWeaponItemDefinition : public UMGItemDefinition
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly)
	FBoxCollisionInfo CollisionTransform;
	
	UPROPERTY(EditDefaultsOnly)
	FWeaponAttributes WeaponAttributes;
	
    //武器所带的Ability,Effect,等信息
	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<UMGAbilitySet> AbilitySet;
};
```

**武器可以通过输入标签在所带的AbilitySet中找到对应的Ability，再由输入标签和武器标签找到对应的Montage**
***TODO*** *一个映射资产：谁来持有*

### Combat
**角色力量决定能持多少质量的武器，质量(惯性)决定Target和Source的僵直(前后摇)，动作幅度越大僵直越长造成伤害越高**
**近战武器的攻击有单手持和双手持，当双手都持有物品时意味着只能用单手攻击，单手攻击的动作幅度往往越小，双手攻击的动作幅度往往越大。双持不同武器时可左右手交替单手攻击，可以有组合攻击？或者在双手攻击的部分单纯混合手臂动作？**
**播放Montage会给角色添加僵直状态标签，在所有Montage通知结束之前都属于僵直状态，僵直状态阻止角色所有动作。攻击命中时先移除Target僵直状态标签再让Target(仅能)播放受击Montage**
**检测方式：Sphere Trace和Line Trace**

***TODO*** *GameplayAbility_MeleeAttack，角色僵直状态标签*