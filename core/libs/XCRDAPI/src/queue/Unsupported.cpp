#include "Queue.hpp"

#include <anybox/Unimplemented.hpp>

void AnyBox::Storage::Queue::EnqueueSignal(IStorageUnknown*, std::uint64_t) { ANYBOX_NOT_IMPLEMENTED(); }
void AnyBox::Storage::Queue::RetrieveErrorRecord(StorageErrorRecord*) { ANYBOX_NOT_IMPLEMENTED(); }
void AnyBox::Storage::Queue::Query(StorageQueueInformation*) { ANYBOX_NOT_IMPLEMENTED(); }
void AnyBox::Storage::Queue::Query1(StorageQueueInformation1*) { ANYBOX_NOT_IMPLEMENTED(); }
