#ifndef ODFAEG_STRING_HPP
#define ODFAEG_STRING_HPP
#include "export.hpp"
#include "utf.hpp"
namespace odfaeg {
    namespace core {
        class ODFAEG_CORE_API String
        {
        public:

            ////////////////////////////////////////////////////////////
            // Types
            ////////////////////////////////////////////////////////////
            typedef std::basic_string<std::uint32_t>::iterator       Iterator;      ///< Iterator type
            typedef std::basic_string<std::uint32_t>::const_iterator ConstIterator; ///< Read-only iterator type

            ////////////////////////////////////////////////////////////
            // Static member data
            ////////////////////////////////////////////////////////////
            static const std::size_t InvalidPos; ///< Represents an invalid position in the string

            ////////////////////////////////////////////////////////////
            /// \brief Default constructor
            ///
            /// This constructor creates an empty string.
            ///
            ////////////////////////////////////////////////////////////
            String();

            ////////////////////////////////////////////////////////////
            /// \brief Construct from a single ANSI character and a locale
            ///
            /// The source character is converted to UTF-32 according
            /// to the given locale.
            ///
            /// \param ansiChar ANSI character to convert
            /// \param locale   Locale to use for conversion
            ///
            ////////////////////////////////////////////////////////////
            String(char ansiChar, const std::locale& locale = std::locale());

            ////////////////////////////////////////////////////////////
            /// \brief Construct from single wide character
            ///
            /// \param wideChar Wide character to convert
            ///
            ////////////////////////////////////////////////////////////
            String(wchar_t wideChar);

            ////////////////////////////////////////////////////////////
            /// \brief Construct from single UTF-32 character
            ///
            /// \param utf32Char UTF-32 character to convert
            ///
            ////////////////////////////////////////////////////////////
            String(std::uint32_t utf32Char);

            ////////////////////////////////////////////////////////////
            /// \brief Construct from a null-terminated C-style ANSI string and a locale
            ///
            /// The source string is converted to UTF-32 according
            /// to the given locale.
            ///
            /// \param ansiString ANSI string to convert
            /// \param locale     Locale to use for conversion
            ///
            ////////////////////////////////////////////////////////////
            String(const char* ansiString, const std::locale& locale = std::locale());

            ////////////////////////////////////////////////////////////
            /// \brief Construct from an ANSI string and a locale
            ///
            /// The source string is converted to UTF-32 according
            /// to the given locale.
            ///
            /// \param ansiString ANSI string to convert
            /// \param locale     Locale to use for conversion
            ///
            ////////////////////////////////////////////////////////////
            String(const std::string& ansiString, const std::locale& locale = std::locale());

            ////////////////////////////////////////////////////////////
            /// \brief Construct from null-terminated C-style wide string
            ///
            /// \param wideString Wide string to convert
            ///
            ////////////////////////////////////////////////////////////
            String(const wchar_t* wideString);

            ////////////////////////////////////////////////////////////
            /// \brief Construct from a wide string
            ///
            /// \param wideString Wide string to convert
            ///
            ////////////////////////////////////////////////////////////
            String(const std::wstring& wideString);

            ////////////////////////////////////////////////////////////
            /// \brief Construct from a null-terminated C-style UTF-32 string
            ///
            /// \param utf32String UTF-32 string to assign
            ///
            ////////////////////////////////////////////////////////////
            String(const std::uint32_t* utf32String);

            ////////////////////////////////////////////////////////////
            /// \brief Construct from an UTF-32 string
            ///
            /// \param utf32String UTF-32 string to assign
            ///
            ////////////////////////////////////////////////////////////
            String(const std::basic_string<std::uint32_t>& utf32String);

            ////////////////////////////////////////////////////////////
            /// \brief Copy constructor
            ///
            /// \param copy Instance to copy
            ///
            ////////////////////////////////////////////////////////////
            String(const String& copy);

            ////////////////////////////////////////////////////////////
            /// \brief Create a new core::String from a UTF-8 encoded string
            ///
            /// \param begin Forward iterator to the beginning of the UTF-8 sequence
            /// \param end   Forward iterator to the end of the UTF-8 sequence
            ///
            /// \return A core::String containing the source string
            ///
            /// \see fromUtf16, fromUtf32
            ///
            ////////////////////////////////////////////////////////////
            template <typename T>
            static String fromUtf8(T begin, T end);

            ////////////////////////////////////////////////////////////
            /// \brief Create a new core::String from a UTF-16 encoded string
            ///
            /// \param begin Forward iterator to the beginning of the UTF-16 sequence
            /// \param end   Forward iterator to the end of the UTF-16 sequence
            ///
            /// \return A core::String containing the source string
            ///
            /// \see fromUtf8, fromUtf32
            ///
            ////////////////////////////////////////////////////////////
            template <typename T>
            static String fromUtf16(T begin, T end);

