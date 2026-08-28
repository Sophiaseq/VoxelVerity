// Copyright MundusGranum. All Rights Reserved.

#include "Voxel/SignedDistanceField.h"

namespace SdfInternal
{
	/** 站点间的“无穷”平方距离：够大，且与 q² 相加不会溢出 float。 */
	constexpr float InfSq = 1.0e12f;

	/**
	 * 一维平方距离变换（Felzenszwalb–Huttenlocher 抛物线下包络）。
	 * 输入/输出：Line[i] 为初始平方距离（0 = 站点，InfSq = 非站点）；就地改写为
	 * d[i] = min_j ( Line[j] + (i - j)² )。
	 * Mat 非空时同步传播 argmin 站点的材质（用于最近实心体素材质场）。
	 * 注意：读输入用 Src 副本——FH 变换评估阶段必须读原始 f[v[k]]，就地会读到已覆盖值。
	 */
	void DistanceTransform1D(TArray<float>& Line, TArray<FMaterialId>* Mat, int32 N)
	{
		const TArray<float> Src = Line;
		const TArray<FMaterialId> SrcMat = Mat ? *Mat : TArray<FMaterialId>();

		TArray<int32> V; // V[k] = 下包络中第 k 条抛物线的站点下标
		TArray<float> Z; // Z[k] = 第 k 条抛物线区间的左边界
		V.SetNum(N);
		Z.SetNum(N + 1);

		int32 K = 0;
		V[0] = 0;
		Z[0] = -TNumericLimits<float>::Max();
		Z[1] = TNumericLimits<float>::Max();

		for (int32 Q = 1; Q < N; ++Q)
		{
			const float FQ = Src[Q] + float(Q) * float(Q);

			float S = 0.0f;
			for (;;)
			{
				const float FV = Src[V[K]] + float(V[K]) * float(V[K]);
				S = (FQ - FV) / (2.0f * float(Q - V[K]));
				if (S <= Z[K])
				{
					--K; // 上一条抛物线被完全覆盖，弹出
				}
				else
				{
					break;
				}
			}

			++K;
			V[K] = Q;
			Z[K] = S;
			Z[K + 1] = TNumericLimits<float>::Max();
		}

		K = 0;
		for (int32 Q = 0; Q < N; ++Q)
		{
			while (Z[K + 1] < float(Q))
			{
				++K;
			}
			const float D = float(Q - V[K]);
			Line[Q] = D * D + Src[V[K]];
			if (Mat)
			{
				(*Mat)[Q] = SrcMat[V[K]];
			}
		}
	}

	/**
	 * 三维可分平方距离变换：沿 X/Y/Z 各做一次一维 DT。
	 * Mat 非空时同步传播最近站点的材质 ID。
	 */
	void SeparableEDT(TArray<float>& Distance, TArray<FMaterialId>* Mat, const FIntVector& Size)
	{
		const int32 SX = Size.X, SY = Size.Y, SZ = Size.Z;
		auto Index = [SX, SY](int32 X, int32 Y, int32 Z)
		{
			return X + SX * (Y + SY * Z);
		};

		const int32 MaxDim = FMath::Max3(SX, SY, SZ);
		TArray<float> Line;
		TArray<FMaterialId> MatLine;
		Line.SetNum(MaxDim);
		if (Mat) MatLine.SetNum(MaxDim);

		// X 轴（连续，步长 1）。
		for (int32 Z = 0; Z < SZ; ++Z)
		for (int32 Y = 0; Y < SY; ++Y)
		{
			const int32 Base = Index(0, Y, Z);
			for (int32 X = 0; X < SX; ++X)
			{
				Line[X] = Distance[Base + X];
				if (Mat) MatLine[X] = (*Mat)[Base + X];
			}
			DistanceTransform1D(Line, Mat ? &MatLine : nullptr, SX);
			for (int32 X = 0; X < SX; ++X)
			{
				Distance[Base + X] = Line[X];
				if (Mat) (*Mat)[Base + X] = MatLine[X];
			}
		}

		// Y 轴（步长 SX）。
		for (int32 Z = 0; Z < SZ; ++Z)
		for (int32 X = 0; X < SX; ++X)
		{
			const int32 Base = Index(X, 0, Z);
			for (int32 Y = 0; Y < SY; ++Y)
			{
				Line[Y] = Distance[Base + SX * Y];
				if (Mat) MatLine[Y] = (*Mat)[Base + SX * Y];
			}
			DistanceTransform1D(Line, Mat ? &MatLine : nullptr, SY);
			for (int32 Y = 0; Y < SY; ++Y)
			{
				Distance[Base + SX * Y] = Line[Y];
				if (Mat) (*Mat)[Base + SX * Y] = MatLine[Y];
			}
		}

		// Z 轴（步长 SX*SY）。
		for (int32 Y = 0; Y < SY; ++Y)
		for (int32 X = 0; X < SX; ++X)
		{
			const int32 Base = Index(X, Y, 0);
			for (int32 Z = 0; Z < SZ; ++Z)
			{
				Line[Z] = Distance[Base + SX * SY * Z];
				if (Mat) MatLine[Z] = (*Mat)[Base + SX * SY * Z];
			}
			DistanceTransform1D(Line, Mat ? &MatLine : nullptr, SZ);
			for (int32 Z = 0; Z < SZ; ++Z)
			{
				Distance[Base + SX * SY * Z] = Line[Z];
				if (Mat) (*Mat)[Base + SX * SY * Z] = MatLine[Z];
			}
		}
	}
	/**
	 * 沿某一轴做 [1,2,1]/4 模糊（边界 clamp），Src → Dst。
	 */
	void BlurAlongAxis(const TArray<float>& Src, TArray<float>& Dst, const FIntVector& Size, int32 Axis)
	{
		const int32 SX = Size.X, SY = Size.Y, SZ = Size.Z;
		auto Idx = [SX, SY](int32 X, int32 Y, int32 Z) { return X + SX * (Y + SY * Z); };

		for (int32 Z = 0; Z < SZ; ++Z)
		for (int32 Y = 0; Y < SY; ++Y)
		for (int32 X = 0; X < SX; ++X)
		{
			int32 XM = X, XP = X, YM = Y, YP = Y, ZM = Z, ZP = Z;
			if (Axis == 0) { XM = FMath::Max(0, X - 1); XP = FMath::Min(SX - 1, X + 1); }
			else if (Axis == 1) { YM = FMath::Max(0, Y - 1); YP = FMath::Min(SY - 1, Y + 1); }
			else { ZM = FMath::Max(0, Z - 1); ZP = FMath::Min(SZ - 1, Z + 1); }

			const int32 I = Idx(X, Y, Z);
			Dst[I] = (Src[Idx(XM, YM, ZM)] + 2.0f * Src[I] + Src[Idx(XP, YP, ZP)]) * 0.25f;
		}
	}

