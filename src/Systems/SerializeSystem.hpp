//
// Created by carlo on 2025-02-24.
//

#ifndef SERIALIZE_SYSTEM_HPP
#define SERIALIZE_SYSTEM_HPP

namespace systems
{
template <typename value_type>
struct i_serializable
{
	virtual ~i_serializable()                                 = default;
	virtual nlohmann::json serialize()                       = 0;
	virtual value_type             *deserialize(std::string filename) = 0;
};
}        // namespace systems

#endif        // SERIALIZE_SYSTEM_HPP
