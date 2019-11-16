/**
 * Utility.hpp - Definitions, common macros and utility functions
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
 * @author: Nov 2019 - Mikee47 <mike@sillyhouse.net>
 *
 ****/

#pragma once

#include "config.hpp"

/**
 * @brief Wrap a type declaration so it can be passed with commas in it
 * @note Example:
 *
 *	template <typename ElementType, size_t Columns>
 *	struct MultiRow
 *	{
 *		ElementType values[Columns];
 *	}
 *
 * These fail:
 *
 * 	DECLARE_FSTR_ARRAY(myArray, MultiRow<double, 3>);
 * 	DECLARE_FSTR_ARRAY(myArray, (MultiRow<double, 3>));
 *
 * Use DECL like this:
 *
 * 	DECLARE_FSTR_ARRAY(myArray, DECL((MultiRow<double, 3>)) );
 *
 * Although for this example we should probably do this:
 *
 * 	using MultiRow_double_3 = MultiRow<double, 3>;
 * 	DECLARE_FSTR_ARRAY(myArray, MultiRow_double_3);
 *
 */
#define DECL(t) argument_type<void(t)>::type
template <typename T> struct argument_type;
template <typename T, typename U> struct argument_type<T(U)> {
	typedef U type;
};

/**
 * @brief Define a reference to an object
 * @param name Name for reference
 * @param ObjectType Fully qualified typename of object required, e.g. FSTR::String, FlashString, FSTR::Vector<int>, etc.
 * @param object FSTR::Object instance to cast
 */
#define DEFINE_FSTR_REF(name, ObjectType, object) constexpr const ObjectType& name PROGMEM = object.as<ObjectType>();
#define DEFINE_FSTR_REF_LOCAL(name, ObjectType, object) static DEFINE_FSTR_REF(name, ObjectType, object)

#define DEFINE_FSTR_REF_NAMED(name, ObjectType) DEFINE_FSTR_REF(name, ObjectType, FSTR_DATA_NAME(name).object);

/**
 * @brief Provide internal name for generated flash string structures
 */
#define FSTR_DATA_NAME(name) fstr_data_##name

/**
 * @brief Check structure is POD-compliant and correctly aligned
 */
#define FSTR_CHECK_STRUCT(name)                                                                                        \
	static_assert(std::is_pod<decltype(name)>::value, "FSTR structure not POD");                                       \
	static_assert(offsetof(decltype(name), data) == sizeof(uint32_t), "FSTR structure alignment error");

/**
 * @brief Link the contents of a file
 * @note We need inline assembler's `.incbin` instruction to actually import the data.
 * We use a macro STR() so that if required the name can be resolved from a #defined value.
 * @note This provides a more efficient way to read constant (read-only) file data.
 * The file content is bound into firmware image at link time.
 * @note The String object must be referenced or the linker won't emit it.
 * @note Use PROJECT_DIR to locate files in your project's source tree:
 *  		IMPORT_FSTR_DATA(myFlashData, PROJECT_DIR "/files/my_flash_file.txt");
 * Use COMPONENT_PATH within a component.
 * @note No C/C++ symbol is declared, this is type-dependent and must be done separately:
 * 			extern "C" FlashString myFlashData;
 */
#define STR(x) XSTR(x)
#define XSTR(x) #x
#ifdef __WIN32
#define IMPORT_FSTR_DATA(name, file)                                                                                   \
	__asm__(".section .rodata\n"                                                                                       \
			".global _" STR(name) "\n"                                                                                 \
			".def _" STR(name) "; .scl 2; .type 32; .endef\n"                                                          \
			".align 4\n"                                                                                               \
			"_" STR(name) ":\n"                                                                                        \
			".long _" STR(name) "_end - _" STR(name) " - 4\n"                                                          \
			".incbin \"" file "\"\n"                                                                                   \
			"_" STR(name) "_end:\n");
#else
#ifdef ARCH_HOST
#define IROM_SECTION ".rodata"
#else
#define IROM_SECTION ".irom0.text"
#endif
#define IMPORT_FSTR_DATA(name, file)                                                                                   \
	__asm__(".section " IROM_SECTION "\n"                                                                              \
			".global " STR(name) "\n"                                                                                  \
			".type " STR(name) ", @object\n"                                                                           \
			".align 4\n" STR(name) ":\n"                                                                               \
			".long _" STR(name) "_end - " STR(name) " - 4\n"                                                           \
			".incbin \"" file "\"\n"                                                                                   \
			"_" STR(name) "_end:\n");
#endif

namespace FSTR
{
template <typename T> FSTR_INLINE typename std::enable_if<sizeof(T) == 1, T>::type readValue(const T* ptr)
{
	return static_cast<T>(pgm_read_byte(ptr));
}

template <typename T> FSTR_INLINE typename std::enable_if<sizeof(T) == 2, T>::type readValue(const T* ptr)
{
	return static_cast<T>(pgm_read_word(ptr));
}

template <typename T> FSTR_INLINE typename std::enable_if<IS_ALIGNED(sizeof(T)), T>::type readValue(const T* ptr)
{
	return *static_cast<const T*>(ptr);
}

} // namespace FSTR