            ////////////////////////////////////////////////////////////
            /// \brief Create a new core::String from a UTF-32 encoded string
            ///
            /// This function is provided for consistency, it is equivalent to
            /// using the constructors that takes a const sf::std::uint32_t* or
            /// a std::basic_string<sf::std::uint32_t>.
            ///
            /// \param begin Forward iterator to the beginning of the UTF-32 sequence
            /// \param end   Forward iterator to the end of the UTF-32 sequence
            ///
            /// \return A core::String containing the source string
            ///
            /// \see fromUtf8, fromUtf16
            ///
            ////////////////////////////////////////////////////////////
            template <typename T>
            static String fromUtf32(T begin, T end);

            ////////////////////////////////////////////////////////////
            /// \brief Implicit conversion operator to std::string (ANSI string)
            ///
            /// The current global locale is used for conversion. If you
            /// want to explicitly specify a locale, see toAnsiString.
            /// Characters that do not fit in the target encoding are
            /// discarded from the returned string.
            /// This operator is defined for convenience, and is equivalent
            /// to calling toAnsiString().
            ///
            /// \return Converted ANSI string
            ///
            /// \see toAnsiString, operator std::wstring
            ///
            ////////////////////////////////////////////////////////////
            operator std::string() const;

            ////////////////////////////////////////////////////////////
            /// \brief Implicit conversion operator to std::wstring (wide string)
            ///
            /// Characters that do not fit in the target encoding are
            /// discarded from the returned string.
            /// This operator is defined for convenience, and is equivalent
            /// to calling toWideString().
            ///
            /// \return Converted wide string
            ///
            /// \see toWideString, operator std::string
            ///
            ////////////////////////////////////////////////////////////
            operator std::wstring() const;

            ////////////////////////////////////////////////////////////
            /// \brief Convert the Unicode string to an ANSI string
            ///
            /// The UTF-32 string is converted to an ANSI string in
            /// the encoding defined by \a locale.
            /// Characters that do not fit in the target encoding are
            /// discarded from the returned string.
            ///
            /// \param locale Locale to use for conversion
            ///
            /// \return Converted ANSI string
            ///
            /// \see toWideString, operator std::string
            ///
            ////////////////////////////////////////////////////////////
            std::string toAnsiString(const std::locale& locale = std::locale()) const;

            ////////////////////////////////////////////////////////////
            /// \brief Convert the Unicode string to a wide string
            ///
            /// Characters that do not fit in the target encoding are
            /// discarded from the returned string.
            ///
            /// \return Converted wide string
            ///
            /// \see toAnsiString, operator std::wstring
            ///
            ////////////////////////////////////////////////////////////
            std::wstring toWideString() const;

            ////////////////////////////////////////////////////////////
            /// \brief Convert the Unicode string to a UTF-8 string
            ///
            /// \return Converted UTF-8 string
            ///
            /// \see toUtf16, toUtf32
            ///
            ////////////////////////////////////////////////////////////
            std::basic_string<std::uint8_t> toUtf8() const;

            ////////////////////////////////////////////////////////////
            /// \brief Convert the Unicode string to a UTF-16 string
            ///
            /// \return Converted UTF-16 string
            ///
            /// \see toUtf8, toUtf32
            ///
            ////////////////////////////////////////////////////////////
            std::basic_string<std::uint16_t> toUtf16() const;

            ////////////////////////////////////////////////////////////
            /// \brief Convert the Unicode string to a UTF-32 string
            ///
            /// This function doesn't perform any conversion, since the
            /// string is already stored as UTF-32 internally.
            ///
            /// \return Converted UTF-32 string
            ///
            /// \see toUtf8, toUtf16
            ///
            ////////////////////////////////////////////////////////////
            std::basic_string<std::uint32_t> toUtf32() const;

            ////////////////////////////////////////////////////////////
            /// \brief Overload of assignment operator
            ///
            /// \param right Instance to assign
            ///
            /// \return Reference to self
            ///
            ////////////////////////////////////////////////////////////
            String& operator =(const String& right);

            ////////////////////////////////////////////////////////////
            /// \brief Overload of += operator to append an UTF-32 string
            ///
            /// \param right String to append
            ///
            /// \return Reference to self
            ///
            ////////////////////////////////////////////////////////////
            String& operator +=(const String& right);

