# 模块化初始化流程搭建指南（InitState + GameFrameworkComponentManager）

> 从零搭出 Lyra 式模块化初始化流程：每个组件（PawnExtension / Hero / Health…）在不互相引用、不依赖固定生成顺序的前提下，被一张状态机井然有序地初始化到位。
>
> 面向**懂 UE（会写 Actor / Component / GameplayTag / 子系统），但不熟悉这套框架**的读者。
>
> 两个贯穿全文的原则：
> 1. **每一步都有一个"为什么"**——它之所以这样设计，是为了解决某个具体问题；而且"为什么"要深到"这个前提本身为什么存在"。
> 2. **每一步都有明确的"搭建清单"**——要新建哪个类、写哪几个函数、完成什么功能。照着做，9 个阶段走完你就有一套能跑的初始化框架。

---

## 搭建地图（先看全局，再逐层搭）

| 阶段 | 要添加的类/资产 | 核心函数 | 完成后你得到什么 |
|---|---|---|---|
| ③ 四态 | 4 个 GameplayTag | （仅 tag 定义） | 一套统一的"初始化刻度" |
| ④ 机制 | `UMGGameInstance` | `Init` → `RegisterInitState` ×4 | 状态顺序被登记 |
| ⑤ 单 feature | `UMGPawnExtensionComponent` | `OnRegister` / `CanChangeInitState` / `CheckDefaultInitialization` / `BeginPlay` | 一个组件自己能沿四态推进 |
| ⑥ 多 feature | `UMGHeroComponent` | `BindOnActorInitStateChanged` / `OnActorInitStateChanged` | 两个组件用委托互相协调 |
| ⑦ 推动机制 | （无新增，理解） | （无） | 彻底搞懂"谁在推状态" |
| ⑧ 扩展事件 | GameFeatureAction | `SendGameFrameworkComponentExtensionEvent` | 松耦合的"时刻广播" |

---

## 阶段一：引擎原生启动链路

### 要解决的问题：先看清"现状"——引擎是怎么把世界跑起来的

引擎启动是一条**固定、线性、不可协商**的链：

```
进程引导 → 加载模块 → 创建 Engine → 创建 GameInstance
  → 加载地图/创建 World → 创建 GameMode/GameState
  → 创建 PlayerController/PlayerState → 生成 Pawn → Possess/绑输入
```

- 每个环节的**类**从 `DefaultEngine.ini` 读出来（`GameEngineClassName`、`GameInstanceClass`、`GlobalDefaultGameMode`…）。
- 每个环节都留了**可重写的虚函数**作为钩子：`UEngine::Init`、`UGameInstance::Init`、`AGameModeBase::InitGame`、`AGameModeBase::SpawnDefaultPawnAtTransform`、`APawn::PossessedBy`…

### 为什么这条链是"僵硬的"

引擎启动的唯一目标是"**尽快、确定地把一个可渲染的世界跑起来**"。为了确定性，它必须：
- 固定调用顺序（谁先谁后写死），否则每次启动行为不同。
- 每个环节**同步完成**（到点就调，不等待）。

代价就是：**引擎保证"生成顺序"，不保证"数据就绪顺序"。** 钩子只告诉你"到这一步了"，不告诉你"这一步时你依赖的东西是否已经就绪"。

> 这一阶段没有代码要写，只记住一句话，它直接引出阶段二：**引擎保证生成顺序，不保证数据就绪顺序。**

---

## 阶段二：问题——组件互相依赖，而引擎不给协调机制

### 前提是什么

一个真实角色身上的组件有依赖图：Hero 要绑输入，前提是 PlayerState、Controller、InputComponent、PawnExtension 都就绪；PawnExtension 要把 ASC 绑到 Pawn，前提是 PawnData、Controller 就绪、且所有 feature 都到 DataAvailable。

### 为什么这个前提会存在（三个根因）

**根因 1：数据来自多个来源，且各自到达的时刻不同。**

- `PawnData` → GameMode 在 `SpawnDefaultPawnAtTransform` 时塞进去（生成时）。
- `PlayerState` → Login 阶段就创建（生成 Pawn **之前**）。
- `Controller` → Possess 时才挂上（生成 Pawn **之后**）。
- `InputComponent` → Possess 过程中的 `PawnClientRestart` 才创建（更晚）。
- 客户端上，`PawnData`/`PlayerState` 靠**网络复制**到达（更晚、顺序不定）。
- `Experience` 是**异步**加载的（资产 + GameFeature）。

