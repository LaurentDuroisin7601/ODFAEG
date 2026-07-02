module;
#include <algorithm>
#include <locale>
#include <string>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <iostream>
export module odfaeg.core.string;
import odfaeg.core.utf;


export namespace odfaeg {
    namespace core {
        class String
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
            inline static const std::size_t InvalidPos = std::basic_string<std::uint32_t>::npos; ///< Represents an invalid position in the string

            ////////////////////////////////////////////////////////////
            String()
            {
            }


            ////////////////////////////////////////////////////////////
            String(char ansiChar, const std::locale& locale = std::locale())
            {
                m_string += Utf32::decodeAnsi(ansiChar, locale);
            }


            ////////////////////////////////////////////////////////////
            String(wchar_t wideChar)
            {
                m_string += Utf32::decodeWide(wideChar);
            }


            ////////////////////////////////////////////////////////////
            String(std::uint32_t utf32Char)
            {
                m_string += utf32Char;
            }


            ////////////////////////////////////////////////////////////
            String(const char* ansiString, const std::locale& locale = std::locale())
            {
                if (ansiString)
                {
                    std::size_t length = strlen(ansiString);
                    if (length > 0)
                    {
                        m_string.reserve(length + 1);
                        Utf32::fromAnsi(ansiString, ansiString + length, std::back_inserter(m_string), locale);
                    }
                }
            }


            ////////////////////////////////////////////////////////////
            String(const std::string& ansiString, const std::locale& locale = std::locale())
            {
                m_string.reserve(ansiString.length() + 1);
                Utf32::fromAnsi(ansiString.begin(), ansiString.end(), std::back_inserter(m_string), locale);
            }


            ////////////////////////////////////////////////////////////
            String(const wchar_t* wideString)
            {
                if (wideString)
                {
                    std::size_t length = std::wcslen(wideString);
                    if (length > 0)
                    {
                        m_string.reserve(length + 1);
                        Utf32::fromWide(wideString, wideString + length, std::back_inserter(m_string));
                    }
                }
            }


            ////////////////////////////////////////////////////////////
            String(const std::wstring& wideString)
            {
                m_string.reserve(wideString.length() + 1);
                Utf32::fromWide(wideString.begin(), wideString.end(), std::back_inserter(m_string));
            }


            ////////////////////////////////////////////////////////////
            String(const std::uint32_t* utf32String)
            {
                if (utf32String)
                    m_string = utf32String;
            }


            ////////////////////////////////////////////////////////////
            String(const std::basic_string<std::uint32_t>& utf32String) :
                m_string(utf32String)
            {
            }


            ////////////////////////////////////////////////////////////
            String(const String& copy) :
                m_string(copy.m_string)
            {
            }


            ////////////////////////////////////////////////////////////
            operator std::string() const
            {
                return toAnsiString();
            }


            ////////////////////////////////////////////////////////////
            operator std::wstring() const
            {
                return toWideString();
            }


            ////////////////////////////////////////////////////////////
            std::string toAnsiString(const std::locale& locale = std::locale()) const
            {
                // Prepare the output string
                //std::cout<<"length : "<<m_string.length()<<std::endl;
                std::string output;
                output.reserve(m_string.length() + 1);

                // Convert
                Utf32::toAnsi(m_string.begin(), m_string.end(), std::back_inserter(output), 0, locale);

                return output;
            }


            ////////////////////////////////////////////////////////////
            std::wstring toWideString() const
            {
                // Prepare the output string
                std::wstring output;
                output.reserve(m_string.length() + 1);

                // Convert
                Utf32::toWide(m_string.begin(), m_string.end(), std::back_inserter(output), 0);

                return output;
            }


            ////////////////////////////////////////////////////////////
            std::basic_string<std::uint8_t> toUtf8() const
            {
                // Prepare the output string
                std::basic_string<std::uint8_t> output;
                output.reserve(m_string.length());

                // Convert
                Utf32::toUtf8(m_string.begin(), m_string.end(), std::back_inserter(output));

                return output;
            }


            ////////////////////////////////////////////////////////////
            std::basic_string<std::uint16_t> toUtf16() const
            {
                // Prepare the output string
                std::basic_string<std::uint16_t> output;
                output.reserve(m_string.length());

                // Convert
                Utf32::toUtf16(m_string.begin(), m_string.end(), std::back_inserter(output));

                return output;
            }


            ////////////////////////////////////////////////////////////
            std::basic_string<std::uint32_t> toUtf32() const
            {
                return m_string;
            }


