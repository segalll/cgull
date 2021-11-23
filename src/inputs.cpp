#include "inputs.h"

#include <numeric>
#include <functional>

namespace cgull {
	namespace key {
        key_combination seq(std::initializer_list<key_mods> mods, key_code key) {
        	unsigned int m = std::transform_reduce(
        		mods.begin(), mods.end(),
        		0, std::bit_or<unsigned int>{},
        		[](key_mods km){return static_cast<unsigned int>(km);}
        	);
        	return key_combination(key, m);
        }

		key_combination seq(key_code key) {
			return key_combination(key, key_mods::none);
		}
	}
}