也就是说，"某个组件依赖的数据"散布在这条线性链的**不同时间点**，而且客户端和服务器、第一次进局和重生，到达顺序还不一样。

**根因 2：引擎的钩子是"到点让你跑代码"，不是"让你等别人"。**

引擎给的所有虚函数（`InitGame`、`PossessedBy`…）本质都是回调——"现在轮到你了，你可以做点事"。但引擎**没有"依赖"这个概念**：你不能声明"先别初始化我，等 PlayerState 的数据好了再来"。

**根因 3：后果——"依赖就绪了吗"的责任全压在组件自己身上，朴素做法扛不住。**

于是你只能自己硬扛：`BeginPlay` 里 `if (Data == nullptr) return;`、加定时器重试、或者让 A 直接调 B。当组件只有一两个、没有复制、没有重生时还能忍；一旦 5+ 组件交叉依赖、加上复制和重生，这套散落的 null 检查会崩。

### 解决方案的方向

**让每个组件声明"我是谁、我需要什么、我到哪一步了"，交给一个中立协调者统一调度。** 这就是 InitState 状态机 + GFCM。

> 本阶段无代码。记住结论：**问题 = 多源数据 + 到达时刻不同 + 引擎无依赖概念；解法 = 状态声明 + 中央协调。**

---

## 阶段三：核心概念——InitState 四态

### 要解决的问题：把"初始化进行到哪了"变成可比较、可依赖的统一刻度

如果只有"初始化好了/没好"两个状态，表达不了"数据来了但还没消费""数据已初始化但还没完全就绪"。

### 📋 本阶段搭建清单

| 要添加的类/资产 | 要写的函数 | 完成的功能 |
|---|---|---|
| 4 个 GameplayTag | 无（只写 tag 的声明 + 定义） | 有了统一的四态刻度，供后续所有门控引用 |

**Step 1 — 声明**（`MundusGranumGameplayTags.h`）：

```cpp
namespace MundusGranumGameplayTags
{
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(InitState_Spawned);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(InitState_DataAvailable);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(InitState_DataInitialized);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(InitState_GameplayReady);
}
```

**Step 2 — 定义**（`MundusGranumGameplayTags.cpp`）：

```cpp
UE_DEFINE_GAMEPLAY_TAG_COMMENT(InitState_Spawned,        "InitState.Spawned",        "1: 已生成，可被扩展");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(InitState_DataAvailable,  "InitState.DataAvailable",  "2: 所需数据已加载/复制");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(InitState_DataInitialized,"InitState.DataInitialized","3: 数据已初始化");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(InitState_GameplayReady,  "InitState.GameplayReady",  "4: 完全可游戏");
```

### 为什么是四个状态（不是两个）

- `Spawned → DataAvailable` 表达"**等数据**"（复制/加载/Possess）。
- `DataAvailable → DataInitialized` 表达"**消费数据做初始化**"（绑 ASC、绑输入）。
- `DataInitialized → GameplayReady` 表达"**收尾/放行**"。

把"数据到位"和"数据被用掉"分成两个门控点，是整套框架最精妙的地方——别人能等"你的数据"，也能等"你做完初始化"。

---

## 阶段四：机制——GFCM 协调者 + 接口 + 注册状态顺序

### 要解决的问题：谁来记住"每个 feature 到哪一步"，谁来通知依赖方

两个角色：

**① `UGameFrameworkComponentManager`（GFCM）** —— 引擎 ModularGameplay 插件自带的一个 `UGameInstanceSubsystem`，跨关卡存活。内部维护 `Actor → { FeatureName, CurrentState, Implementer }`，提供**登记 / 查询 / 通知**三类能力。

**② `IGameFrameworkInitStateInterface`** —— 每个 feature（组件/Actor）实现它，声明三件事：

| 方法 | 职责 | 类比 |
|---|---|---|
| `CanChangeInitState(Cur, Desired)` | **门控**：从 Cur 到 Desired 需要什么条件 | "我能过去吗？" |
| `HandleChangeInitState(Cur, Desired)` | **动作**：进入某状态时做什么 | "过去时做什么？" |
| `CheckDefaultInitialization()` | **推进**：试着沿状态链往前走 | "现在往前走吧" |

