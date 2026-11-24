#include <anybox/texture/AddressSpace.hpp>
#include <exception>
#include <stdexcept>

AnyBox::Texture::AddressSpace::~AddressSpace() {
    if (!resources.empty()) std::terminate();
}

void AnyBox::Texture::AddressSpace::insert(std::uint64_t address, Resource& resource) {
    std::scoped_lock lock(mutex);
    if (!resources.emplace(address, &resource).second) throw std::logic_error("Texture GPU address is already registered");
}

void AnyBox::Texture::AddressSpace::remove(std::uint64_t address) noexcept {
    std::scoped_lock lock(mutex);
    if (resources.erase(address) != 1) std::terminate();
}

AnyBox::Texture::Resource& AnyBox::Texture::AddressSpace::Resolve(std::uint64_t address) const {
    std::scoped_lock lock(mutex);
    auto found = resources.find(address);
    if (found == resources.end()) throw std::out_of_range("Texture GPU address is not registered");
    return *found->second;
}
