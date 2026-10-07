// Copyright MundusGranum. All Rights Reserved.

#include "Voxel/SurfaceReconstructor.h"
#include "Voxel/SignedDistanceField.h"

namespace VoxelInternal
{
	FVector ToVec(const FIntVector& V)
	{
		return FVector(float(V.X), float(V.Y), float(V.Z));
	}

	// 单元格的 8 个角（相对单元格原点的体素偏移，单位因子）。
	const FIntVector GCorners[8] = {
		FIntVector(0,0,0), FIntVector(1,0,0), FIntVector(1,1,0), FIntVector(0,1,0),
		FIntVector(0,0,1), FIntVector(1,0,1), FIntVector(1,1,1), FIntVector(0,1,1),
	};

	// 12 条棱，每条由两个角的下标定义。
	const int32 GEdges[12][2] = {
		{0,1},{1,2},{2,3},{3,0},   // 底面
		{4,5},{5,6},{6,7},{7,4},   // 顶面
		{0,4},{1,5},{2,6},{3,7},   // 竖直
	};

	/** 一条棱：由两个角坐标确定（用于边哈希）。 */
	struct FEdgeKey
	{
		FIntVector A;
		FIntVector B;

		bool operator==(const FEdgeKey& Other) const
		{
			return A == Other.A && B == Other.B;
		}
	};

	uint32 GetTypeHash(const FEdgeKey& K)
	{
		return HashCombine(GetTypeHash(K.A), GetTypeHash(K.B));
	}
} // namespace VoxelInternal

using namespace VoxelInternal;