	/**
	 * 平滑圆角场：沿 X/Y/Z 各做一次可分 [1,2,1]/4 模糊，消除材质边界处的尖锐接缝
	 * （难度 #8「材质间过渡」的最小处理）。常数区域不受影响，仅材质接口被铺开。
	 */
	void SmoothRoundnessField(TArray<float>& R, const FIntVector& Size, int32 Iterations)
	{
		TArray<float> Tmp;
		Tmp.SetNumUninitialized(R.Num());
		for (int32 It = 0; It < Iterations; ++It)
		{
			BlurAlongAxis(R, Tmp, Size, 0);
			BlurAlongAxis(Tmp, R, Size, 1);
			BlurAlongAxis(R, Tmp, Size, 2);
			Swap(R, Tmp);
		}
	}

} // namespace SdfInternal

FSignedDistanceField BuildSignedDistanceField(const FVoxelGrid& Grid, const FVoxelMaterialTable& Materials)
{
	FSignedDistanceField Field;
	Field.Size = Grid.Size;

	const int32 SX = Grid.Size.X;
	const int32 SY = Grid.Size.Y;
	const int32 SZ = Grid.Size.Z;
	if (SX < 2 || SY < 2 || SZ < 2)
	{
		return Field; // 太小，无法形成表面
	}

	const int32 CellCount = SX * SY * SZ;
	auto Index = [SX, SY](int32 X, int32 Y, int32 Z)
	{
		return X + SX * (Y + SY * Z);
	};

	Field.Distance.SetNum(CellCount);
	Field.Roundness.SetNum(CellCount);

	// 1) 最近实心体素材质：站点 = 所有实心体素（距离 0，材质 = 体素材质）。
	{
		TArray<FMaterialId> Mat;
		Mat.SetNum(CellCount);

		for (int32 Z = 0; Z < SZ; ++Z)
		for (int32 Y = 0; Y < SY; ++Y)
		for (int32 X = 0; X < SX; ++X)
		{
			const FIntVector P(X, Y, Z);
			const int32 I = Index(X, Y, Z);
			if (Grid.IsSolid(P))
			{
				Field.Distance[I] = 0.0f;
				Mat[I] = Grid.Get(P);
			}
			else
			{
				Field.Distance[I] = SdfInternal::InfSq;
				Mat[I] = 0;
			}
		}

		SdfInternal::SeparableEDT(Field.Distance, &Mat, Grid.Size);

		for (int32 I = 0; I < CellCount; ++I)
		{
			Field.Roundness[I] = Materials.IsValidIndex(Mat[I]) ? Materials[Mat[I]].Roundness : 0.0f;
		}

		// 平滑圆角场：消除材质边界处的尖锐接缝（难度 #8）。
		SdfInternal::SmoothRoundnessField(Field.Roundness, Grid.Size, /*Iterations=*/1);
	}

	// 2) 几何 SDF：站点 = 边界体素（实心且有界内空邻接，越界不视为空——chunk 是世界的局部窗口）。
	{
		bool bAnySite = false;
		for (int32 Z = 0; Z < SZ; ++Z)
		for (int32 Y = 0; Y < SY; ++Y)
		for (int32 X = 0; X < SX; ++X)
		{
			const FIntVector P(X, Y, Z);
			const int32 I = Index(X, Y, Z);
			if (!Grid.IsSolid(P))
			{
				Field.Distance[I] = SdfInternal::InfSq;
				continue;
			}

			const FIntVector Neighbors[6] = {
				FIntVector(X - 1, Y, Z), FIntVector(X + 1, Y, Z),
				FIntVector(X, Y - 1, Z), FIntVector(X, Y + 1, Z),
				FIntVector(X, Y, Z - 1), FIntVector(X, Y, Z + 1),
			};
			bool bBoundary = false;
			for (const FIntVector& N : Neighbors)
			{
				if (Grid.InBounds(N) && !Grid.IsSolid(N))
				{
					bBoundary = true;
					break;
				}
			}

			Field.Distance[I] = bBoundary ? 0.0f : SdfInternal::InfSq;
			bAnySite |= bBoundary;
		}

		if (!bAnySite)
		{
			Field.Distance.Empty();
			Field.Roundness.Empty();
			Field.Size = FIntVector::ZeroValue;
			return Field; // 全空或全实
		}

		SdfInternal::SeparableEDT(Field.Distance, nullptr, Grid.Size);
	}

	// 转为有符号距离。设 d = 到最近边界体素中心的距离：表面在实心一侧 d+0.5、空气一侧 d-0.5，
	// 故实心 = -(d + 0.5)（内部为负）、空气 = d - 0.5（外部为正）。
	for (int32 Z = 0; Z < SZ; ++Z)
	for (int32 Y = 0; Y < SY; ++Y)
	for (int32 X = 0; X < SX; ++X)
	{
		const FIntVector P(X, Y, Z);
		const int32 I = Index(X, Y, Z);
		const float D = FMath::Sqrt(FMath::Max(0.0f, Field.Distance[I]));
		Field.Distance[I] = Grid.IsSolid(P) ? -(D + 0.5f) : (D - 0.5f);
	}

	return Field;
}

