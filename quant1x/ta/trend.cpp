#include <quant1x/ta/trend.h>

namespace ta {
    // 计算两条直线的交点（如果存在）
    std::optional<point> line_intersection(
        const point &p1, const point &p2,
        const point &v1, const point &v2) {

        double x1 = p1.x, y1 = p1.y;
        double x2 = p2.x, y2 = p2.y;
        double x3 = v1.x, y3 = v1.y;
        double x4 = v2.x, y4 = v2.y;

        double dx1 = x2 - x1;
        double dy1 = y2 - y1;
        double dx2 = x4 - x3;
        double dy2 = y4 - y3;

        double denominator = dy2 * dx1 - dx2 * dy1;

        // 如果分母为0，说明两直线平行或重合
        if (std::abs(denominator) < 1e-8) {
            return std::nullopt;
        }

        // 计算 t 和 s
        double t_numerator = dx2 * (y1 - y3) - dy2 * (x1 - x3);
        double s_numerator = dx1 * (y1 - y3) - dy1 * (x1 - x3);

        double t = t_numerator / denominator;
        // double s = s_numerator / denominator; // s 可选，用于其他用途
        (void)s_numerator;

        // 交点坐标
        double x = x1 + t * dx1;
        double y = y1 + t * dy1;

        return point{x, y};
    }
} // namespace ta