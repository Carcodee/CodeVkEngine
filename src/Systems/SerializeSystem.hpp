//
// Created by carlo on 2025-02-24.
//

#ifndef SERIALIZE_SYSTEM_HPP
#define SERIALIZE_SYSTEM_HPP

namespace systems
{
template <typename Value_Type>
struct I_Serializable
{
	virtual ~I_Serializable()                                 = default;
	virtual nlohmann::json Serialize()                       = 0;
	virtual Value_Type             *Deserialize(std::string filename) = 0;
};
}        // namespace systems

#endif        // SERIALIZE_SYSTEM_HPP
