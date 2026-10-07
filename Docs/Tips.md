# Tips — UE5 网络复制备忘

## For Mixed Replication Mode：Owner 相关要点

> 背景：GAS 的 `EGameplayEffectReplicationMode` 三种复制模式中，`Mixed` 专用于"多人 · 玩家控制"的角色。其最关键的坑就是 Owner 的绑定规则。

### 一句话结论

`Mixed` 模式只把 GameplayEffect 复制给 **owning client**，而 owning client 的判定完全依赖 **Owner 链**，因此 **OwnerActor 的 Owner 必须最终指向一个 Controller（PlayerController）**。否则 GE 会因"没有可送达的接收方"而静默丢失。

### 三种模式对比

| 模式 | 适用场景 | GameplayEffect | GameplayTag / Cue |
|---|---|---|---|
| `Full` | 单人 | 复制到所有客户端 | 复制到所有客户端 |
| `Mixed` | 多人 · 玩家控制 | **仅复制到 owning client** | 复制到所有客户端 |
| `Minimal` | 多人 · AI 控制 | 不复制 | 复制到所有客户端 |

### 为什么 Owner 必须是 Controller（原理）

- owning client 由 `AActor::GetNetConnection()` 决定：沿 Owner 链向上（`GetNetOwner()`）找到 Controller，再取 `Controller->GetNetConnection()` 得到该客户端对应的 `UNetConnection`。
- Owner 链走向：`Actor →(Owner)→ … → PlayerController → OwningConnection`。
- 若 OwnerActor 的 Owner 链尽头不是 Controller，owning connection 为空 → "只复制给 owning client" 变成无接收方 → GE 静默丢弃，且不报错。

### Owner 的自动 / 手动设置

| ASC 所在 Actor | Owner 是否自动正确 | 说明 |
|---|---|---|
| Pawn / Character | 自动 | `PossessedBy()` 自动把 Owner 设为 PlayerController |
| PlayerState | 自动 | PlayerState 的 Owner 自动为 Controller |
| 其他任意 Actor | **必须手动** | 需自行 `SetOwner(Controller)`，否则 Mixed 不生效 |

### 代码片段

```cpp
// 1) 非 Pawn / 非 PlayerState 的 OwnerActor：手动绑定 Controller
AbilityOwnerActor->SetOwner(GetWorld()->GetFirstPlayerController());

// 2) 设置 Mixed 复制模式
AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

// 3) possession 完成后再初始化（顺序必须保证）
AbilitySystemComponent->InitAbilityActorInfo(OwnerActor, AvatarActor);
```

### OwnerActor vs AvatarActor

- **OwnerActor**：ASC 所属、拥有网络所有权的 Actor（GE 复制目标据此判定）。
- **AvatarActor**：ASC 的视觉载体 / 实际控制的 Actor。
- AI 角色：两者都是 Character 本身。
- 玩家角色（ASC 放在 PlayerState）：OwnerActor = PlayerState，AvatarActor = Character。

## ActivateAbility全链路

外部触发 (Input / GameplayEvent / TryActivateAbility)
    │
    ▼
┌─────────────────────────────────────────────────────────┐
│ UAbilitySystemComponent::TryActivateAbility()           │  ← 不在本文件中，在 ASC 中
│   - 查找 AbilitySpec                                    │
│   - 处理 InstancingPolicy / ReplicationPolicy           │
│   - 调用 CanActivateAbility()                           │
│   - 通过后调用 CallActivateAbility()                    │
└─────────────────────────────────────────────────────────┘
    │
    ▼
┌─────────────────────────────────────────────────────────┐
│ UGameplayAbility::CanActivateAbility()  [虚函数]        │
│   - 检查 AvatarActor 是否有效                            │
│   - 检查 AbilitySystemComponent 是否有效                 │
│   - 检查 FindAbilitySpecFromHandle()                     │
│   - 检查 GetUserAbilityActivationInhibited()             │
│   - CheckCooldown()                                      │
│   - CheckCost()                                          │
│   - DoesAbilitySatisfyTagRequirements()                  │
│   - IsAbilityInputBlocked()                              │
│   - K2_CanActivateAbility()  [蓝图实现]                  │
└─────────────────────────────────────────────────────────┘
    │ 返回 true
    ▼
┌─────────────────────────────────────────────────────────┐
│ UGameplayAbility::CallActivateAbility()  [非虚]         │
│   ├── PreActivate()                                      │
│   └── ActivateAbility()                                  │
└─────────────────────────────────────────────────────────┘
    │
    ├──────────────────────────────────────────────────┐
    ▼                                                  ▼
┌──────────────────────────────────┐  ┌──────────────────────────────────┐
│ PreActivate()                    │  │ ActivateAbility()  [虚函数]      │
│   - FlushServerMoves()           │  │   - 子类重写                     │
│   - bIsActive = true             │  │   - 蓝图: K2_ActivateAbility()   │
│   - bIsBlockingOtherAbilities    │  │   - 蓝图事件:                     │
│   - bIsCancelable = true         │  │     K2_ActivateAbilityFromEvent()│
│   - SetCurrentInfo()             │  │                                  │
│   - CurrentEventData = ...       │  │   ┌──────────────────────────┐   │
│   - HandleChangeAbilityCan...    │  │   │ CommitAbility()          │   │
│   - AddLooseGameplayTags()       │  │   │   ├── CommitCheck()      │   │
│   - NotifyAbilityActivated()     │  │   │   │   ├── CheckCooldown │   │
│   - ApplyAbilityBlockAnd...      │  │   │   │   └── CheckCost     │   │
│   - Spec->ActiveCount++          │  │   │   ├── CommitExecute()    │   │
│                                  │  │   │   │   ├── ApplyCooldown │   │
│                                  │  │   │   │   └── ApplyCost     │   │
│                                  │  │   │   └── K2_CommitExecute() │   │
│                                  │  │   └──────────────────────────┘   │
│                                  │  │                                  │
│                                  │  │   ┌──────────────────────────┐   │
│                                  │  │   │ 创建 AbilityTask         │   │
│                                  │  │   │ (PlayMontage, WaitEvent) │   │
│                                  │  │   └──────────────────────────┘   │
│                                  │  │                                  │
│                                  │  │   ┌──────────────────────────┐   │
│                                  │  │   │ EndAbility()             │   │
│                                  │  │   └──────────────────────────┘   │
└──────────────────────────────────┘  └──────────────────────────────────┘