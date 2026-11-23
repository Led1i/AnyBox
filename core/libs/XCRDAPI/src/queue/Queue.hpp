#ifndef ANYBOX_STORAGE_QUEUE_HPP
#define ANYBOX_STORAGE_QUEUE_HPP

#include "Operation.hpp"
#include "../common/ReferenceCount.hpp"
#include "../compression/ZlibDecoder.hpp"

#include <StorageInterfaces.hpp>
#include "../staging/Pool.hpp"

#include <condition_variable>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <vector>

namespace AnyBox::Storage {

class Queue final : public IStorageQueue {
public:
    explicit Queue(const StorageQueueDescription1& description);
    ~Queue();
    std::int32_t QueryInterface(const Guid& interfaceId, void** object) override;
    std::uint32_t AddRef() override;
    std::uint32_t Release() override;
    void EnqueueRequest(const StorageRequest* request) override;
    void EnqueueStatus(IStorageStatusArray* array, std::uint32_t index) override;
    void EnqueueSignal(IStorageUnknown* fence, std::uint64_t value) override;
    void Submit() override;
    void CancelRequestsWithTag(std::uint64_t mask, std::uint64_t value) override;
    void Close() override;
    void* GetErrorEvent() override;
    void RetrieveErrorRecord(StorageErrorRecord* record) override;
    void Query(StorageQueueInformation* information) override;
    void EnqueueSetEvent(void* event) override;
    void Query1(StorageQueueInformation1* information) override;

private:
    void requireSpace() const;
    void publish();
    void run();
    void read(ReadOperation& operation);
    void readFile(ReadOperation& operation, std::span<std::byte> destination);
    void decompress(std::span<const std::byte> source, std::span<std::byte> destination);
    StorageObjectRegistration registration;
    ReferenceCount references{1};
    StorageQueueDescription1 description;
    std::string name;
    std::mutex mutex;
    std::condition_variable wake;
    std::condition_variable drained;
    std::vector<Operation> operations;
    std::size_t head = 0;
    std::size_t count = 0;
    std::size_t submitted = 0;
    bool busy = false;
    bool closing = false;
    bool closed = false;
    std::optional<ZlibDecoder> decoder;
    std::unique_ptr<std::byte[]> overlapBuffer;
    std::size_t overlapCapacity = 0;
    Handle errorEvent;
    Handle readEvent;
    std::jthread worker;
};

}

#endif
