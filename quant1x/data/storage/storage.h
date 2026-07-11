#pragma once
#ifndef QUANT1X_DATA_STORAGE_STORAGE_H
#define QUANT1X_DATA_STORAGE_STORAGE_H 1

#include <quant1x/encoding/csv.h>
#include <quant1x/data/meta/timestamp.h>
#include <quant1x/data/meta/instrument.h>
#include <quant1x/base/filesystem.h>

#include <string>
#include <vector>

namespace quant1x::data::storage {

    /**
     * @brief 文件存储接口（专用于单一数据类型 T）
     *
     * 提供抽象的文件存储生命周期：初始化 → 更新 → 加载/保存。
     * 子类只需实现文件名生成和是否需要初始化/更新的判断逻辑。
     *
     * @tparam T 数据类型
     */
    template<typename T>
    class FileStorage {
    protected:
        std::string _file_name;

    public:
        FileStorage() = default;

        virtual ~FileStorage() = default;

        FileStorage(const FileStorage&) = delete;
        FileStorage& operator=(const FileStorage&) = delete;
        FileStorage(FileStorage&&) = default;
        FileStorage& operator=(FileStorage&&) = default;

        /**
         * @brief 返回文件名
         */
        [[nodiscard]] virtual std::string file_name() const = 0;

        /**
         * @brief 判断是否需要初始化
         * @param timestamp 时间戳
         */
        [[nodiscard]] virtual bool should_initialize(
            const meta::Timestamp& timestamp) const = 0;

        /**
         * @brief 判断是否需要更新
         * @param timestamp 时间戳
         */
        [[nodiscard]] virtual bool should_update(
            const meta::Timestamp& timestamp) const = 0;

        /**
         * @brief 更新数据（无参，类型已固定）
         */
        virtual void update() = 0;

        /**
         * @brief 加载数据
         */
        [[nodiscard]] std::vector<T> load() const {
            return encoding::csv::csv_to_slices<T>(_file_name);
        }

        /**
         * @brief 保存数据
         */
        void save(const std::vector<T>& data) const {
            encoding::csv::slices_to_csv(data, _file_name);
        }

        /**
         * @brief 检出数据（自动更新 + 加载）
         */
        [[nodiscard]] std::vector<T> checkout(const meta::Timestamp& timestamp = meta::Timestamp::now()) {
            if (should_initialize(timestamp) || should_update(timestamp)) {
                update();
            }
            return load();
        }
    };

    /**
     * @brief 基础数据文件存储类
     *
     * 用于存储与具体证券标的（Instrument）关联的基础数据。
     *
     * @tparam T 数据类型
     */
    template<typename T>
    class BasedataFileStorage : public FileStorage<T> {
    protected:
        meta::Instrument _inst;

    public:
        explicit BasedataFileStorage(const meta::Instrument& inst)
            : _inst(inst) {
            this->_file_name = this->file_name();
        }

        explicit BasedataFileStorage(meta::Instrument&& inst)
            : _inst(std::move(inst)) {
            this->_file_name = this->file_name();
        }
    };

    /**
     * @brief 元数据文件存储类
     *
     * 用于存储与数据类型绑定的元数据，文件名自动生成为 "{TypeName}.csv"。
     *
     * @tparam T 数据类型
     */
    template<typename T>
    class MetaFileStorage : public FileStorage<T> {
    public:
        MetaFileStorage() {
            this->_file_name = this->file_name();
        }

        [[nodiscard]] std::string file_name() const override {
            return std::string(typeid(T).name()) + ".csv";
        }
    };

} // namespace quant1x::data::storage

#endif // QUANT1X_DATA_STORAGE_STORAGE_H
