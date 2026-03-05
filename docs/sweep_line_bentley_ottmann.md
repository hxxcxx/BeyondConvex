# Sweep Line 线段求交算法（Bentley--Ottmann）

## 1. 算法简介

Sweep Line（扫描线）算法是一种经典的计算几何算法，用于在
**平面中查找所有线段的交点**。

最著名的实现是 **Bentley--Ottmann Algorithm**。

时间复杂度：

O((n + k) log n)

其中：

-   n：线段数量
-   k：交点数量

相比朴素算法：

O(n²)

扫描线算法在大规模数据下效率更高。

------------------------------------------------------------------------

# 2. 核心思想

假设有一条 **垂直扫描线 L** 从左向右移动。

在扫描过程中维护两个结构：

1.  **事件队列（Event Queue）**
2.  **扫描线状态结构（Status Structure）**

当扫描线遇到 **端点或交点** 时执行相应操作。

------------------------------------------------------------------------

# 3. 数据结构

## 3.1 Event Queue

按 **x 坐标排序**的优先队列。

事件类型：

  事件类型         说明
  ---------------- ----------------
  Left Endpoint    线段左端点
  Right Endpoint   线段右端点
  Intersection     两条线段的交点

排序规则：

1.  x 小优先
2.  若 x 相同，y 小优先

------------------------------------------------------------------------

## 3.2 Status Structure

表示 **当前扫描线穿过的线段集合**。

按扫描线位置的 **y 顺序**排序。

常见实现：

-   Balanced Binary Search Tree
-   std::set（红黑树）
-   AVL Tree

支持操作：

insert(e)\
delete(e)\
pred(e) // 前驱\
succ(e) // 后继\
swap(e,f)

------------------------------------------------------------------------

# 4. 事件处理

扫描线遇到三种情况。

------------------------------------------------------------------------

# Case A：扫描到左端点

L touches the left endpoint of segment e

步骤：

1.  插入线段

status.insert(e)

2.  检测相邻线段交点

pred(e)\
succ(e)

检测：

intersection(e, pred(e))\
intersection(e, succ(e))

若存在交点：

加入 Event Queue

------------------------------------------------------------------------

# Case B：扫描到右端点

L touches the right endpoint of segment e

步骤：

1.  找邻居

p = pred(e)\
s = succ(e)

2.  删除线段

status.delete(e)

3.  检测新邻居

intersection(p, s)

若存在交点：

加入 Event Queue

------------------------------------------------------------------------

# Case C：扫描到交点

L encounters intersection point of e and f

步骤：

1.  输出交点

record intersection

2.  交换顺序

swap(e, f)

3.  检测新的邻居

intersection(pred(e), e)\
intersection(f, succ(f))

若存在：

加入 Event Queue

------------------------------------------------------------------------

# 5. 算法流程

    SweepLineIntersection(segments):

        eventQueue ← 所有端点
        status ← 空

        while eventQueue not empty:

            event ← pop(eventQueue)

            if event 是 left endpoint:
                insert segment into status
                check neighbor intersections

            if event 是 right endpoint:
                remove segment
                check neighbor intersections

            if event 是 intersection:
                output intersection
                swap segments
                check new neighbors

------------------------------------------------------------------------

# 6. 为什么只检测相邻线段

几何性质：

扫描线状态结构中 **只有相邻线段可能首先相交**。

如果两条线段在 status 中
**不相邻**，说明中间存在其它线段，因此它们不会首先发生交点。

这就是算法能够达到：

O((n + k) log n)

复杂度的关键原因。

------------------------------------------------------------------------

# 7. 工程实现注意点

## 浮点误差

需要使用 epsilon 比较避免精度问题。

## 退化情况

需要处理：

-   共线
-   端点重合
-   重叠线段
-   垂直线段

## comparator 依赖 sweep line

如果使用 std::set：

排序依赖当前 sweep line 的 x。

常见解决方案：

-   使用全局 sweep_x
-   自定义平衡树
-   kinetic data structure

------------------------------------------------------------------------

# 8. 总结

Sweep Line（Bentley--Ottmann）算法核心：

扫描线 + 事件驱动 + 邻居检测

复杂度：

O((n + k) log n)
