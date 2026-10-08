#pragma once
#include "TransferProtocol.h"


struct TransferMessage{
    Transfer::MessageHeader header{};
    std::vector<uint8_t> payload;
};
