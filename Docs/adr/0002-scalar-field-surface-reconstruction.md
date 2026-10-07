# 标量场提取曲面重建（而非几何网格平滑）

方块间的平滑曲面用标量场提取生成：把体素占据/材质当密度场，用每材质的核函数平滑后提等值面（surface nets / dual contouring 一族）。

选了它而非"先生成方块面再平滑顶点"，因为"相邻多个面合并成一个曲面"本质是拓扑合并，几何平滑无法稳定处理跨块合并与 LOD。具体算法（surface nets vs dual contouring vs marching cubes）留待实现阶段再定。
