//
// Created by carlo on 2025-02-24.
//

#ifndef SERIALIZESYSTEM_HPP
#define SERIALIZESYSTEM_HPP

namespace SYSTEMS
{
template <typename T>
struct ISerializable
{
	virtual ~ISerializable()                                 = default;
	virtual nlohmann::json Serialize()                       = 0;
	virtual T             *Deserialize(std::string filename) = 0;
};
}        // namespace SYSTEMS

#endif        // SERIALIZESYSTEM_HPP
