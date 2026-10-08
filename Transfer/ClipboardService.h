#pragma once

#include "TransferProtocol.h"
#include <functional>
#include <mutex>

class ClipboardService
{
public:
    using Callback = std::function<void(const Transfer::ClipboardData &)>;
    void SetCallback(Callback callback);
    void Handle(uint8_t format, const uint8_t *data, size_t size);
    Transfer::ClipboardData LastData() const;

private:
    mutable std::mutex mutex_;
    Transfer::ClipboardData last_data_{};
    Callback callback_;
};
