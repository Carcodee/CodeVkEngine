//

// Created by carlo on 2024-11-22.
//

#ifndef SINGLETONSTORAGE_HPP
#define SINGLETONSTORAGE_HPP

namespace SYSTEMS{

    class SingletonStorage
    {
    public:

    template<typename T>
    T* Get()
    {
        std::type_index typeIndex(typeid(T));
        auto it = storage.find(typeIndex);
        if (it != storage.end())
        {
            return std::any_cast<T*>(it->second);
        }

        auto insert = storage.emplace(typeIndex, std::make_any<T>());

        return std::any_cast<T*>(insert.first->second);
    }
        SingletonStorage(const SingletonStorage&)=delete;
        SingletonStorage operator=(const SingletonStorage&)=delete;

        static SingletonStorage* instance()
        {
            static SingletonStorage instance;
            return &instance;
        }

    private:
        SingletonStorage() = default;
        std::unordered_map<std::type_index, std::any> storage;
    };

}

#endif //SINGLETONSTORAGE_HPP
