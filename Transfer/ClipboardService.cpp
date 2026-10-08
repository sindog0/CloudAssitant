#include "ClipboardService.h"

void ClipboardService::SetCallback(Callback callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    callback_ = std::move(callback);
}

void ClipboardService::Handle(uint8_t format, const uint8_t *data, size_t size)
{
    Callback callback;
    Transfer::ClipboardData value;
    value.format = format;
    if (size != 0) value.data.assign(data, data + size);
    {
        std::lock_guard<std::mutex> lock(mutex_);
        last_data_ = value;
        callback = callback_;
    }
    if (callback) callback(value);
}

Transfer::ClipboardData ClipboardService::LastData() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return last_data_;
}
