#include "overlap_checker.h"

#include <iostream>
#include <cmath>
#include <chrono>
#include <random>

namespace post_process_event {

bool OverlapChecker::checkOverlap(const Point quad[], const Point rect[])
{
    // 检查所有边是否有交集
    for (int i = 0; i < 4; i++) {
        int next = (i + 1) % 4;
        for (int j = 0; j < 4; j++) {
            int nextj = (j + 1) % 4;
            if (doIntersect(quad[i], quad[next], rect[j], rect[nextj]))
                return true;
        }
    }
    // 检查是否有点在另一个四边形内，确定是否是包含关系
    for (int i = 0; i < 4; i++) {
        if (isInside(quad, 4, rect[i])) return true;
    }
    for (int i = 0; i < 4; i++) {
        if (isInside(rect, 4, quad[i])) return true;
    }
    return false;
}

void OverlapChecker::generateAndTest(int numTests /* = 10000*/)
{
    int overlapCount = 0;

    std::default_random_engine generator(std::chrono::system_clock::now().time_since_epoch().count());
    std::uniform_real_distribution<float> distribution(-1.0, 2.0);

    for (int test = 0; test < numTests; ++test) {
        Point quad[4];
        Point rect[4];

        for (int i = 0; i < 4; ++i) {
            quad[i].x = distribution(generator);
            quad[i].y = distribution(generator);

            rect[i].x = distribution(generator);
            rect[i].y = distribution(generator);
        }

        if (checkOverlap(quad, rect)) {
            overlapCount++;
        }
    }

    std::cout << "Out of " << numTests << " tests, " << overlapCount << " had an area overlap.\n";
}

int OverlapChecker::orientation(const Point& p, const Point& o, const Point& q)
{
    float val = (p.x - o.x) * (q.y - o.y) - (q.x - o.x) * (p.y - o.y);
    if (std::abs(val) < 1e-9) return 0;
    return (val > 0) ? 1 : -1;
}

bool OverlapChecker::onSegment(const Point& a, const Point& p, const Point& q)
{
    // 叉乘保证点在直线上
    return (orientation(p, a, q) == 0
        // 保证点在线段的矩形内
        && std::min(p.x, q.x) <= a.x && a.x <= std::max(p.x, q.x)
        && std::min(p.y, q.y) <= a.y && a.y <= std::max(p.y, q.y));
}

bool OverlapChecker::doIntersect(const Point& p1, const Point& q1, const Point& p2, const Point& q2)
{
    int o1 = orientation(p1, q1, p2);
    int o2 = orientation(p1, q1, q2);
    int o3 = orientation(p2, q2, p1);
    int o4 = orientation(p2, q2, q1);

    if (o1 != o2 && o3 != o4) return true;
    // 边界条件的判断
    if (o1 == 0 && onSegment(p2, p1, q1)) return true;
    if (o2 == 0 && onSegment(q2, p1, q1)) return true;
    if (o3 == 0 && onSegment(p1, p2, q2)) return true;
    if (o4 == 0 && onSegment(q1, p2, q2)) return true;

    return false;
}

bool OverlapChecker::isInside(const Point poly[], int n, const Point& p)
{
    if (n < 3) return false;
    const Point extreme = { 1e9, p.y }; // 定义远点
    int count = 0, i = 0;
    do {
        int next = (i + 1) % n;
        if (doIntersect(poly[i], poly[next], p, extreme)) {
            if (orientation(poly[i], p, poly[next]) == 0)
                return onSegment(poly[i], p, poly[next]);
            count++;
        }
        i = next;
    } while (i != 0);
    return count & 1;
}

} // namespace post_process_event