### 📋 本阶段搭建清单

| 要添加的类/资产 | 要写的函数 | 完成的功能 |
|---|---|---|
| `UMGGameInstance` | `Init` → `RegisterInitState` ×4 | **登记四态先后顺序**（整条链的地基） |

> ⚠️ **这一步最容易漏，漏了全盘皆输**（见阶段九·坑 2）。`RegisterInitState` 登记的是"四个状态谁先谁后"，跟阶段五的 `RegisterInitStateFeature`（登记"我是某 Actor 上的一个 feature"）是**两回事**。

**Step 1 — 建 GameInstance**（`System/MGGameInstance.h/.cpp`）：

```cpp
UCLASS(Config = Game)
class MUNDUSGRANUM_API UMGGameInstance : public UGameInstance
{
    GENERATED_BODY()
public:
    virtual void Init() override;
};
```

```cpp
void UMGGameInstance::Init()
{
    Super::Init();
    UGameFrameworkComponentManager* ComponentManager = GetSubsystem<UGameFrameworkComponentManager>(this);
    if (ensure(ComponentManager))
    {
        ComponentManager->RegisterInitState(MundusGranumGameplayTags::InitState_Spawned,        false, FGameplayTag());
        ComponentManager->RegisterInitState(MundusGranumGameplayTags::InitState_DataAvailable,   false, MundusGranumGameplayTags::InitState_Spawned);
        ComponentManager->RegisterInitState(MundusGranumGameplayTags::InitState_DataInitialized, false, MundusGranumGameplayTags::InitState_DataAvailable);
        ComponentManager->RegisterInitState(MundusGranumGameplayTags::InitState_GameplayReady,  false, MundusGranumGameplayTags::InitState_DataInitialized);
    }
}
```

**Step 2 — 挂到引擎**（`DefaultEngine.ini`）：

```ini
[/Script/EngineSettings.GameMapsSettings]
GameInstanceClass=/Script/MundusGranum.MGGameInstance
```

### 为什么这样设计

- **门控是纯函数**（只读当前数据、返回 bool），可反复调用、幂等、无副作用。
- **动作只在门控通过后执行一次**（绑 ASC、绑输入）。
- **推进是被动触发**的（阶段七会看到它被频繁戳）。

拆开之后，"什么时候能走"和"走的时候干什么"解耦，协调者才能安全地反复询问"你能过去了吗"而不担心副作用。而 `RegisterInitState` 之所以要在 GameInstance 里做，是因为**顺序是全局的、跨关卡的**，必须在任何 feature 开始推进之前就登记好。

---

## 阶段五：动手①——第一个 feature（单组件）

### 前置：组件是怎么被加到 Actor 上的（三种方式）

在讲"如何把组件接入状态机"之前，先搞清楚组件**本身是怎么被创建到 Actor 上的**——有三种方式：

| 方式 | 谁创建 | 特点 | Lyra 例子 |
|---|---|---|---|
| ① C++ `CreateDefaultSubobject` | 角色构造函数写死 | 永远存在，代码里可见 | `PawnExtensionComponent` / `HealthComponent` / `CameraComponent` |
| ② 蓝图手动加（`BlueprintSpawnableComponent`） | 角色蓝图里拖上去 | 不用改 C++，但每个蓝图要手动加 | `HeroComponent` |
| ③ GFCM + `GameFeatureAction_AddComponents`（数据驱动） | 运行时由 GFCM 动态加 | 配置在 GameFeatureData 资产里 | `EquipmentManagerComponent` / `QuickBarComponent` |

**为什么会有第三条路？**

Equipment 这类"可选玩法系统"如果写死在 Character 构造函数里，每张地图、每个模式都得背着它；而且"加什么组件"属于玩法内容，不该硬编码。所以 Lyra 把它做成**数据**：

```
Experience 加载 → 激活 GameFeature
  → UGameFeatureAction_AddComponents::OnGameFeatureActivating
     → GFCM->AddComponentRequest(ActorClass, ComponentClass)   // 登记"给某 Actor 加某组件"
  → 当"接收者"actor（AModularCharacter 等基类调了 AddGameFrameworkComponentReceiver）生成时
     → GFCM 匹配请求 → 把组件加到该 actor 上
```

