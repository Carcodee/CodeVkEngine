//
// Created by carlo on 2025-01-26.
//

#ifndef MEMORY_HPP
#define MEMORY_HPP

#define IS_WINDOWS

namespace systems
{
#ifndef L1_CACHE_LINE_SIZE
#	define L1_CACHE_LINE_SIZE 64
#endif
void *Alloc_Aligned(size_t size)
{
#if defined(IS_WINDOWS)
	return _aligned_malloc(size, L1_CACHE_LINE_SIZE);
#elif defined(IS_OPENBSD) || defined(IS_OSX)
	void *ptr;
	if (posix_memalign(&ptr, L1_CACHE_LINE_SIZE, size) != 0)
		ptr = nullptr;
	return ptr;
#endif
	return malloc(size);
}

//     template<typename T>
//     void* FreeAligned(T* data)
//     {
// #if defined(IS_WINDOWS)
//         return _aligned_free(data);
// #elif defined (IS_OPENBSD) || defined(IS_OSX)
//         void *ptr;
//         if (posix_memalign(&ptr, L1_CACHE_LINE_SIZE, size) != 0)
//             ptr = nullptr;
//         return ptr;
// #endif
//         return free(data);
//     }
template <typename Value_Type>
Value_Type *Alloc_Aligned(size_t count)
{
	return (Value_Type *) Alloc_Aligned(count * sizeof(Value_Type));
}
class Arena
{
  public:
	Arena(size_t size = 262144)
	{
		max_block_size    = size;
		current_block_pos = 0;
	}

	void *Alloc(size_t size)
	{
		size_t data_size = size;
		if (current_block_pos + data_size > current_alloc_size)
		{
			if (current_block)
			{
				used_blocks.push_back(std::make_pair(current_alloc_size, current_block));
				current_block = nullptr;
			}
			for (auto iter = available_blocks.begin(); iter != available_blocks.end(); iter++)
			{
				if (iter->first >= data_size)
				{
					current_alloc_size = iter->first;
					current_block     = iter->second;
					available_blocks.erase(iter);
					break;
				}
			}
			if (!current_block)
			{
				current_alloc_size = std::max(data_size, max_block_size);
				current_block     = (uint8_t *) Alloc_Aligned(current_alloc_size);
			}
			current_block_pos = 0;
		}
		size_t alignment = 16;
		data_size         = (data_size + alignment - 1) & ~(alignment - 1);
		void *data       = current_block + current_block_pos;
		current_block_pos += data_size;
		return data;
	}

	template <typename Value_Type, typename... Argument_Types>
	Value_Type *Alloc(size_t n = 1, bool run_constructor = true, Argument_Types &&...args)
	{
		Value_Type *data = (Value_Type *) Alloc(n * sizeof(Value_Type));
		if (run_constructor)
		{
			for (int i = 0; i < n; ++i)
			{
				new (&data[i]) Value_Type(std::forward<Argument_Types>(args)...);
				destructors.push_back([ptr = &data[i]] { ptr->~Value_Type(); });
			}
		}
		return data;
	}

	void Reset()
	{
		for (auto &destructor : destructors)
		{
			destructor();
		}
		destructors.clear();
		current_block_pos = 0;
		available_blocks.splice(available_blocks.begin(), used_blocks);
	}

	size_t Total_Allocated() const
	{
		size_t total = current_alloc_size;
		for (const auto &alloc : used_blocks)
		{
			total += alloc.first;
		}
		for (const auto &alloc : available_blocks)
		{
			total += alloc.first;
		}
		return total;
	}

	~Arena()
	{
		for (auto &destructor : destructors)
		{
			destructor();
		}
		destructors.clear();

		_aligned_free(current_block);

		for (auto &used_block : used_blocks)
		{
			_aligned_free(used_block.second);
		}
		for (auto &available_block : available_blocks)
		{
			_aligned_free(available_block.second);
		}
	}

	size_t                                  max_block_size     = 0;
	size_t                                  current_block_pos  = 0;
	size_t                                  current_alloc_size = 0;
	uint8_t                                *current_block     = nullptr;
	std::list<std::pair<size_t, uint8_t *>> used_blocks;
	std::list<std::pair<size_t, uint8_t *>> available_blocks;
	std::vector<std::function<void()>>      destructors;
};
}        // namespace systems

#endif        // MEMORY_HPP
