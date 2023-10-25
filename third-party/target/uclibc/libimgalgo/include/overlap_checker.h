#ifndef __OVERLAP_CHECKER_H_
#define __OVERLAP_CHECKER_H_

namespace post_process_event {

class OverlapChecker {
public:
    struct Point {
        float x, y;
    };

    // quad和rect均需要保持为顺时针或逆时针点的顺序
    static bool checkOverlap(const Point quad[], const Point rect[]);
    // 性能测试接口
    static void generateAndTest(int numTests = 10000);

private:
    // 返回op→oq的方向
    // return: 0共线 1逆时针 -1顺时针
    static int orientation(const Point& p, const Point& o, const Point& q);
    // 检查点a是否在线段pq上
    static bool onSegment(const Point& a, const Point& p, const Point& q);
    /** 检查线段p1q1与p2q2是否相交
     * 交叉检测
     *      /|\
     *      \|/
    */
    static bool doIntersect(const Point& p1, const Point& q1, const Point& p2, const Point& q2);
    // 射线法 根据交点数量判断点是否在多边形内
    static bool isInside(const Point poly[], int n, const Point& p);
};

} // namespace post_process_event

#endif // __OVERLAP_CHECKER_H_
