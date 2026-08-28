// PickupActor.h
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MGItemDefinition.h"
#include "MGDroppedItemActor.generated.h"

class USphereComponent;

UCLASS(Blueprintable)
class MUNDUSGRANUM_API AMGDroppedItemActor : public AActor
{
	GENERATED_BODY()

public:
	AMGDroppedItemActor();

	// 初始化函数，由背包系统调用
	UFUNCTION(BlueprintCallable)
	void InitializeDroppedItemActor(UMGItemDefinition* InItemDef);
	
#if WITH_EDITOR
	UFUNCTION(CallInEditor, Category = "Editor Preview")
	void SaveTraceOffsetsToDataAsset() const;
	//调整BoxTrace开始结束位置时再启用
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	void UpdatePreviewFromDataAsset();
#endif

protected:
	
#if WITH_EDITORONLY_DATA
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Editor Preview")
	TObjectPtr<USceneComponent> BoxTraceStart;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Editor Preview")
	TObjectPtr<USceneComponent> BoxTraceEnd;
	
	UPROPERTY(EditDefaultsOnly, Category = "Editor Preview")
	bool bAutoUpdatePreview = false;
#endif
	
	UFUNCTION(BlueprintCallable)
	virtual void BeginPlay() override;
	
	UFUNCTION(BlueprintCallable)
	void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	UFUNCTION(BlueprintCallable)
	void SphereOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// 场景中的静态模型组件
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	// 碰撞触发器（用于玩家靠近拾取）
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USphereComponent> PickupSphere;

	// 当前指向的定义
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UMGItemDefinition> ItemDef;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)//ReplicatedUsing = OnRep_Count, 
	int32 Count = 1;

	/*UFUNCTION()
	void OnRep_ItemDef();

	// 重写复制规则
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;*/
public:
	FORCEINLINE [[nodiscard]] TObjectPtr<UMGItemDefinition> GetItemDef() const{return ItemDef;}
	FORCEINLINE void SetItemDef(const TObjectPtr<UMGItemDefinition>& Def){this->ItemDef = Def;}
	FORCEINLINE [[nodiscard]] int32 GetCount() const{return Count;}
	FORCEINLINE void SetCount(int32 Num){this->Count = Num;}
};