> **本教程（阶段五~七）聚焦 ① 和 ②**：PawnExtension / Hero 天生就在 Pawn 上，靠 InitState 状态机协调"谁先初始化"。等你需要 Equipment 这类可插拔系统，再回来走 ③ 的路子。

---

### 要解决的问题：把一个组件接入状态机的最小步骤

### 📋 本阶段搭建清单

| 要添加的类/资产 | 要写的函数 | 完成的功能 |
|---|---|---|
| `UMGPawnExtensionComponent`（实现 `IGameFrameworkInitStateInterface`） | `OnRegister` / `BeginPlay` / `CanChangeInitState` / `CheckDefaultInitialization` | 一个组件自己能沿四态推进 |

**Step 1 — 登记自己是个 feature（`OnRegister`）**

```cpp
void UMGPawnExtensionComponent::OnRegister()
{
    Super::OnRegister();
    RegisterInitStateFeature();   // 向 GFCM 登记"我是这个 Pawn 上的一个 feature"
}
```

> **为什么在 `OnRegister` 而不是 `BeginPlay`**：要赶在别的 feature 查询/监听之前登记，否则"别人等我"会漏掉我。

**Step 2 — 声明门控（`CanChangeInitState`）**

```cpp
bool UMGPawnExtensionComponent::CanChangeInitState(UGameFrameworkComponentManager* Manager,
    FGameplayTag CurrentState, FGameplayTag DesiredState) const
{
    APawn* Pawn = GetPawn<APawn>();

    if (!CurrentState.IsValid() && DesiredState == InitState_Spawned)
        return Pawn != nullptr;

    if (CurrentState == InitState_Spawned && DesiredState == InitState_DataAvailable)
    {
        if (!PawnData) return false;                       // 等 PawnData
        if ((Pawn->HasAuthority() || Pawn->IsLocallyControlled())
            && !GetController<AController>()) return false; // 等 Possess
        return true;
    }

    if (CurrentState == InitState_DataAvailable && DesiredState == InitState_DataInitialized)
        return Manager->HaveAllFeaturesReachedInitState(Pawn, InitState_DataAvailable);

    if (CurrentState == InitState_DataInitialized && DesiredState == InitState_GameplayReady)
        return true;

    return false;
}
```

**Step 3 — 启动状态链（`BeginPlay`）+ 定义推进（`CheckDefaultInitialization`）**

```cpp
void UMGPawnExtensionComponent::BeginPlay()
{
    Super::BeginPlay();
    TryToChangeInitState(InitState_Spawned);   // 先进入 Spawned
    CheckDefaultInitialization();              // 试着沿链往前冲
}

void UMGPawnExtensionComponent::CheckDefaultInitialization()
{
    static const TArray<FGameplayTag> StateChain =
        { InitState_Spawned, InitState_DataAvailable, InitState_DataInitialized, InitState_GameplayReady };
    ContinueInitStateChain(StateChain);
}
```

### 为什么这样设计

门控里**只读不写**，所以 `CheckDefaultInitialization` 能安全地反复调用；每个 `return false` 都是一句"我还在等 X"，把隐性依赖变成了显性的、可读的条件。

---

## 阶段六：动手②——多 feature + 委托监听

### 要解决的问题：A 要等 B 到某个状态，但 A 不该每帧去问 B

现在有 `PawnExtension` 和 `Hero` 两个 feature，依赖关系：

```
Hero 的 DataAvailable→DataInitialized：需要 PawnExtension 已到 DataInitialized
PawnExtension 的 DataAvailable→DataInitialized：需要所有 feature（含 Hero）都到 DataAvailable
```

**核心设计：用委托让 B 状态变化时主动通知 A，而不是 A 轮询 B。**

### 📋 本阶段搭建清单

| 要添加的类/资产 | 要写的函数 | 完成的功能 |
|---|---|---|
| `UMGHeroComponent`（实现 `IGameFrameworkInitStateInterface`） | `BeginPlay`（绑定监听）/ `OnActorInitStateChanged` / `CanChangeInitState`（门控查依赖） | 两个组件用委托互相协调初始化 |

