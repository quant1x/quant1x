#pragma once
#ifndef QUANT1X_TECHNICAL_ANALYSIS_TREND_H
#define QUANT1X_TECHNICAL_ANALYSIS_TREND_H 1

#include <quant1x/std/api.h>
#include <cmath>    // for atan2, abs, M_PI
#include <numbers>

namespace ta {

    // 点
    struct point {
        f64 x;
        f64 y;
    public:
        std::string to_string() const {
            return fmt::format("{{x={},y={}}}", x, y);
        }
    };

    // 线
    struct line {
        double a, b, c;  // ax + by + c = 0

        // 构造函数: 通过一般式构造
        line(double a_, double b_, double c_) : a(a_), b(b_), c(c_) {}

        // 构造函数: 通过两个点构造直线
        line(const point &p1, const point &p2) {
            a = p2.y - p1.y;
            b = p1.x - p2.x;
            c = p2.x * p1.y - p1.x * p2.y;
        }

        // 构造函数: 通过斜率和一个点构造直线(kx - y + b = 0)
        line(double k, const point &p) {
            a = k;
            b = -1;
            c = -(k * p.x - p.y);
        }

        // 新增构造函数: 通过斜率和截距构造直线
        line(double slope, double intercept) {
            a = slope;
            b = -1;
            c = intercept;
        }

        // 支持垂直直线, 垂直直线无法用斜截式表示(因为斜率无穷大)
        line(double x_intercept, bool is_vertical) {
            if (is_vertical) {
                a = 1;
                b = 0;
                c = -x_intercept;  // x = x_intercept => 1x + 0y - x_intercept = 0
            } else {
                // 默认构造为水平线 y = x_intercept
                a = 0;
                b = 1;
                c = -x_intercept;
            }
        }

        // 新增: 构造水平线 y = y_value
        static line horizontal(double y_value) {
            return line(0, 1, -y_value);  // 0x + 1y - y_value = 0
        }

        // 新增: 构造垂直线 x = x_value
        static line vertical(double x_value) {
            return line(1, 0, -x_value);  // 1x + 0y - x_value = 0
        }

        // 返回斜率(如果存在)
        std::optional<double> slope() const {
            if (b == 0.0) {
                return std::nullopt; // 垂直线, 无定义斜率
            }
            return -a / b;
        }

        // 返回直线与 x 轴的夹角(弧度制)
        double angle() const {
            // 一般式中, 方向向量为 (b, -a), 所以角度为 atan2(-a, b)
            return std::atan2(-a, b);
        }

        // 返回直线与 x 轴的夹角(弧度), 范围 [0, π)
        double angle_radians() const {
            double theta = std::atan2(-a, b); // 方向角
            theta = std::abs(theta);          // 取绝对值, 变成 [0, π]
            if (theta > std::numbers::pi) theta -= std::numbers::pi;  // 调整到 [0, π)
            return theta;
        }

        // 返回直线与 x 轴的夹角(角度), 范围 [0°, 180°)
        double angle_degrees() const {
            return angle_radians() * 180.0 / std::numbers::pi;
        }

        // 新增: 计算与另一条直线的交点
        std::optional<point> intersection_with(const line &other) const {
            double denominator = a * other.b - other.a * b;

            // 判断是否平行或重合
            if (std::abs(denominator) < 1e-8) {
                return std::nullopt;
            }

            double x = (b * other.c - other.b * c) / denominator;
            double y = (other.a * c - a * other.c) / denominator;

            return point{x, y};
        }

        // 静态方法: 计算两条平行线之间的距离
        static double distance_between_parallel_lines(const line& line1, const line& line2) {
            // 确保是平行线(方向向量共线)
            double cross = line1.a * line2.b - line2.a * line1.b;
            if (std::fabs(cross) > 1e-8) {
                throw std::invalid_argument("Lines are not parallel.");
            }

            // 使用公式: |c2 - c1| / sqrt(a^2 + b^2)
            double numerator = std::fabs(line2.c - line1.c);
            double denominator = std::sqrt(line1.a * line1.a + line1.b * line1.b);

            if (denominator < 1e-8) {
                throw std::invalid_argument("Invalid line: a and b cannot both be zero.");
            }

            return numerator / denominator;
        }

        // 给定 x, 求 y(如果可能)
        std::optional<double> y(double x) const {
            if (std::fabs(b) < 1e-8) {
                return std::nullopt; // 垂直线, 无法唯一确定 y
            }
            return (-a * x - c) / b;
        }

        // 给定 y, 求 x(如果可能)
        std::optional<double> x(double y) const {
            if (std::fabs(a) < 1e-8) {
                return std::nullopt; // 水平线, 无法唯一确定 x
            }
            return (-b * y - c) / a;
        }

        // 静态方法: 返回通过点 p 且与当前直线平行的新直线
        static line parallel_line_through_point(const line& original, const point& p) {
            double new_c = -(original.a * p.x + original.b * p.y);
            return line(original.a, original.b, new_c);
        }

        // 静态方法: 返回与 line1 平行, 且关于点 p 对称的直线 line2
        static line symmetric_parallel_line(const line& line1, const point& p) {
            double numerator = line1.a * p.x + line1.b * p.y + line1.c;
            double new_c = -2 * numerator - line1.c;
            return line(line1.a, line1.b, new_c);
        }

        // 构造镜像线(关于垂直线 x = x_axis)
        line static mirror_line(const line& original, double x_axis) {
            // 构造镜像线(关于垂直线 x = x_axis)
            double a = original.a;
            double b = original.b;
            double c = original.c;

            // 原直线: a x + b y + c = 0
            // 镜像线: a (2*x_axis - x) + b y + c = 0
            // 展开后得: -a x + b y + (2*a*x_axis + c) = 0

            return line(-a, b, 2 * a * x_axis + c);
        }
    };

} // namespace ta

#endif //QUANT1X_TECHNICAL_ANALYSIS_TREND_H
