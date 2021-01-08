#include "imagecache.h"

#include <numeric>
#include <fstream>

inline std::string lowercase(std::string str)
{
    std::transform(str.begin(), str.end(), str.begin(), tolower);
    return str;
}
inline std::string& lowercase(std::string& str)
{
    std::transform(str.begin(), str.end(), str.begin(), tolower);
    return str;
}

ThorQ::ImageCache::Info::Info(const ThorQ::ImageCache::Info &other)
    : id(other.id)
    , size(other.size)
    , isLoaded(other.isLoaded)
{
    for (std::size_t i = 0; i < velocity.size(); i++)
    {
        velocity[i].store(other.velocity[i].load());
    }
}
ThorQ::ImageCache::Info::Info(ThorQ::Uuid id, std::size_t size)
    : id(id)
    , velocity{0}
    , size(size)
    , isLoaded(false)
{
}
ThorQ::ImageCache::Info &ThorQ::ImageCache::Info::operator=(const ThorQ::ImageCache::Info &other)
{
    id = other.id;
    size = other.size;
    isLoaded = other.isLoaded;
    for (std::size_t i = 0; i < velocity.size(); i++)
    {
        velocity[i].store(other.velocity[i].load());
    }
    return *this;
}
bool ThorQ::ImageCache::Info::operator==(const ThorQ::ImageCache::Info &other) { return id == other.id; }
bool ThorQ::ImageCache::Info::operator!=(const ThorQ::ImageCache::Info &other) { return !(*this == other); }

ThorQ::ImageCache::ImageCache()
    : m_directory()
    , m_cacheSize()
    , m_cacheRoof()
    , l_meta()
    , m_meta()
    , l_cache()
    , m_cache()
{
}

bool ThorQ::ImageCache::add(ThorQ::Uuid id, std::shared_ptr<std::vector<uint8_t>> data)
{
    std::unique_lock lu_m(l_meta);
    std::unique_lock lu_c(l_cache);
    noLock_saveImage(id, data);
    m_cache.emplace(id, data);
    m_meta.push_back(ThorQ::ImageCache::Info(id, data->size()));

    return true;
}

bool ThorQ::ImageCache::get(ThorQ::Uuid id, std::shared_ptr<std::vector<uint8_t> > data)
{
    m_meta.
}

bool ThorQ::ImageCache::accumSorter(ThorQ::ImageCache::Info lhs, ThorQ::ImageCache::Info rhs)
{
    return std::accumulate(lhs.velocity.begin(), lhs.velocity.end(), 0) < std::accumulate(rhs.velocity.begin(), rhs.velocity.end(), 0);
}

void ThorQ::ImageCache::syncWithStorage()
{
    std::unique_lock ls_i(l_meta);
    std::unique_lock lu_c(l_cache);

    std::filesystem::directory_iterator dir_it(m_directory, std::filesystem::directory_options::skip_permission_denied);

    // Get all directory files
    std::unordered_map<ThorQ::Uuid, std::size_t> dir_files;
    for (const auto& entry : dir_it)
    {
        auto path = entry.path();
        ThorQ::Uuid id;
        if (lowercase(path.extension().string()) == ".png" && ThorQ::Uuid::TryParse(path.filename().string(), id))
        {
            dir_files.emplace(id, entry.file_size());
        }
    }

    // Update cache
    for (auto it = m_meta.begin(); it != m_meta.end(); it++)
    {
        auto search = dir_files.find(it->id);
        if (search != dir_files.end())
        {
            dir_files.erase(search);
        }
        else
        {
            m_cache.erase(it->id);
            m_meta.erase(it--);
        }
    }

    // Add new files
    for (const auto& entry : dir_files)
    {
        m_meta.push_back(ThorQ::ImageCache::Info(entry.first, entry.second));
    }
}

void ThorQ::ImageCache::sortByVelocity()
{
    std::unique_lock l(l_meta);
    std::sort(m_meta.begin(), m_meta.end(), ThorQ::ImageCache::accumSorter);
}

void ThorQ::ImageCache::reCache()
{
    std::shared_lock ls_i(l_meta);
    std::unique_lock lu_c(l_cache);

    auto fwd_it = m_meta.begin();
    auto rev_it = m_meta.rend();

    std::size_t size = 0;
    while (fwd_it != rev_it.base()-1)
    {
        if (size <= m_cacheRoof)
        {
            fwd_it++;
            if (!fwd_it->isLoaded)
            {
                noLock_loadImage(fwd_it->id);
                fwd_it->isLoaded = true;
                size += fwd_it->size;
            }
        }
        else
        {
            rev_it++;
            if (rev_it->isLoaded)
            {
                nolock_unLoadImage(rev_it->id);
                rev_it->isLoaded = false;
                size -= rev_it->size;
            }
        }
    }
}

void ThorQ::ImageCache::unCache()
{
    std::unique_lock lu_c(l_cache);
    m_cache.clear();
}

bool ThorQ::ImageCache::noLock_saveImage(ThorQ::Uuid id, std::shared_ptr<std::vector<uint8_t>> data)
{
    std::fstream file(m_directory / id.toString().append(".png"), std::ios::out | std::ios::binary | std::ios::ate);

    if (!file.is_open())
    {
        return false;
    }

    file.write((char*)data->data(), data->size());
    file.close();

    return true;
}

bool ThorQ::ImageCache::noLock_loadImage(ThorQ::Uuid id)
{
    std::fstream file(m_directory / id.toString().append(".png"), std::ios::in | std::ios::binary | std::ios::ate);

    if (!file.is_open())
    {
        return false;
    }

    std::size_t len = file.tellg();
    file.seekg(std::ios::beg);

    auto ptr = std::make_shared<std::vector<std::uint8_t>>();
    ptr->resize(len);
    file.read((char*)ptr->data(), len);
    file.close();

    return m_cache.emplace(id, ptr).second;
}

void ThorQ::ImageCache::nolock_unLoadImage(ThorQ::Uuid id)
{
    m_cache.erase(id);
}