            ////////////////////////////////////////////////////////////
            /// \brief Overload of [] operator to access a character by its position
            ///
            /// This function provides read-only access to characters.
            /// Note: the behavior is undefined if \a index is out of range.
            ///
            /// \param index Index of the character to get
            ///
            /// \return Character at position \a index
            ///
            ////////////////////////////////////////////////////////////
            std::uint32_t operator [](std::size_t index) const;

            ////////////////////////////////////////////////////////////
            /// \brief Overload of [] operator to access a character by its position
            ///
            /// This function provides read and write access to characters.
            /// Note: the behavior is undefined if \a index is out of range.
            ///
            /// \param index Index of the character to get
            ///
            /// \return Reference to the character at position \a index
            ///
            ////////////////////////////////////////////////////////////
            std::uint32_t& operator [](std::size_t index);

            ////////////////////////////////////////////////////////////
            /// \brief Clear the string
            ///
            /// This function removes all the characters from the string.
            ///
            /// \see isEmpty, erase
            ///
            ////////////////////////////////////////////////////////////
            void clear();

            ////////////////////////////////////////////////////////////
            /// \brief Get the size of the string
            ///
            /// \return Number of characters in the string
            ///
            /// \see isEmpty
            ///
            ////////////////////////////////////////////////////////////
            std::size_t getSize() const;

            ////////////////////////////////////////////////////////////
            /// \brief Check whether the string is empty or not
            ///
            /// \return True if the string is empty (i.e. contains no character)
            ///
            /// \see clear, getSize
            ///
            ////////////////////////////////////////////////////////////
            bool isEmpty() const;

            ////////////////////////////////////////////////////////////
            /// \brief Erase one or more characters from the string
            ///
            /// This function removes a sequence of \a count characters
            /// starting from \a position.
            ///
            /// \param position Position of the first character to erase
            /// \param count    Number of characters to erase
            ///
            ////////////////////////////////////////////////////////////
            void erase(std::size_t position, std::size_t count = 1);

            ////////////////////////////////////////////////////////////
            /// \brief Insert one or more characters into the string
            ///
            /// This function inserts the characters of \a str
            /// into the string, starting from \a position.
            ///
            /// \param position Position of insertion
            /// \param str      Characters to insert
            ///
            ////////////////////////////////////////////////////////////
            void insert(std::size_t position, const String& str);

            ////////////////////////////////////////////////////////////
            /// \brief Find a sequence of one or more characters in the string
            ///
            /// This function searches for the characters of \a str
            /// in the string, starting from \a start.
            ///
            /// \param str   Characters to find
            /// \param start Where to begin searching
            ///
            /// \return Position of \a str in the string, or String::InvalidPos if not found
            ///
            ////////////////////////////////////////////////////////////
            std::size_t find(const String& str, std::size_t start = 0) const;

            ////////////////////////////////////////////////////////////
            /// \brief Replace a substring with another string
            ///
            /// This function replaces the substring that starts at index \a position
            /// and spans \a length characters with the string \a replaceWith.
            ///
            /// \param position    Index of the first character to be replaced
            /// \param length      Number of characters to replace. You can pass InvalidPos to
            ///                    replace all characters until the end of the string.
            /// \param replaceWith String that replaces the given substring.
            ///
            ////////////////////////////////////////////////////////////
            void replace(std::size_t position, std::size_t length, const String& replaceWith);

            ////////////////////////////////////////////////////////////
            /// \brief Replace all occurrences of a substring with a replacement string
            ///
            /// This function replaces all occurrences of \a searchFor in this string
            /// with the string \a replaceWith.
            ///
            /// \param searchFor   The value being searched for
            /// \param replaceWith The value that replaces found \a searchFor values
            ///
            ////////////////////////////////////////////////////////////
            void replace(const String& searchFor, const String& replaceWith);

            ////////////////////////////////////////////////////////////
            /// \brief Return a part of the string
            ///
            /// This function returns the substring that starts at index \a position
            /// and spans \a length characters.
            ///
            /// \param position Index of the first character
            /// \param length   Number of characters to include in the substring (if
            ///                 the string is shorter, as many characters as possible
            ///                 are included). \ref InvalidPos can be used to include all
            ///                 characters until the end of the string.
            ///
            /// \return String object containing a substring of this object
            ///
            ////////////////////////////////////////////////////////////
            String substring(std::size_t position, std::size_t length = InvalidPos) const;

            ////////////////////////////////////////////////////////////
            /// \brief Get a pointer to the C-style array of characters
            ///
            /// This functions provides a read-only access to a
            /// null-terminated C-style representation of the string.
            /// The returned pointer is temporary and is meant only for
            /// immediate use, thus it is not recommended to store it.
            ///
            /// \return Read-only pointer to the array of characters
            ///
            ////////////////////////////////////////////////////////////
            const std::uint32_t* getData() const;

