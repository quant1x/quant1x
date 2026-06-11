#include <quant1x/test/test.h>

#include <iostream>
#include <vector>
#include <cmath>
#include <queue>
#include <quant1x/contrib/data/tdx/level1/transaction_data.h>
#include <quant1x/config/cache.h>

using namespace std;

// 定义二维点
struct Point {
    double x, y;
    int cluster; // -1 表示未访问或噪声点
};

class DBSCAN {
public:
    vector<Point> points;
    double eps;
    size_t minPts;

    DBSCAN(double eps, int minPts) : eps(eps), minPts(minPts) {}

    // 计算两点之间的欧几里得距离
    double dist(const Point &a, const Point &b) {
        return sqrt((a.x - b.x)*(a.x - b.x) + (a.y - b.y)*(a.y - b.y));
    }

    // 获取所有在eps范围内的邻近点
    vector<size_t> regionQuery(size_t idx) {
        vector<size_t> neighbors;
        for (size_t i = 0; i < points.size(); ++i) {
            if (dist(points[idx], points[i]) <= eps) {
                neighbors.emplace_back(i);
            }
        }
        return neighbors;
    }

    // 扩展簇
    void expandCluster(size_t idx, int clusterId) {
        queue<size_t> seeds;
        vector<size_t> neighbors = regionQuery(idx);

        if (neighbors.size() < minPts) {
            points[idx].cluster = -1; // 噪声点
            return;
        }

        // 标记为核心点并分配簇ID
        points[idx].cluster = clusterId;

        // 将邻近点加入队列
        for (size_t neighborIdx : neighbors) {
            if (points[neighborIdx].cluster == -1) { // 如果是噪声点
                points[neighborIdx].cluster = clusterId;
                seeds.push(neighborIdx);
            } else if (points[neighborIdx].cluster == 0) { // 未访问
                points[neighborIdx].cluster = clusterId;
                seeds.push(neighborIdx);
            }
        }

        // BFS 扩展簇
        while (!seeds.empty()) {
            size_t currentIdx = seeds.front();
            seeds.pop();

            vector<size_t> neighborOfNeighbor = regionQuery(currentIdx);

            if (neighborOfNeighbor.size() >= minPts) {
                for (size_t neighborIdx : neighborOfNeighbor) {
                    if (points[neighborIdx].cluster == -1) {
                        points[neighborIdx].cluster = clusterId;
                        seeds.push(neighborIdx);
                    } else if (points[neighborIdx].cluster == 0) {
                        points[neighborIdx].cluster = clusterId;
                        seeds.push(neighborIdx);
                    }
                }
            }
        }
    }

    // 主函数执行DBSCAN
    int run() {
        int clusterId = 1;

        for (size_t i = 0; i < points.size(); ++i) {
            if (points[i].cluster != 0)
                continue;

            vector<size_t> neighbors = regionQuery(i);

            if (neighbors.size() < minPts) {
                points[i].cluster = -1; // 噪声点
            } else {
                expandCluster(i, clusterId);
                ++clusterId;
            }
        }

        return clusterId - 1; // 返回总簇数
    }
};

TEST_CASE("dbscan-v1", "[ta]") {
    vector<Point> data = {
        {0, 0, 0},
        {1, 1, 0},
        {1, 0, 0},
        {0, 1, 0},
        {5, 5, 0},
        {5, 6, 0},
        {6, 5, 0},
        {6, 6, 0},
        {10, 10, 0}
    };

    DBSCAN dbscan(1.5, 3); // 设置 eps=1.5, minPts=3
    dbscan.points = data;

    int numClusters = dbscan.run();

    cout << "找到 " << numClusters << " 个簇：" << endl;
    for (size_t i = 0; i < dbscan.points.size(); ++i) {
        cout << "点 (" << dbscan.points[i].x << ", " << dbscan.points[i].y
             << ") 属于簇: " << dbscan.points[i].cluster << endl;
    }
}

vector<level1::TickTransaction> readCSV(const string &filename) {
    vector<level1::TickTransaction> ticks;
    ifstream file(filename);
    string line;

    if (!file.is_open()) {
        cerr << "无法打开文件: " << filename << endl;
        return ticks;
    }

    // 跳过表头
    getline(file, line);

    while (getline(file, line)) {
        stringstream ss(line);
        string token;
        level1::TickTransaction t;

        getline(ss, token, ','); t.time = token;
        getline(ss, token, ','); t.price = stod(token);
        getline(ss, token, ','); t.vol = stoll(token);
        getline(ss, token, ','); t.num = stoll(token);
        getline(ss, token, ','); t.amount = stod(token);
        getline(ss, token, ','); t.buyOrSell = stoll(token);

        ticks.push_back(t);
    }

    return ticks;
}

