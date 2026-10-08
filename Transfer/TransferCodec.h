#pragma once
#include "TransferMessage.h"
#include <vector>

class TransferCodec
{
public:
    static std::vector<uint8_t> Encode(const TransferMessage& message);
    static bool Decode(const char* data, uint32_t size, TransferMessage& message);
};
