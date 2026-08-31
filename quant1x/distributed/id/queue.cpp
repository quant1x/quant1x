// ID 队列的实现
#include <quant1x/distributed/id/queue.h>

#include <thread>

namespace quant1x::distributed::id {

Result<std::shared_ptr<Queue>> Queue::create(size_t capacity) {
    if (capacity == 0) {
        return Error::invalid_size();
    }
    // std::make_shared 无法访问私有构造, 显式 new 后立即交给 shared_ptr
    return std::shared_ptr<Queue>(new Queue(capacity));
}

Status Queue::try_push(const Id &value) {
    if (closed_.load(std::memory_order_acquire)) {
        return Error::closed();
    }
    // 底层 try_push 为 noexcept 实例化 (Id 为 trivial 类型), 满时返回 false
    if (inner_.try_push(value)) {
        return Error{};
    }
    return Error::queue_full();
}

Result<Id> Queue::try_pop() {
    Id value{};
    if (inner_.try_pop(value)) {
        return value;
    }
    // 底层不区分"空"与"已关闭且为空", 由关闭标志判定
    if (closed_.load(std::memory_order_acquire)) {
        return Error::closed();
    }
    return Error::queue_empty();
}

Status Queue::push(const Id &value) {
    while (true) {
        if (closed_.load(std::memory_order_acquire)) {
            return Error::closed();
        }
        if (inner_.try_push(value)) {
            return Error{};
        }
        // 队满, 让出后重试 (对齐 Go Push 的阻塞语义)
        std::this_thread::yield();
    }
}

Result<Id> Queue::pop() {
    while (true) {
        if (inner_.is_empty()) {
            if (closed_.load(std::memory_order_acquire)) {
                return Error::closed();
            }
            std::this_thread::yield();
            continue;
        }
        Id value{};
        if (inner_.try_pop(value)) {
            return value;
        }
        std::this_thread::yield();
    }
}

size_t Queue::len() const noexcept {
    return inner_.len();
}

size_t Queue::cap() const noexcept {
    return inner_.capacity();
}

void Queue::close() noexcept {
    closed_.store(true, std::memory_order_release);
    // 同时关闭底层队列, 使其内部不再接受写入语义
    inner_.close();
}

bool Queue::is_closed() const noexcept {
    return closed_.load(std::memory_order_acquire);
}

void Queue::wait_for_close() const {
    while (!inner_.is_empty()) {
        std::this_thread::yield();
    }
}

}  // namespace quant1x::distributed::id
