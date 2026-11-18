#ifndef ESR_RENDER_RESOURCE_H
#define ESR_RENDER_RESOURCE_H

#include <cmath>
#include <cstring>
#include <optional>
#include <vector>

#include "../render_api.h"

namespace esr {

template <typename T>
class UniformBuffer {
public:
    void set_value(const T& new_value) {
        value = new_value;
    }

    std::shared_ptr<Pathfinder::Buffer> get_buffer() const {
        return buffer.value_or(nullptr);
    }

    void write_buffer(R_RenderDevice& device, R_RenderQueue& queue) {
        if (!buffer.has_value()) {
            Pathfinder::BufferDescriptor desc{
                Pathfinder::BufferType::Uniform,
                sizeof(T),
                Pathfinder::MemoryProperty::HostVisibleAndCoherent
            };
            buffer = device.driver_->create_buffer(desc, label.value_or("UniformBuffer"));
        }

        buffer.value()->upload_via_mapping(sizeof(T), 0, &value);
    }

private:
    T value;
    std::optional<std::shared_ptr<Pathfinder::Buffer>> buffer;
    std::optional<std::string> label;
};

const uint32_t UNIFORM_BUFFER_OFFSET_ALIGNMENT = 256;

template <typename T>
class DynamicUniformBuffer {
public:
    DynamicUniformBuffer() {
        aligned_size =
            std::ceil((float)sizeof(T) / (float)UNIFORM_BUFFER_OFFSET_ALIGNMENT) * UNIFORM_BUFFER_OFFSET_ALIGNMENT;
    }

    std::shared_ptr<Pathfinder::Buffer> get_buffer() const {
        return buffer.value_or(nullptr);
    }

    size_t get_aligned_size() const {
        return aligned_size;
    }

    void write_buffer(R_RenderDevice& device, R_RenderQueue& queue) {
        if (values.empty()) {
            return;
        }

        auto buffer_size = values.size();

        if (changed || !buffer.has_value()) {
            Pathfinder::BufferDescriptor desc{
                Pathfinder::BufferType::Uniform,
                buffer_size,
                Pathfinder::MemoryProperty::HostVisibleAndCoherent
            };
            buffer = device.driver_->create_buffer(desc, label);

            capacity = get_uniform_count();
            changed = false;
        }

        buffer.value()->upload_via_mapping(buffer_size, 0, values.data());
    }

    uint32_t push(const T& value) {
        values.reserve(values.size() + aligned_size);
        for (size_t i = 0; i < aligned_size; i++) {
            values.emplace_back(0);
        }

        std::memcpy(values.data() + (get_uniform_count() - 1) * aligned_size, &value, sizeof(value));

        if (get_uniform_count() > capacity) {
            changed = true;
        }

        return get_uniform_count() - 1;
    }

    size_t get_uniform_count() {
        assert((values.size() % aligned_size) == 0);
        return values.size() / aligned_size;
    }

    void clear() {
        values.clear();
    }

private:
    std::vector<uint8_t> values;
    size_t aligned_size = 0;
    uint32_t capacity = 0;
    std::optional<std::shared_ptr<Pathfinder::Buffer>> buffer;
    std::string label = "DynamicUniformBuffer";
    bool changed = false;
};

template <typename T>
class BufferVec {
public:
    BufferVec() = default;

    BufferVec(Pathfinder::BufferType usage) {
        buffer_type = usage;
    }

    void clear() {
        values.clear();
    }

    size_t push(T value) {
        auto index = values.size();
        values.push_back(value);
        return index;
    }

    void set_values(const std::vector<T>& new_values) {
        values = new_values;
    }

    size_t get_size() const {
        return values.size();
    }

    std::optional<std::shared_ptr<Pathfinder::Buffer>> get_buffer() const {
        return buffer;
    }

    void reserve(size_t capacity_, R_RenderDevice& device) {
        if (capacity_ > capacity || label_changed) {
            capacity = capacity_;

            auto size = item_size * capacity;

            Pathfinder::BufferDescriptor desc{
                buffer_type,
                size,
                Pathfinder::MemoryProperty::HostVisibleAndCoherent
            };

            buffer = device.driver_->create_buffer(desc, label.value_or("BufferVec"));

            label_changed = false;
        }
    }

    void write_buffer(R_RenderDevice& device, R_RenderQueue& queue) {
        if (values.empty()) {
            return;
        }

        reserve(values.size(), device);

        if (buffer.has_value()) {
            buffer.value()->upload_via_mapping(item_size * values.size(), 0, values.data());
        }
    }

private:
    std::vector<T> values;
    std::optional<std::shared_ptr<Pathfinder::Buffer>> buffer;
    size_t capacity = 0;
    size_t item_size = sizeof(T);
    Pathfinder::BufferType buffer_type = Pathfinder::BufferType::Vertex;
    std::optional<std::string> label;
    bool label_changed = false;
};

} // namespace esr

#endif // ESR_RENDER_RESOURCE_H
