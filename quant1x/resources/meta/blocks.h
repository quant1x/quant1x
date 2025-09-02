#pragma once
#ifndef QUANT1X_RESOURCES_META_BLOCKS_H
#define QUANT1X_RESOURCES_META_BLOCKS_H 1

#include <quant1x/std/api.h>
#include "tdxhy.inc"
#include "tdxzs.inc"
#include "tdxzs3.inc"

struct embed_file {
    std::string filename;
    std::vector<uint8_t> data;
    int64_t timestamp;
};

inline auto resources_meta_block_files = {
    embed_file{
        .filename = tdxhy_filename,
        .data = {tdxhy_data, tdxhy_data + tdxhy_length},
        .timestamp = tdxhy_timestamp,
    },
    embed_file{
        .filename = tdxzs_filename,
        .data = {tdxzs_data, tdxzs_data + tdxzs_length},
        .timestamp = tdxzs_timestamp,
    },
    embed_file{
        .filename = tdxzs3_filename,
        .data = {tdxzs3_data, tdxzs3_data + tdxzs3_length},
        .timestamp = tdxzs3_timestamp,
    },
};




#endif //QUANT1X_RESOURCES_META_BLOCKS_H