FReconstructedMesh ReconstructSurface(const FVoxelGrid& Grid, const FVoxelMaterialTable& Materials, const FCellPredicate& KeepCell, const FTransitionSpec& Transition)
{
	FReconstructedMesh Mesh;

	const FSignedDistanceField Sdf = BuildSignedDistanceField(Grid, Materials);
	if (!Sdf.IsValid())
	{
		return Mesh; // 全空或全实
	}

	// 组合密度场：几何 SDF 减去 per-material 圆角，等值面在 0。
	auto Density = [&Sdf](const FIntVector& C) -> float
	{
		// 单元格角是体素下标，surface nets 采样在体素中心（C + 0.5）。
		// 若直接 Sample(整数角)，会在 halo 边界处做跨块插值（越界 clamp），
		// 使相邻 chunk 对同一 corner 用不同体素集合 → 密度不一致 → 接缝裂缝。
		const FVector P = ToVec(C) + FVector(0.5f, 0.5f, 0.5f);
		// +epsilon 避免密度恰为 0（如 Roundness=0.5 时 SDF-Roundness 在空气体素中心恰好 0），
		// 否则等值面恰好穿过体素中心、边发射产生洞。
		return Sdf.Sample(P) - Sdf.SampleRoundness(P) + 1e-4f;
	};

	auto DensityAt = [&Sdf](const FVector& P) -> float
	{
		// 与 Density 保持一致：采样体素中心（P + 0.5），否则梯度/法线/绕序会偏移 0.5。
		const FVector Q = P + FVector(0.5f, 0.5f, 0.5f);
		return Sdf.Sample(Q) - Sdf.SampleRoundness(Q) + 1e-4f;
	};

	auto DensityGradient = [&DensityAt](const FVector& P) -> FVector
	{
		const float H = 0.5f;
		return FVector(
			DensityAt(P + FVector(H, 0, 0)) - DensityAt(P - FVector(H, 0, 0)),
			DensityAt(P + FVector(0, H, 0)) - DensityAt(P - FVector(0, H, 0)),
			DensityAt(P + FVector(0, 0, H)) - DensityAt(P - FVector(0, 0, H)));
	};

	// 单元格数量（细网格）。
	const int32 CW = Grid.Size.X - 1;
	const int32 CH = Grid.Size.Y - 1;
	const int32 CD = Grid.Size.Z - 1;

	// 某列是否面向更粗的邻居（该列的单元格在面内两轴加宽）。
	auto CoarseXCol = [&Transition, CW](int32 X) { return (Transition.CoarseMinus[0] && X == 1) || (Transition.CoarsePlus[0] && X == CW - 1); };
	auto CoarseYCol = [&Transition, CH](int32 Y) { return (Transition.CoarseMinus[1] && Y == 1) || (Transition.CoarsePlus[1] && Y == CH - 1); };
	auto CoarseZCol = [&Transition, CD](int32 Z) { return (Transition.CoarseMinus[2] && Z == 1) || (Transition.CoarsePlus[2] && Z == CD - 1); };

	// 单元格 per-axis size（sizeX 只依赖 Y/Z 列，sizeY 只依赖 X/Z 列，sizeZ 只依赖 X/Y 列）。
	auto CellSize = [&](int32 X, int32 Y, int32 Z) -> FIntVector
	{
		FIntVector S(1, 1, 1);
		if (CoarseYCol(Y) || CoarseZCol(Z)) S.X = 2;
		if (CoarseXCol(X) || CoarseZCol(Z)) S.Y = 2;
		if (CoarseXCol(X) || CoarseYCol(Y)) S.Z = 2;
		return S;
	};

	// 是否为规范单元格（在加宽方向上取奇下标：kept 范围 [1,CS]，粗单元格从 voxel 1 起拼贴）。
	auto IsCanonical = [&](int32 X, int32 Y, int32 Z) -> bool
	{
		const FIntVector S = CellSize(X, Y, Z);
		return (S.X == 1 || X % 2 == 1) && (S.Y == 1 || Y % 2 == 1) && (S.Z == 1 || Z % 2 == 1);
	};

	// 角密度缓存。
	TMap<FIntVector, float> CornerDensity;
	auto CornDens = [&](const FIntVector& C) -> float
	{
		if (float* Found = CornerDensity.Find(C))
		{
			return *Found;
		}
		const float D = Density(C);
		CornerDensity.Add(C, D);
		return D;
	};

	// 生成所有单元格并放置顶点（Pass 1）。
	struct FCell
	{
		FIntVector Min;
		FIntVector Size;
		int32 Vertex = -1;
	};
	TArray<FCell> Cells;

	for (int32 cz = 0; cz < CD; ++cz)
	for (int32 cy = 0; cy < CH; ++cy)
	for (int32 cx = 0; cx < CW; ++cx)
	{
		if (!IsCanonical(cx, cy, cz))
		{
			continue;
		}
		if (KeepCell && !KeepCell(FIntVector(cx, cy, cz)))
		{
			continue;
		}

		const FIntVector Size = CellSize(cx, cy, cz);
		const FIntVector Min(cx, cy, cz);

		// 8 角密度。
		float D[8];
		bool bIn[8];
		int32 InCount = 0;
		for (int32 i = 0; i < 8; ++i)
		{
			const FIntVector Corner = Min + FIntVector(Size.X * GCorners[i].X, Size.Y * GCorners[i].Y, Size.Z * GCorners[i].Z);
			D[i] = CornDens(Corner);
			bIn[i] = D[i] < 0.0f;
			InCount += bIn[i] ? 1 : 0;
		}
		if (InCount == 0 || InCount == 8)
		{
			continue; // 全外或全内，无表面
		}

		// 顶点 = 各交叉棱上等值面交点的平均。
		FVector Sum = FVector::ZeroVector;
		int32 Count = 0;
		for (int32 e = 0; e < 12; ++e)
		{
			const int32 A = GEdges[e][0];
			const int32 B = GEdges[e][1];
			if (bIn[A] == bIn[B])
			{
				continue;
			}
			float t = (0.0f - D[A]) / (D[B] - D[A]);
			t = FMath::Clamp(t, 0.0f, 1.0f);
			const FVector CA = ToVec(Min + FIntVector(Size.X * GCorners[A].X, Size.Y * GCorners[A].Y, Size.Z * GCorners[A].Z));
			const FVector CB = ToVec(Min + FIntVector(Size.X * GCorners[B].X, Size.Y * GCorners[B].Y, Size.Z * GCorners[B].Z));
			Sum += FMath::Lerp(CA, CB, t);
			++Count;
		}

		FVector Pos = Count > 0 ? Sum / float(Count) : ToVec(Min);

		// 过渡单元格：把窄轴（面向粗邻居）的顶点坐标吸附到粗网格，使与粗块顶点焊接。
		if (Size != FIntVector(1, 1, 1))
		{
			auto AxisLen = [&](int32 Axis) { return Axis == 0 ? CW : (Axis == 1 ? CH : CD); };
			for (int32 A = 0; A < 3; ++A)
			{
				if (Size[A] != 1)
				{
					continue;
				}
				if (Transition.CoarsePlus[A] && Min[A] == AxisLen(A) - 1)
				{
					Pos[A] = float(Min[A]);          // 吸附到 -A 面（粗网格）
				}
				else if (Transition.CoarseMinus[A] && Min[A] == 1)
				{
					Pos[A] = float(Min[A] + 1);      // 吸附到 +A 面
				}
			}
		}

		// 顶点材质 = 第一个实心角的材质（用于着色）。
		FMaterialId VertexMat = 0;
		for (int32 i = 0; i < 8; ++i)
		{
			if (bIn[i])
			{
				const FIntVector Corner = Min + FIntVector(Size.X * GCorners[i].X, Size.Y * GCorners[i].Y, Size.Z * GCorners[i].Z);
				VertexMat = Grid.Get(Corner);
				if (VertexMat != 0)
				{
					break;
				}
			}
		}

		FCell Cell;
		Cell.Min = Min;
		Cell.Size = Size;
		Cell.Vertex = Mesh.Vertices.Num();
		Mesh.Vertices.Add(Pos);
		Mesh.MaterialIds.Add(VertexMat);
		Cells.Add(Cell);
	}

	// 边哈希：为每条交叉棱记录共享它的单元格（Pass 2）。
	TMap<FEdgeKey, TArray<int32>> EdgeCells;
	for (int32 ci = 0; ci < Cells.Num(); ++ci)
	{
		const FCell& Cell = Cells[ci];
		for (int32 e = 0; e < 12; ++e)
		{
			const int32 A = GEdges[e][0];
			const int32 B = GEdges[e][1];
			const FIntVector CA = Cell.Min + FIntVector(Cell.Size.X * GCorners[A].X, Cell.Size.Y * GCorners[A].Y, Cell.Size.Z * GCorners[A].Z);
			const FIntVector CB = Cell.Min + FIntVector(Cell.Size.X * GCorners[B].X, Cell.Size.Y * GCorners[B].Y, Cell.Size.Z * GCorners[B].Z);
			const float DA = CornDens(CA);
			const float DB = CornDens(CB);
			if ((DA < 0.0f) == (DB < 0.0f))
			{
				continue; // 非交叉棱
			}
			FEdgeKey Key;
			if (CA.X < CB.X || (CA.X == CB.X && (CA.Y < CB.Y || (CA.Y == CB.Y && CA.Z < CB.Z))))
			{
				Key.A = CA; Key.B = CB;
			}
			else
			{
				Key.A = CB; Key.B = CA;
			}
			EdgeCells.FindOrAdd(Key).Add(ci);
		}
	}

	// 过渡单元格：把粗单元格加入其内侧 2×2 面的「十字」细棱，形成 fan 三角形。
	// 这些细棱是细单元格的角棱（已在上面记录 2 个细格），但粗单元格不共享它们；
	// 这里补上粗格，使它们变成 3 格（三角形）。
	auto AddEdge = [&](const FIntVector& A, const FIntVector& B, int32 CellIdx)
	{
		const float DA = CornDens(A);
		const float DB = CornDens(B);
		if ((DA < 0.0f) == (DB < 0.0f))
		{
			return;
		}
		FEdgeKey Key;
		if (A.X < B.X || (A.X == B.X && (A.Y < B.Y || (A.Y == B.Y && A.Z < B.Z))))
		{
			Key.A = A; Key.B = B;
		}
		else
		{
			Key.A = B; Key.B = A;
		}
		EdgeCells.FindOrAdd(Key).Add(CellIdx);
	};

	auto AxisLen = [&](int32 Axis) { return Axis == 0 ? CW : (Axis == 1 ? CH : CD); };

	for (int32 ci = 0; ci < Cells.Num(); ++ci)
	{
		const FCell& Cell = Cells[ci];
		if (Cell.Size == FIntVector(1, 1, 1))
		{
			continue;
		}

		for (int32 A = 0; A < 3; ++A)
		{
			if (Cell.Size[A] != 1)
			{
				continue; // A 是窄轴（面向粗邻居）
			}
			const int32 B = (A + 1) % 3;
			const int32 C = (A + 2) % 3;
			if (Cell.Size[B] != 2 || Cell.Size[C] != 2)
			{
				continue; // 面内两轴须为宽
			}

			int32 FaceCoord = -1;
			if (Transition.CoarsePlus[A] && Cell.Min[A] == AxisLen(A) - 1)
			{
				FaceCoord = Cell.Min[A];       // 内侧 = -A 面
			}
			else if (Transition.CoarseMinus[A] && Cell.Min[A] == 1)
			{
				FaceCoord = Cell.Min[A] + 1;   // 内侧 = +A 面
			}
			if (FaceCoord < 0)
			{
				continue;
			}

			const int32 MB = Cell.Min[B];
			const int32 MC = Cell.Min[C];
			auto Pt = [&](int32 Va, int32 Vb, int32 Vc) -> FIntVector
			{
				int32 V[3]; V[A] = Va; V[B] = Vb; V[C] = Vc;
				return FIntVector(V[0], V[1], V[2]);
			};

			// 4 条十字细棱（单位）：中心连到 4 条边的中点。细格已各自记录这 4 条棱，
			// 这里补上宽格，使它们变成 3 格（三角形）。
			AddEdge(Pt(FaceCoord, MB + 1, MC), Pt(FaceCoord, MB + 1, MC + 1), ci);
			AddEdge(Pt(FaceCoord, MB + 1, MC + 1), Pt(FaceCoord, MB + 1, MC + 2), ci);
			AddEdge(Pt(FaceCoord, MB, MC + 1), Pt(FaceCoord, MB + 1, MC + 1), ci);
			AddEdge(Pt(FaceCoord, MB + 1, MC + 1), Pt(FaceCoord, MB + 2, MC + 1), ci);

			// 4 条外围 2 单位边拆成 8 条单位边：细格记录的是单位半边（各自 2 个细格），
			// 宽格记录的是长度-2 边（无人共享 → 被跳过）。把宽格补到每条单位半边，
			// 使它们也变成 3 格（三角形），否则过渡面外围漏面、接缝不 watertight。
			AddEdge(Pt(FaceCoord, MB, MC), Pt(FaceCoord, MB + 1, MC), ci);       // 底
			AddEdge(Pt(FaceCoord, MB + 1, MC), Pt(FaceCoord, MB + 2, MC), ci);
			AddEdge(Pt(FaceCoord, MB, MC + 2), Pt(FaceCoord, MB + 1, MC + 2), ci); // 顶
			AddEdge(Pt(FaceCoord, MB + 1, MC + 2), Pt(FaceCoord, MB + 2, MC + 2), ci);
			AddEdge(Pt(FaceCoord, MB, MC), Pt(FaceCoord, MB, MC + 1), ci);       // 左
			AddEdge(Pt(FaceCoord, MB, MC + 1), Pt(FaceCoord, MB, MC + 2), ci);
			AddEdge(Pt(FaceCoord, MB + 2, MC), Pt(FaceCoord, MB + 2, MC + 1), ci); // 右
			AddEdge(Pt(FaceCoord, MB + 2, MC + 1), Pt(FaceCoord, MB + 2, MC + 2), ci);
		}
	}

	// 为每条交叉棱发射多边形（4 格 = 四边形，3 格 = 过渡三角形）。
	for (const auto& Pair : EdgeCells)
	{
		const TArray<int32>& Idx = Pair.Value;
		const int32 N = Idx.Num();
		if (N < 3)
		{
			continue; // 网格边界棱，不发射
		}

		// 按绕棱角度排序。
		const FVector EdgeDir = ToVec(Pair.Key.B - Pair.Key.A);
		const FVector Mid = (ToVec(Pair.Key.A) + ToVec(Pair.Key.B)) * 0.5f;
		TArray<int32> Sorted = Idx;
		Sorted.Sort([&](int32 A, int32 B)
		{
			const FVector VA = Mesh.Vertices[Cells[A].Vertex] - Mid;
			const FVector VB = Mesh.Vertices[Cells[B].Vertex] - Mid;
			// 绕棱轴的方位角（任取垂直面两轴做 atan2）。
			FVector U = FVector::CrossProduct(EdgeDir, FVector(1, 0, 0));
			if (U.SizeSquared() < 1e-6f) U = FVector::CrossProduct(EdgeDir, FVector(0, 1, 0));
			const FVector V = FVector::CrossProduct(EdgeDir, U).GetSafeNormal();
			U = U.GetSafeNormal();
			return FMath::Atan2(FVector::DotProduct(VA, V), FVector::DotProduct(VA, U))
				 < FMath::Atan2(FVector::DotProduct(VB, V), FVector::DotProduct(VB, U));
		});

		// 法线朝外，据此统一绕序。
		const FVector Outward = DensityGradient(Mid).GetSafeNormal();
		FVector FaceN = FVector::ZeroVector;
		for (int32 i = 0; i < N; ++i)
		{
			const FVector& V0 = Mesh.Vertices[Cells[Sorted[i]].Vertex];
			const FVector& V1 = Mesh.Vertices[Cells[Sorted[(i + 1) % N]].Vertex];
			const FVector& V2 = Mesh.Vertices[Cells[Sorted[(i + 2) % N]].Vertex];
			FaceN += FVector::CrossProduct(V1 - V0, V2 - V0);
		}
		// UE 前置面为顺时针绕序（DirectX 式左手系），而 FVector::CrossProduct 是右手定则、
		// 对顺时针三角形给出的法线指向内（与 Outward 相反）。故 FaceN 与 Outward 同向时应翻转。
		const bool bFlip = FVector::DotProduct(FaceN, Outward) > 0.0f;

		for (int32 i = 0; i < N - 2; ++i)
		{
			int32 A = Sorted[0];
			int32 B = Sorted[i + 1];
			int32 C = Sorted[i + 2];
			if (bFlip)
			{
				Swap(B, C);
			}
			Mesh.Indices.Add(Cells[A].Vertex);
			Mesh.Indices.Add(Cells[B].Vertex);
			Mesh.Indices.Add(Cells[C].Vertex);
		}
	}

	// Pass 3：法线 = 顶点处的密度梯度（平滑着色，方向朝外）。
	Mesh.Normals.SetNum(Mesh.Vertices.Num());
	for (int32 i = 0; i < Mesh.Vertices.Num(); ++i)
	{
		Mesh.Normals[i] = DensityGradient(Mesh.Vertices[i]).GetSafeNormal();
	}

	return Mesh;
}
