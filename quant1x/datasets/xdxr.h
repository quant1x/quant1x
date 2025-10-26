#pragma once
#ifndef QUANT1X_DATASETS_XDXR_H
#define QUANT1X_DATASETS_XDXR_H 1

#include <quant1x/datasets/base.h>
#include <quant1x/level1/client.h>
#include <quant1x/engine/action.h>

namespace factors {
    struct CumulativeAdjustment;
}

namespace datasets {

    // 加载除权除息记录
    std::vector<level1::XdxrInfo> load_xdxr(const std::string& code);

    // 除权除息
    class DataXdxr : public cache::DataAdapter {
    public:
        cache::Kind Kind() const override;
        std::string Owner() override ;
        std::string Key() const override ;
        std::string Name() const override ;
        std::string Usage() const override;
        void Print(const std::string &code, const std::vector<exchange::timestamp> &dates) override;
        void Update(const std::string &code, const exchange::timestamp &date) override;
    private:
        //void save_xdxr(const std::string &code, const std::string &date, const std::vector<level1::XdxrInfo>& values);
    };
}

#endif //QUANT1X_DATASETS_XDXR_H