// DBSCAN 使用的点结构
struct Point2 {
    vector<double> features;
    int cluster; // -1 = noise, 0 = unvisited, >0 = cluster id
};

vector<Point2> extractFeatures(const vector<level1::TickTransaction>& ticks) {
    vector<Point2> points;

    for (size_t i = 0; i < ticks.size(); ++i) {
        Point2 p;
        p.features.push_back(ticks[i].price);               // 价格
        p.features.push_back(static_cast<double>(ticks[i].vol)); // 成交量
        p.features.push_back(static_cast<double>(ticks[i].buyOrSell)); // 买卖方向
        p.features.push_back(static_cast<double>(i));        // 序号代替时间
        p.cluster = 0;
        points.push_back(p);
    }

    return points;
}

class DBSCAN2 {
public:
    vector<Point2> points;
    double eps;
    size_t minPts;

    DBSCAN2(double eps, size_t minPts) : eps(eps), minPts(minPts) {}

    double dist(const Point2 &a, const Point2 &b) {
        double sum = 0.0;
        for (size_t i = 0; i < a.features.size(); ++i)
            sum += pow(a.features[i] - b.features[i], 2);
        return sqrt(sum);
    }

    vector<size_t> regionQuery(size_t idx) {
        vector<size_t> neighbors;
        for (size_t i = 0; i < points.size(); ++i) {
            if (dist(points[idx], points[i]) <= eps)
                neighbors.push_back(i);
        }
        return neighbors;
    }

    void expandCluster(size_t idx, int clusterId) {
        queue<size_t> seeds;
        auto neighbors = regionQuery(idx);

        if (neighbors.size() < minPts) {
            points[idx].cluster = -1;
            return;
        }

        points[idx].cluster = clusterId;

        for (size_t neighborIdx : neighbors) {
            if (points[neighborIdx].cluster == -1 || points[neighborIdx].cluster == 0) {
                points[neighborIdx].cluster = clusterId;
                seeds.push(neighborIdx);
            }
        }

        while (!seeds.empty()) {
            size_t current = seeds.front();
            seeds.pop();

            auto subNeighbors = regionQuery(current);
            if (subNeighbors.size() >= minPts) {
                for (size_t n : subNeighbors) {
                    if (points[n].cluster == -1 || points[n].cluster == 0) {
                        points[n].cluster = clusterId;
                        seeds.push(n);
                    }
                }
            }
        }
    }

    int run() {
        int clusterId = 1;

        for (size_t i = 0; i < points.size(); ++i) {
            if (points[i].cluster != 0) continue;

            auto neighbors = regionQuery(i);
            if (neighbors.size() < minPts)
                points[i].cluster = -1;
            else {
                expandCluster(i, clusterId);
                ++clusterId;
            }
        }

        return clusterId - 1;
    }
};

void saveClusteredData(const vector<level1::TickTransaction>& ticks, const vector<Point2>& points, const string& outputFile) {
    ofstream out(outputFile);
    out << "time,price,vol,num,amount,buyOrSell,cluster\n";

    for (size_t i = 0; i < ticks.size(); ++i) {
        out << ticks[i].time << ","
            << ticks[i].price << ","
            << ticks[i].vol << ","
            << ticks[i].num << ","
            << ticks[i].amount << ","
            << ticks[i].buyOrSell << ","
            << points[i].cluster << "\n";
    }

    out.close();
    cout << "聚类结果已保存至: " << outputFile << endl;
}

TEST_CASE("dbscan-v2", "[ta]") {
    std::string code = "sh600600";
    std::string date = "2025-06-19";
    std::string inputFile = config::get_historical_trade_filename(code, date);
    //string inputFile = "tick_data.csv";       // 输入文件路径
    string outputFile = "clustered_tick.csv"; // 输出文件路径

    auto ticks = readCSV(inputFile);
    auto points = extractFeatures(ticks);

    DBSCAN2 dbscan(2, 3); // eps=2.0, minPts=3
    dbscan.points = points;

    int numClusters = dbscan.run();
    cout << "找到 " << numClusters << " 个簇" << endl;

    saveClusteredData(ticks, dbscan.points, outputFile);
}