**Step 1 — A 声明"我要监听 B"（`Hero::BeginPlay`）**

```cpp
void UMGHeroComponent::BeginPlay()
{
    Super::BeginPlay();
    // 监听 PawnExtension 这个 feature 的状态变化
    BindOnActorInitStateChanged(UMGPawnExtensionComponent::NAME_ActorFeatureName,
                                FGameplayTag(), /*bCallIfReached=*/false);
    TryToChangeInitState(InitState_Spawned);
    CheckDefaultInitialization();
}
```

**Step 2 — A 收到通知后回应（`OnActorInitStateChanged`）**

```cpp
void UMGHeroComponent::OnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{
    if (Params.FeatureName == UMGPawnExtensionComponent::NAME_ActorFeatureName &&
        Params.FeatureState == InitState_DataInitialized)
    {
        CheckDefaultInitialization();   // PawnExtension 就绪了，我再试试往前冲
    }
}
```

**Step 3 — 门控里用"查询"判断依赖（`Hero::CanChangeInitState`）**

```cpp
else if (CurrentState == InitState_DataAvailable && DesiredState == InitState_DataInitialized)
{
    AMGPlayerState* MGPS = GetPlayerState<AMGPlayerState>();
    return MGPS && Manager->HasFeatureReachedInitState(Pawn,
        UMGPawnExtensionComponent::NAME_ActorFeatureName, InitState_DataInitialized);
}
```

### 为什么用"委托监听"而不是"轮询"

1. **事件驱动**：B 状态一变，通知立刻到达，A 不用每帧空转。
2. **解耦**：A 只需要 B 的 **feature 名（FName）** 和 **目标状态（GameplayTag）**，不需要拿 B 的指针。
3. **可回溯**：`bCallIfReached` 参数能处理"B 在我绑定之前就已经到了那个状态"的边界。

---

## 阶段七：谁在推动状态——拉/重试模型 + 委托的完整机制

### 要解决的问题：状态不会自己动，是谁在"推"——以及为什么是"拉"不是"推"

**结论：没有"推手"，是"拉/重试"模型。** 状态只在被"戳一下"时，重新评估门控，满足就自己往前走。

### 一次推进的内部流程

引擎 `TryToChangeInitState` 三步：

```cpp
if (!CanChangeInitState(Manager, Cur, Desired)) return false;    // ① 门控
HandleChangeInitState(Manager, Cur, Desired);                    // ② 动作
Manager->ChangeFeatureInitState(Actor, FeatureName, this, Desired); // ③ 落状态+广播
```

### 委托的完整通知链（重点）

第 ③ 步 `ChangeFeatureInitState` 是**唯一**的"落状态 + 广播"点：

```
ChangeFeatureInitState
  → FoundState->CurrentState = DesiredState      // 先落状态
  → ProcessFeatureStateChange
       → StateChangeQueue.Emplace(...)           // 进队列（非递归）
       → CallFeatureStateDelegates(...)          // 逐个调匹配的委托
            → 委托 = CreateWeakLambda(this, [this](Params){ OnActorInitStateChanged(Params); })
                 → OnActorInitStateChanged
                      → CheckDefaultInitialization   // 回到起点，级联
```

### 为什么每一处都要这样设计

| 设计点 | 为什么 | 解决的什么问题 |
|---|---|---|
| **先落状态再广播** | 先改 `CurrentState` 再通知监听者 | 监听者回调里读到的状态一定最新 |
| **`StateChangeQueue` 非递归** | 回调里触发新变更时只追加到队列，外层 while 统一处理 | 避免递归栈溢出；通知有序、不重入 |
| **`CreateWeakLambda` 弱引用** | 委托持弱引用，`this` 销毁后自动失效 | 避免悬空指针 |
| **拉/重试（不推）** | 门控是纯函数、`CheckDefaultInitialization` 幂等 | 谁先就绪都无所谓；反复戳都安全；没有"推错顺序" |

### 完整级联（一次 Possess）

```
Possess → PossessedBy → PawnExtension::HandleControllerChanged
  → CheckDefaultInitialization
      ├─ CheckDefaultInitializationForImplementers()   ← 戳醒所有其它 feature
      └─ ContinueInitStateChain(...)
           → 门控过 → HandleChangeInitState → ChangeFeatureInitState
              → 广播 → 监听者 OnActorInitStateChanged → CheckDefaultInitialization（级联）
```

