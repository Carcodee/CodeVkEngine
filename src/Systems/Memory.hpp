//
// Created by carlo on 2025-01-26.
//

#ifndef MEMORY_HPP
#define MEMORY_HPP

#define IS_WINDOWS

namespace Systems
{
#ifndef L1_CACHE_LINE_SIZE
#define L1_CACHE_LINE_SIZE 64
#endif
    void* AllocAligned(size_t size)
    {
#if defined(IS_WINDOWS)
        return _aligned_malloc(size, L1_CACHE_LINE_SIZE);
#elif defined (IS_OPENBSD) || defined(IS_OSX)
        void *ptr;
        if (posix_memalign(&ptr, L1_CACHE_LINE_SIZE, size) != 0)
            ptr = nullptr;
        return ptr;
#endif
        return malloc(size);
    }
    template <typename T>
    T* AllocAligned(size_t count)
    {
        return (T*)AllocAligned(count * sizeof(T));
    }
    class Arena
    {

    public:
        Arena(size_t size = 262144){
            maxBlockSize = size;
            currentBlockPos = 0; 
        }

        void* Alloc(size_t size)
        {
            size_t dataSize = size;
            if (currentBlockPos + dataSize > currentAllocSize)
            {
                if (currentBlock)
                {
                    usedBlocks.push_back(std::make_pair(currentAllocSize, currentBlock));
                    currentBlock = nullptr;
                }
                for (auto iter = availableBlocks.begin(); iter != availableBlocks.end(); iter++)
                {
                    if (iter->first >= dataSize)
                    {
                        currentAllocSize = iter->first;
                        currentBlock = iter->second;
                        availableBlocks.erase(iter);
                        break;
                    }
                }
                if (!currentBlock)
                {
                    currentAllocSize = std::max(dataSize, maxBlockSize);
                    currentBlock = (uint8_t*)AllocAligned(currentAllocSize);
                }
                currentBlockPos = 0;
            }
            size_t alignment = 16;
            dataSize = (dataSize + alignment -1) & ~ (alignment - 1);
            void* data = currentBlock + currentBlockPos;
            currentBlockPos += dataSize;
            return data;
        }

        template <typename T, typename... Args>
        T* Alloc(size_t n = 1 ,bool runConstructor = true, Args&&... args)
        {
            T* data = (T*)Alloc(n * sizeof(T));
            if (runConstructor)
            {
                for (int i = 0; i < n; ++i)
                {
                    new (&data[i])T(std::forward<Args>(args)...);
                }
            }
            return data;
        }


        void Reset()
        {
            currentBlockPos = 0;
            availableBlocks.splice(availableBlocks.begin(), usedBlocks);
        }
        
        size_t TotalAllocated() const {
            size_t total = currentAllocSize;
            for (const auto& alloc : usedBlocks)
            {
                total += alloc.first;
            }
            for (const auto& alloc : availableBlocks)
            {
                total += alloc.first;
            }
            return total;
        }
        ~Arena()
        {
            for (auto& usedBlock : usedBlocks)
            {
                free(usedBlock.second);
            }
            for (auto& availableBlock : availableBlocks)
            {
                free(availableBlock.second);
            }
        }
        
        size_t maxBlockSize = 0;
        size_t currentBlockPos = 0;
        size_t currentAllocSize = 0;
        uint8_t* currentBlock;
        std::list<std::pair<size_t, uint8_t*>> usedBlocks;
        std::list<std::pair<size_t, uint8_t*>> availableBlocks;
    };
}

#endif //MEMORY_HPP
