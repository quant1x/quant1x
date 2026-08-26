// Copyright (c) 2026 Quant1X. All rights reserved.
// Author: wangfeng <wangfengxy@sina.cn>
// SPDX-License-Identifier: MIT

package quant1x.id;

import java.io.IOException;
import java.util.Optional;

/**
 * HLC 状态存储抽象，对应 Go 版的 {@code stateStore} 接口。
 *
 * <p>实现必须保证 {@link #next} 的原子性与跨实例串行化
 * （例如通过文件锁），从而支撑"跨重启强唯一"。
 */
interface StateStore {

    /**
     * 加载持久化的最新状态。
     *
     * @return 若状态文件不存在则返回 {@link Optional#empty()}
     * @throws IOException 状态文件损坏且无有效记录时抛出
     */
    Optional<PersistentState> load() throws IOException;

    /**
     * 以当前内存状态与本地时钟推进状态，并持久化返回的新状态。
     *
     * @param local 当前内存中的状态
     * @param nowMs 当前物理时间（毫秒）
     * @param seed  逻辑种子（16 位）
     * @return 推进后的新状态
     * @throws IOException 存储失败时抛出
     */
    PersistentState next(PersistentState local, long nowMs, int seed) throws IOException;
}
