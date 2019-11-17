/**
 * ObjectBase.cpp
 *
 * Copyright 2019 mikee47 <mike@sillyhouse.net>
 *
 * This file is part of the FlashString Library
 *
 * This library is free software: you can redistribute it and/or modify it under the terms of the
 * GNU General Public License as published by the Free Software Foundation, version 3 or later.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with FlashString.
 * If not, see <https://www.gnu.org/licenses/>.
 *
 ****/

#include "include/FlashString/ObjectBase.hpp"
#include <esp_spi_flash.h>

namespace FSTR
{
const ObjectBase ObjectBase::empty_{0};
constexpr uint32_t ObjectBase::copyBit;

size_t ObjectBase::readFlash(size_t offset, void* buffer, size_t count) const
{
	auto ptr = getObjectPtr();
	auto len = getObjectLength(ptr);
	if(offset >= len) {
		return 0;
	}

	count = std::min(len - offset, count);
	auto addr = flashmem_get_address(getObjectData(ptr) + offset);
	return flashmem_read(buffer, addr, count);
}

const ObjectBase* ObjectBase::getObjectPtr() const
{
	const ObjectBase* ptr;
	if(isCopy()) {
		ptr = reinterpret_cast<const ObjectBase*>(flashLength_ & ~copyBit);
	} else if(flashLength_ == 0) {
		ptr = &empty_;
	} else {
#ifdef ARCH_HOST
		// Cannot yet differentiate memory addresses on Host
		ptr = this;
#else
		// Just in case this object was copied directly
		assert(isFlashPtr(this));
		if(isFlashPtr(this)) {
			ptr = this;
		} else {
			// In release code just return an empty object
			ptr = &empty_;
		}
#endif
	}
	return ptr;
}

} // namespace FSTR
