#pragma once

// ─────────────────────────────────────────────────────────────────────────────
// FrostUI Signals (Observer Pattern)
// ─────────────────────────────────────────────────────────────────────────────

#include "frost/core/types.hpp"
#include <functional>
#include <vector>
#include <algorithm>

namespace frost {

// ─────────────────────────────────────────────────────────────────────────────
// Connection Handle
// ─────────────────────────────────────────────────────────────────────────────

class Connection {
public:
    Connection() = default;
    Connection(u64 id, std::function<void()> disconnect_fn)
        : id_(id), disconnect_(std::move(disconnect_fn)) {}

    void disconnect() {
        if (disconnect_) {
            disconnect_();
            disconnect_ = nullptr;
        }
    }

    [[nodiscard]] bool is_connected() const { return disconnect_ != nullptr; }
    [[nodiscard]] u64 id() const { return id_; }

private:
    u64 id_{0};
    std::function<void()> disconnect_;
};

// ─────────────────────────────────────────────────────────────────────────────
// Scoped Connection (RAII)
// ─────────────────────────────────────────────────────────────────────────────

class ScopedConnection {
public:
    ScopedConnection() = default;
    explicit ScopedConnection(Connection conn) : connection_(std::move(conn)) {}

    ~ScopedConnection() {
        connection_.disconnect();
    }

    ScopedConnection(const ScopedConnection&) = delete;
    ScopedConnection& operator=(const ScopedConnection&) = delete;

    ScopedConnection(ScopedConnection&& other) noexcept
        : connection_(std::move(other.connection_)) {
        other.connection_ = Connection{};
    }

    ScopedConnection& operator=(ScopedConnection&& other) noexcept {
        if (this != &other) {
            connection_.disconnect();
            connection_ = std::move(other.connection_);
            other.connection_ = Connection{};
        }
        return *this;
    }

    void disconnect() { connection_.disconnect(); }
    [[nodiscard]] bool is_connected() const { return connection_.is_connected(); }

    Connection release() {
        Connection result = std::move(connection_);
        connection_ = Connection{};
        return result;
    }

private:
    Connection connection_;
};

// ─────────────────────────────────────────────────────────────────────────────
// Signal
// ─────────────────────────────────────────────────────────────────────────────

template<typename... Args>
class Signal {
public:
    using Slot = std::function<void(Args...)>;

    Signal() = default;
    ~Signal() = default;

    Signal(const Signal&) = delete;
    Signal& operator=(const Signal&) = delete;

    Signal(Signal&&) = default;
    Signal& operator=(Signal&&) = default;

    Connection connect(Slot slot) {
        u64 id = next_id_++;
        slots_.push_back({id, std::move(slot)});

        return Connection(id, [this, id]() {
            disconnect(id);
        });
    }

    [[nodiscard]] ScopedConnection connect_scoped(Slot slot) {
        return ScopedConnection(connect(std::move(slot)));
    }

    void disconnect(u64 id) {
        auto it = std::find_if(slots_.begin(), slots_.end(),
            [id](const SlotEntry& e) { return e.id == id; });

        if (it != slots_.end()) {
            if (is_emitting_) {
                it->slot = nullptr; // Mark for lazy removal
            } else {
                slots_.erase(it);
            }
        }
    }

    void disconnect_all() {
        slots_.clear();
    }

    void emit(Args... args) {
        is_emitting_ = true;

        for (auto& entry : slots_) {
            if (entry.slot) {
                entry.slot(args...);
            }
        }

        is_emitting_ = false;

        // Remove disconnected slots
        slots_.erase(
            std::remove_if(slots_.begin(), slots_.end(),
                [](const SlotEntry& e) { return !e.slot; }),
            slots_.end()
        );
    }

    void operator()(Args... args) {
        emit(std::forward<Args>(args)...);
    }

    [[nodiscard]] usize connection_count() const { return slots_.size(); }
    [[nodiscard]] bool has_connections() const { return !slots_.empty(); }

private:
    struct SlotEntry {
        u64 id;
        Slot slot;
    };

    Vector<SlotEntry> slots_;
    u64 next_id_{1};
    bool is_emitting_{false};
};

// ─────────────────────────────────────────────────────────────────────────────
// Property (Value with change notification)
// ─────────────────────────────────────────────────────────────────────────────

template<typename T>
class Property {
public:
    using ChangeSignal = Signal<const T&, const T&>; // old, new

    Property() = default;
    explicit Property(T initial) : value_(std::move(initial)) {}

    [[nodiscard]] const T& get() const { return value_; }
    [[nodiscard]] const T& operator*() const { return value_; }
    [[nodiscard]] const T* operator->() const { return &value_; }

    void set(T new_value) {
        if (value_ != new_value) {
            T old_value = std::move(value_);
            value_ = std::move(new_value);
            on_changed_.emit(old_value, value_);
        }
    }

    Property& operator=(T new_value) {
        set(std::move(new_value));
        return *this;
    }

    Connection on_change(typename ChangeSignal::Slot slot) {
        return on_changed_.connect(std::move(slot));
    }

    [[nodiscard]] ScopedConnection on_change_scoped(typename ChangeSignal::Slot slot) {
        return ScopedConnection(on_change(std::move(slot)));
    }

private:
    T value_{};
    ChangeSignal on_changed_;
};

} // namespace frost
