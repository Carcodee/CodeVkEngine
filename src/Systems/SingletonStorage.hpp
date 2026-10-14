//

// Created by carlo on 2024-11-22.
//

#ifndef SINGLETON_STORAGE_HPP
#define SINGLETON_STORAGE_HPP

namespace systems
{

class singleton_storage
{
  public:
	template <typename value_type>
	value_type *get()
	{
		std::type_index type_index(typeid(value_type));
		auto            it = storage.find(type_index);
		if (it != storage.end())
		{
			return std::any_cast<value_type *>(it->second);
		}

		auto insert = storage.emplace(type_index, std::make_any<value_type>());

		return std::any_cast<value_type *>(insert.first->second);
	}
	singleton_storage(const singleton_storage &)           = delete;
	singleton_storage operator=(const singleton_storage &) = delete;

	static singleton_storage *instance()
	{
		static singleton_storage instance;
		return &instance;
	}

  private:
	singleton_storage() = default;
	std::unordered_map<std::type_index, std::any> storage;
};

}        // namespace systems

#endif        // SINGLETON_STORAGE_HPP
