#ifndef IMAGECACHE_H
#define IMAGECACHE_H

#include <memory>
#include <vector>
#include <atomic>
#include <filesystem>
#include <shared_mutex>
#include <unordered_map>
#include <unordered_set>

#include "uuid.h"

namespace ThorQ {
class ImageCache
{
    struct Info
    {
        Info(const ThorQ::ImageCache::Info& other);
        Info(ThorQ::Uuid id, std::size_t size);
        ThorQ::Uuid id;
        std::array<std::atomic_uint32_t, 4> velocity; // last hour
        std::uint64_t size;
        bool isLoaded;

        Info& operator=(const ThorQ::ImageCache::Info& other);
        bool operator==(const ThorQ::ImageCache::Info& other);
        bool operator!=(const ThorQ::ImageCache::Info& other);
    };
public:
    ImageCache();
    ~ImageCache();

    bool add(ThorQ::Uuid id, std::shared_ptr<std::vector<std::uint8_t>> data);
    bool get(ThorQ::Uuid id, std::shared_ptr<std::vector<std::uint8_t>> data);
private:
    static bool accumSorter(ThorQ::ImageCache::Info lhs, ThorQ::ImageCache::Info rhs);

    // Cache metadata modifiers
    void syncWithStorage();
    void sortByVelocity();

    // Cache modifiers
    void reCache();
    void unCache();

    bool noLock_saveImage(ThorQ::Uuid id, std::shared_ptr<std::vector<std::uint8_t>> data);
    bool noLock_loadImage(ThorQ::Uuid id);
    void nolock_unLoadImage(ThorQ::Uuid id);

    std::filesystem::path m_directory;

    std::atomic_uint64_t m_cacheSize;
    std::atomic_uint64_t m_cacheRoof;

    std::shared_mutex l_meta;
    std::vector<ThorQ::ImageCache::Info> m_meta;

    std::shared_mutex l_cache;
    std::unordered_map<ThorQ::Uuid, std::shared_ptr<std::vector<std::uint8_t>>> m_cache;
};
}

#endif // IMAGECACHE_H