float FSignedDistanceField::SampleArray(const TArray<float>& Array, const FVector& P) const
{
	// 体素中心格坐标 = P - 0.5（体素 i 的中心在 P = i + 0.5，对应格下标 i）。
	const FVector C = P - FVector(0.5f, 0.5f, 0.5f);
	const FIntVector Base(
		FMath::FloorToInt(C.X),
		FMath::FloorToInt(C.Y),
		FMath::FloorToInt(C.Z));
	const FVector Frac = C - FVector(float(Base.X), float(Base.Y), float(Base.Z));

	auto D = [this, &Array](const FIntVector& I) -> float
	{
		const FIntVector K(
			FMath::Clamp(I.X, 0, Size.X - 1),
			FMath::Clamp(I.Y, 0, Size.Y - 1),
			FMath::Clamp(I.Z, 0, Size.Z - 1));
		return Array[IndexOf(K)];
	};

	const float d000 = D(Base);
	const float d100 = D(Base + FIntVector(1, 0, 0));
	const float d010 = D(Base + FIntVector(0, 1, 0));
	const float d110 = D(Base + FIntVector(1, 1, 0));
	const float d001 = D(Base + FIntVector(0, 0, 1));
	const float d101 = D(Base + FIntVector(1, 0, 1));
	const float d011 = D(Base + FIntVector(0, 1, 1));
	const float d111 = D(Base + FIntVector(1, 1, 1));

	const float X = Frac.X, Y = Frac.Y, Z = Frac.Z;
	const float d00 = FMath::Lerp(d000, d100, X);
	const float d10 = FMath::Lerp(d010, d110, X);
	const float d01 = FMath::Lerp(d001, d101, X);
	const float d11 = FMath::Lerp(d011, d111, X);
	const float d0 = FMath::Lerp(d00, d10, Y);
	const float d1 = FMath::Lerp(d01, d11, Y);
	return FMath::Lerp(d0, d1, Z);
}

float FSignedDistanceField::Sample(const FVector& P) const
{
	if (!IsValid())
	{
		return 1.0f; // 无表面 → 全空
	}
	return SampleArray(Distance, P);
}

float FSignedDistanceField::SampleRoundness(const FVector& P) const
{
	if (!IsValid())
	{
		return 0.0f;
	}
	return SampleArray(Roundness, P);
}
