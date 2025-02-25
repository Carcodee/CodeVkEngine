//
// Created by carlo on 2025-02-24.
//

#ifndef SERIALIZESYSTEM_HPP
#define SERIALIZESYSTEM_HPP

namespace SYSTEMS
{
    template <typename T>
    struct ISerializable{
        virtual ~ISerializable() = default;
        virtual std::string Serialize(std::string filename) = 0;
        virtual T Deserialize(std::string filename) = 0;
    };
    
}

#endif //SERIALIZESYSTEM_HPP