            ////////////////////////////////////////////////////////////
            String& operator =(const String& right)
            {
                m_string = right.m_string;
                return *this;
            }


            ////////////////////////////////////////////////////////////
            String& operator +=(const String& right)
            {
                m_string += right.m_string;
                return *this;
            }


            ////////////////////////////////////////////////////////////
            std::uint32_t operator [](std::size_t index) const
            {
                return m_string[index];
            }


            ////////////////////////////////////////////////////////////
            std::uint32_t& operator [](std::size_t index)
            {
                return m_string[index];
            }


            ////////////////////////////////////////////////////////////
            void clear()
            {
                m_string.clear();
            }


            ////////////////////////////////////////////////////////////
            std::size_t getSize() const
            {
                return m_string.size();
            }


            ////////////////////////////////////////////////////////////
            bool isEmpty() const
            {
                return m_string.empty();
            }


            ////////////////////////////////////////////////////////////
            void erase(std::size_t position, std::size_t count)
            {
                m_string.erase(position, count);
            }


            ////////////////////////////////////////////////////////////
            void insert(std::size_t position, const String& str)
            {
                m_string.insert(position, str.m_string);
            }


            ////////////////////////////////////////////////////////////
            std::size_t find(const String& str, std::size_t start=0) const
            {
                return m_string.find(str.m_string, start);
            }


            ////////////////////////////////////////////////////////////
            void replace(std::size_t position, std::size_t length, const String& replaceWith)
            {
                m_string.replace(position, length, replaceWith.m_string);
            }


            ////////////////////////////////////////////////////////////
            void replace(const String& searchFor, const String& replaceWith)
            {
                std::size_t step = replaceWith.getSize();
                std::size_t len = searchFor.getSize();
                std::size_t pos = find(searchFor);

                // Replace each occurrence of search
                while (pos != InvalidPos)
                {
                    replace(pos, len, replaceWith);
                    pos = find(searchFor, pos + step);
                }
            }


            ////////////////////////////////////////////////////////////
            String substring(std::size_t position, std::size_t length) const
            {
                return m_string.substr(position, length);
            }


            ////////////////////////////////////////////////////////////
            const std::uint32_t* getData() const
            {
                return m_string.c_str();
            }


            ////////////////////////////////////////////////////////////
            String::Iterator begin()
            {
                return m_string.begin();
            }


            ////////////////////////////////////////////////////////////
            String::ConstIterator begin() const
            {
                return m_string.begin();
            }


            ////////////////////////////////////////////////////////////
            String::Iterator end()
            {
                return m_string.end();
            }


            ////////////////////////////////////////////////////////////
            String::ConstIterator end() const
            {
                return m_string.end();
            }

            template <typename T>
            static String fromUtf8(T begin, T end)
            {
                String string;
                Utf8::toUtf32(begin, end, std::back_inserter(string.m_string));
                return string;
            }


            ////////////////////////////////////////////////////////////
            template <typename T>
            static String fromUtf16(T begin, T end)
            {
                String string;
                Utf16::toUtf32(begin, end, std::back_inserter(string.m_string));
                return string;
            }


            ////////////////////////////////////////////////////////////
            template <typename T>
            static String fromUtf32(T begin, T end)
            {
                String string;
                string.m_string.assign(begin, end);
                return string;
            }
            

        private:

            friend bool operator==(const String& left, const String& right);
            friend bool operator<(const String& left, const String& right);

            ////////////////////////////////////////////////////////////
            // Member data
            ////////////////////////////////////////////////////////////
            std::basic_string<std::uint32_t> m_string; ///< Internal string of UTF-32 characters
        };

        ////////////////////////////////////////////////////////////
        bool operator ==(const String& left, const String& right)
        {
            return left.m_string == right.m_string;
        }


        ////////////////////////////////////////////////////////////
        bool operator !=(const String& left, const String& right)
        {
            return !(left == right);
        }


        ////////////////////////////////////////////////////////////
        bool operator <(const String& left, const String& right)
        {
            return left.m_string < right.m_string;
        }


        ////////////////////////////////////////////////////////////
        bool operator >(const String& left, const String& right)
        {
            return right < left;
        }


        ////////////////////////////////////////////////////////////
        bool operator <=(const String& left, const String& right)
        {
            return !(right < left);
        }


        ////////////////////////////////////////////////////////////
        bool operator >=(const String& left, const String& right)
        {
            return !(left < right);
        }


        ////////////////////////////////////////////////////////////
        String operator +(const String& left, const String& right)
        {
            String string = left;
            string += right;

            return string;
        }
    }
}