> 记住：**生命周期事件（Possess/复制/输入/数据加载）是"火花"，`CheckDefaultInitialization` 是"重试环"，`ChangeFeatureInitState` 的广播是"引信"——三者串成确定性的初始化流水线。**

---

## 阶段八：扩展事件 + GameFeatures

### 要解决的问题：把"我的某个时刻到了"广播出去，让**不认识我的人**也能响应

前面是 **InitState 系统内部**的委托（监听 feature 状态）。还有一层更松的耦合：**扩展事件（FName）**，谁都能发、谁都能听。

| 事件 | 谁在何时发 | 谁在听 |
|---|---|---|
| `GameActorReady`（引擎内置） | 每个 Modular 基类在 `BeginPlay` | `AddWidget` 类 GF Action |
| `LyraAbilitiesReady` | `PlayerState::SetPawnData` 授完能力后 | `AddAbilities` 类 GF Action |
| `BindInputsNow` | `Hero::InitializePlayerInput` 末尾 | `AddInputContextMapping`/`AddInputBinding` 类 GF Action |

### 📋 本阶段搭建清单

| 要添加的类/资产 | 要写的函数 | 完成的功能 |
|---|---|---|
| GameFeatureAction（`AddInputBinding` 等） | 发送侧：在初始化末尾 `SendGameFrameworkComponentExtensionEvent`；监听侧：`OnGameFeatureActivating` 里 `AddExtensionHandler` | 松耦合的"时刻广播"，接上 GameFeatures |

**发送**（`Hero::InitializePlayerInput` 末尾）：

```cpp
UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(Pawn, NAME_BindInputsNow);
```

### 为什么这样设计

- **彻底解耦**：发事件的一方完全不知道谁在听。GameFeature 激活时才去订阅。
- **接上 GameFeatures**：一个 GameFeature 是"打包好的一坨内容（能力/输入/UI）"，激活时监听这些事件，在对的时刻注入内容——这就是"数据驱动、运行时装配玩法"的底座。

### 另一种 GF Action：`AddComponents`（加组件，不发事件）

前面讲的"扩展事件"是 GameFeatures 的**一种** GF Action 形态（发事件 + 监听）。GameFeatures 还有**另一种：`UGameFeatureAction_AddComponents`**——它就是阶段五"第三条路"的落地：**不写代码，靠数据给 Actor 动态加组件**。

**数据长什么样**（在 GameFeatureData 资产的 "Add Components" 列表里配，每条是一个 `FGameFeatureComponentEntry`）：

| 字段 | 含义 | 例子 |
|---|---|---|
| `ActorClass` | 加到哪个 actor 类 | `ALyraCharacter` |
| `ComponentClass` | 加哪个组件类 | `ULyraEquipmentManagerComponent` |
| `bClientComponent` | 客户端加不加 | true |
| `bServerComponent` | 服务器加不加 | true |
| `AdditionFlags` | 加组件规则（`AddUnique` 等） | None |

典型配置（语义示意）：

```
ActorClass = ALyraCharacter         → ComponentClass = ULyraEquipmentManagerComponent   （server + client）
ActorClass = ALyraPlayerController  → ComponentClass = ULyraQuickBarComponent          （server + client）
```

**激活时引擎做了什么**（`GameFeatureAction_AddComponents.cpp` 的 `AddToWorld`）：

```cpp
for (const FGameFeatureComponentEntry& Entry : ComponentList)
{
    const bool bShouldAddRequest = (bIsServer && Entry.bServerComponent) || (bIsClient && Entry.bClientComponent);
    if (bShouldAddRequest)
    {
        TSubclassOf<UActorComponent> ComponentClass = Entry.ComponentClass.LoadSynchronous();
        Handles.ComponentRequestHandles.Add(GFCM->AddComponentRequest(Entry.ActorClass, ComponentClass, AdditionFlags));
    }
}
```

就三件事：**遍历 `ComponentList` → 按服务器/客户端过滤 → 向 GFCM 登记 `AddComponentRequest`**。剩下"等 receiver actor 生成时把组件加上去"由 GFCM 完成（见阶段五"第三条路"）。