            ////////////////////////////////////////////////////////////
            /// \brief Return an iterator to the beginning of the string
            ///
            /// \return Read-write iterator to the beginning of the string characters
            ///
            /// \see end
            ///
            ////////////////////////////////////////////////////////////
            Iterator begin();

            ////////////////////////////////////////////////////////////
            /// \brief Return an iterator to the beginning of the string
            ///
            /// \return Read-only iterator to the beginning of the string characters
            ///
            /// \see end
            ///
            ////////////////////////////////////////////////////////////
            ConstIterator begin() const;

            ////////////////////////////////////////////////////////////
            /// \brief Return an iterator to the end of the string
            ///
            /// The end iterator refers to 1 position past the last character;
            /// thus it represents an invalid character and should never be
            /// accessed.
            ///
            /// \return Read-write iterator to the end of the string characters
            ///
            /// \see begin
            ///
            ////////////////////////////////////////////////////////////
            Iterator end();

            ////////////////////////////////////////////////////////////
            /// \brief Return an iterator to the end of the string
            ///
            /// The end iterator refers to 1 position past the last character;
            /// thus it represents an invalid character and should never be
            /// accessed.
            ///
            /// \return Read-only iterator to the end of the string characters
            ///
            /// \see begin
            ///
            ////////////////////////////////////////////////////////////
            ConstIterator end() const;

        private:

            friend ODFAEG_CORE_API bool operator ==(const String& left, const String& right);
            friend ODFAEG_CORE_API bool operator <(const String& left, const String& right);

            ////////////////////////////////////////////////////////////
            // Member data
            ////////////////////////////////////////////////////////////
            std::basic_string<std::uint32_t> m_string; ///< Internal string of UTF-32 characters
        };

        ////////////////////////////////////////////////////////////
        /// \relates String
        /// \brief Overload of == operator to compare two UTF-32 strings
        ///
        /// \param left  Left operand (a string)
        /// \param right Right operand (a string)
        ///
        /// \return True if both strings are equal
        ///
        ////////////////////////////////////////////////////////////
        ODFAEG_CORE_API bool operator ==(const String& left, const String& right);

        ////////////////////////////////////////////////////////////
        /// \relates String
        /// \brief Overload of != operator to compare two UTF-32 strings
        ///
        /// \param left  Left operand (a string)
        /// \param right Right operand (a string)
        ///
        /// \return True if both strings are different
        ///
        ////////////////////////////////////////////////////////////
        ODFAEG_CORE_API bool operator !=(const String& left, const String& right);

        ////////////////////////////////////////////////////////////
        /// \relates String
        /// \brief Overload of < operator to compare two UTF-32 strings
        ///
        /// \param left  Left operand (a string)
        /// \param right Right operand (a string)
        ///
        /// \return True if \a left is lexicographically before \a right
        ///
        ////////////////////////////////////////////////////////////
        ODFAEG_CORE_API bool operator <(const String& left, const String& right);

        ////////////////////////////////////////////////////////////
        /// \relates String
        /// \brief Overload of > operator to compare two UTF-32 strings
        ///
        /// \param left  Left operand (a string)
        /// \param right Right operand (a string)
        ///
        /// \return True if \a left is lexicographically after \a right
        ///
        ////////////////////////////////////////////////////////////
        ODFAEG_CORE_API bool operator >(const String& left, const String& right);

        ////////////////////////////////////////////////////////////
        /// \relates String
        /// \brief Overload of <= operator to compare two UTF-32 strings
        ///
        /// \param left  Left operand (a string)
        /// \param right Right operand (a string)
        ///
        /// \return True if \a left is lexicographically before or equivalent to \a right
        ///
        ////////////////////////////////////////////////////////////
        ODFAEG_CORE_API bool operator <=(const String& left, const String& right);

        ////////////////////////////////////////////////////////////
        /// \relates String
        /// \brief Overload of >= operator to compare two UTF-32 strings
        ///
        /// \param left  Left operand (a string)
        /// \param right Right operand (a string)
        ///
        /// \return True if \a left is lexicographically after or equivalent to \a right
        ///
        ////////////////////////////////////////////////////////////
        ODFAEG_CORE_API bool operator >=(const String& left, const String& right);

        ////////////////////////////////////////////////////////////
        /// \relates String
        /// \brief Overload of binary + operator to concatenate two strings
        ///
        /// \param left  Left operand (a string)
        /// \param right Right operand (a string)
        ///
        /// \return Concatenated string
        ///
        ////////////////////////////////////////////////////////////
        ODFAEG_CORE_API String operator +(const String& left, const String& right);
    }
}
#endif // ODFAEG_STRING_HPP
