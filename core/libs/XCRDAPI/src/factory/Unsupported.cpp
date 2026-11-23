#include "Factory.hpp"

#include <anybox/Unimplemented.hpp>

std::int32_t AnyBox::Storage::Factory::CreateQueue(const StorageQueueDescription*, const Guid&, void**) {
    ANYBOX_NOT_IMPLEMENTED();
}

std::int32_t AnyBox::Storage::Factory::CreateQueue1(const StorageQueueDescription1*, const Guid&, void**) {
    ANYBOX_NOT_IMPLEMENTED();
}

void AnyBox::Storage::Factory::GetStats(StorageStatistics*) {
    ANYBOX_NOT_IMPLEMENTED();
}