> 一句话区分两类 GF Action：**扩展事件**（`AddAbilities` / `AddInputContextMapping` / `AddWidget`）是"在对的时刻做一件事"；**`AddComponents`** 是"给某类 actor 挂上一个组件"。两者都配在 GameFeatureData 里，都靠 GFCM 落地。

> 记住：**InitState 委托管"组件间的初始化顺序"，扩展事件管"游戏功能间的装配时机"，两层不同粒度的解耦。**

---

## 阶段九：踩坑清单（我们真实踩过的）

### 坑 1：忘调 `RegisterInitStateFeature()`（缺 `OnRegister`）

- **现象**：PawnExtension 不在 GFCM 表里。**修**：`OnRegister` 里补 `RegisterInitStateFeature()`。

### 坑 2：忘调 `RegisterInitState()`（状态顺序没注册）⭐ 最隐蔽

- **现象**：Hero 能到 `DataAvailable`，但 `DataAvailable→DataInitialized` 永远过不去（`InitializePlayerInput` 不执行）。
- **根因**：`IsInitStateAfterOrEqual` 判断"谁先谁后"靠 `InitStateOrder`，这个顺序由 `GameInstance::Init` 里 `RegisterInitState` 填。**没注册 → 只有"状态相等"返回 true，跨状态比较一律 false。**
- **为什么这么隐蔽**：PawnExtension 一口气 `DataAvailable→DataInitialized→GameplayReady` 连跳，等 Hero 的委托收到"到 DataInitialized"的通知时，PawnExtension 其实已经是 `GameplayReady` 了。Hero 门控问"PawnExtension ≥ DataInitialized 吗"，`IsInitStateAfterOrEqual(GameplayReady, DataInitialized)` 因顺序表为空返回 false。
- **修**：见阶段四，在 `GameInstance::Init` 注册四次顺序。

### 坑 3：`GameInstanceClass` / `GlobalDefaultGameMode` 没在 `DefaultEngine.ini` 配

- **现象**：`GetPlayerState<AMGPlayerState>()` / `GetController<AMGPlayerController>()` 返回 null，Hero 卡在 `Spawned`。
- **根因**：GameMode 构造函数里设的 `PlayerControllerClass`/`PlayerStateClass`/`DefaultPawnClass`，**只有这个 GameMode 真被用上才生效**；没配 `GlobalDefaultGameMode` 时引擎用默认 `AGameModeBase`。

### 坑 4：`RegisterInitStateFeature` 和 `RegisterInitState` 搞混

- 前者"我是某 Actor 上的一个 feature"（阶段五），后者"四个状态谁先谁后"（阶段四）。**两个都要调。**

---

## 附：一页速查

| 概念 | 一句话 | 关键函数 |
|---|---|---|
| 四态 | 初始化进行到哪一步的统一刻度 | `InitState.*` 四个 GameplayTag |
| feature | 实现 `IGameFrameworkInitStateInterface` 的组件/Actor | `GetFeatureName()` |
| 门控 | "我能从 A 到 B 吗"（纯函数） | `CanChangeInitState` |
| 动作 | "到 B 时做什么"（副作用） | `HandleChangeInitState` |
| 推进 | "试着沿链往前冲"（幂等） | `CheckDefaultInitialization` → `ContinueInitStateChain` |
| 落状态+广播 | 唯一改状态 + 通知监听者的点 | `ChangeFeatureInitState` |
| 监听依赖 | A 监听 B 的状态变化 | `BindOnActorInitStateChanged` + `OnActorInitStateChanged` |
| 状态顺序 | "谁在前谁在后"的全局顺序 | `RegisterInitState`（GameInstance::Init） |
| 扩展事件 | 松耦合的"时刻广播" | `SendGameFrameworkComponentExtensionEvent` |

---

## 与其它资料的关系

- `LyraStarterGame/Docs/Lyra_Startup_GFCM.drawio` — InitState 协调的时序图 + 门控对照表 + 驱动闭环。
- `LyraStarterGame/Docs/Lyra_Hook_Points.md` — 引擎原生启动链路与 Lyra 三种挂钩方式（本文阶段一、二的上游）。
- 本文是"**怎么搭**"的落地教程，前两份是"**它是什么**"的架构分析。
