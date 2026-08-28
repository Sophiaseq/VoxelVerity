// Copyright MundusGranum. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AVoxelSpikeActor.generated.h"

class UProceduralMeshComponent;

/** 技术验证用：把一个体素球重建为平滑网格并显示出来。 */
UCLASS()
class MUNDUSGRANUM_API AVoxelSpikeActor : public AActor
{
	GENERATED_BODY()

public:
	AVoxelSpikeActor();

	UPROPERTY(EditAnywhere, Category = "Voxel")
	int32 GridSize = 32;

	UPROPERTY(EditAnywhere, Category = "Voxel")
	float SphereRadius = 6.0f;

	/** 材质 1（左球）圆角半径（体素单位）。0 = 锐利方块，越大棱角越圆。 */
	UPROPERTY(EditAnywhere, Category = "Voxel")
	float Roundness = 0.4f;

	/** 材质 2（右球）圆角半径，演示 per-material。 */
	UPROPERTY(EditAnywhere, Category = "Voxel")
	float Roundness2 = 0.9f;

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UProceduralMeshComponent> Mesh;
};
