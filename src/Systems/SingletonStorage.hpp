//

// Created by carlo on 2024-11-22.
//

#ifndef SINGLETON_STORAGE_HPP
#define SINGLETON_STORAGE_HPP

namespace systems
{

class Singleton_Storage
{
  public:
	template <typename Value_Type>
	Value_Type *Get()
	{
		std::type_index type_index(typeid(Value_Type));
		auto            it = storage.find(type_index);
		if (it != storage.end())
		{
			return std::any_cast<Value_Type *>(it->second);
		}

		auto insert = storage.emplace(type_index, std::make_any<Value_Type>());

		return std::any_cast<Value_Type *>(insert.first->second);
	}
	Singleton_Storage(const Singleton_Storage &)           = delete;
	Singleton_Storage operator=(const Singleton_Storage &) = delete;

	static Singleton_Storage *Instance()
	{
		static Singleton_Storage instance;
		return &instance;
	}

  private:
	Singleton_Storage() = default;
	std::unordered_map<std::type_index, std::any> storage;
};

}        // namespace systems

#endif        // SINGLETON_STORAGE_HPP
