#include "archive.hpp"
namespace odfaeg {
    namespace core {
        OTextArchive::OTextArchive(std::ostream& buffer) : buffer(buffer) {
            nbSerialized = 0;
        }
        /** \fn bool isInputArchive()
        *   \brief check if the archive is an input archive.
        *   \return false because it's an output archive.
        */
        bool OTextArchive::isInputArchive() {
            return false;
        }
        /** \fn void clear()
        *   \brief clear the output stream buffer and the registered pointer adresses.
        */
        void OTextArchive::clear() {
            buffer.clear();
            adresses.clear();
            nbSerialized = 0;
        }
        ITextArchive::ITextArchive(std::istream& buffer) : buffer(buffer) {
            nbDeserialized = 0;
        }
        /**
        * \fn bool isAtEndOfInputStream()
        * \brief check if all the data has been extracted from the archive.
        * \return if all the data has been extracted from the archive.
        */
        bool ITextArchive::isAtEndOfInputStream() {
            return buffer.eof();
        }
        /**
        *\fn bool isInputArchive()
        * \brief check if the archive is an input archive.
        * \return true because this is an input archive.
        */
        bool ITextArchive::isInputArchive() {
            return true;
        }
        /**
        * \fn void clear()
        * \brief clear the input stream buffer, and registered addresses.
        */
        void ITextArchive::clear() {
            buffer.clear();
            adresses.clear();
            nbDeserialized = 0;
        }
        /**
        * \fn void operator(T& data, D...)
        * \brief read a char from the archive. (we need to read unformatted input here to also read special chars like \n, spaces, etc...)
        * \param T& the data to read.
        * \param D... used for SFINAE.
        */

        void ITextArchive::operator() (char& data) {
            nbDeserialized++;
            ////////std::cout<<"read char : "<<data<<std::endl;
            buffer.get(data);
            ////////std::cout<<"data"<<std::endl;
            char space;
            buffer.get(space);
        }
        /**
        * \fn void operator(T& data, D...)
        * \brief read a char from the archive. (we need to read unformatted input here to also read special chars like \n, spaces, etc...)
        * \param std::reference_wrapper<char> reference to the data to read.
        * \param D... used for SFINAE.
        */

        void ITextArchive::operator() (std::reference_wrapper<char> ref) {
            char& data = ref.get();
            long long int id;
            buffer >> id;
            ////////std::cout<<"read reference to char : "<<id<<std::endl;
            char space;
            buffer.get(space);
            std::map<long long int, std::string>::iterator it = adresses.find(id);
            if (it != adresses.end()) {
                std::istringstream iss(it->second);
                std::vector<std::string> parts = split(iss.str(), "*");
                data = *reinterpret_cast<char*> (conversionStringULong(parts[1]));
            }
            else {
                std::ostringstream oss;
                oss << typeid(data).name() << "*" << reinterpret_cast<unsigned long long int>(&data);
                std::pair<long long int, std::string> newAddress(id, oss.str());
                adresses.insert(newAddress);
                nbDeserialized++;
                buffer.get(data);
                ////////std::cout<<"data : "<<data<<std::endl;
                char space;
                buffer.get(space);
            }
        }
        /**
        * \fn void operator(T& data, D...)
        * \brief read an unsigned char from the archive. (we need to read unformatted input here to also read special chars like \n, spaces, etc...)
        * \param T& the data to read.
        * \param D... used for SFINAE.
        */

        void ITextArchive::operator() (unsigned char& data) {
            nbDeserialized++;
            buffer.get((char&)data);
            ////////std::cout<<"read unsigned char : "<<data<<std::endl;
            char space;
            buffer.get(space);
        }
        /**
        * \fn void operator(T& data, D...)
        * \brief read an unsigned char from the archive. (we need to read unformatted input here to also read special chars like \n, spaces, etc...)
        * \param std::reference_wrapper<unsigned char> reference the data to read.
        * \param D... used for SFINAE.
        */

        void ITextArchive::operator() (std::reference_wrapper<unsigned char> ref) {
            unsigned char& data = ref.get();
            long long int id;
            buffer >> id;
            ////////std::cout<<"read reference to unsigned char : "<<id<<std::endl;
            char space;
            buffer.get(space);
            std::map<long long int, std::string>::iterator it = adresses.find(id);
            if (it != adresses.end()) {
                std::istringstream iss(it->second);
                std::vector<std::string> parts = split(iss.str(), "*");
                data = *reinterpret_cast<unsigned char*> (conversionStringULong(parts[1]));
            }
            else {
                std::ostringstream oss;
                oss << typeid(data).name() << "*" << reinterpret_cast<unsigned long long int>(&data);
                std::pair<long long int, std::string> newAddress(id, oss.str());
                adresses.insert(newAddress);
                nbDeserialized++;
                buffer.get((char&)data);
                ////////std::cout<<"data : "<<data<<std::endl;
                char space;
                buffer.get(space);
            }
        }
        void ITextArchive::operator() (char*& data) {
            long long int id;
            buffer >> id;
            char space;
            buffer.get(space);
            ////////std::cout<<"read pointer to char : "<<id<<std::endl;
            if (id != -1) {
                std::map<long long int, std::string>::iterator it = adresses.find(id);
                if (it != adresses.end()) {
                    std::istringstream iss(it->second);
                    std::vector<std::string> parts = split(iss.str(), "*");
                    data = reinterpret_cast<char*> (conversionStringULong(parts[1]));
                }
                else {
                    data = new char();
                    std::ostringstream oss;
                    oss << typeid(*data).name() << "*" << reinterpret_cast<unsigned long long int>(data);
                    std::pair<long long int, std::string> newAddress(id, oss.str());
                    adresses.insert(newAddress);
                    nbDeserialized++;
                    buffer >> (*data);
                    char space;
                    buffer.get(space);
                    ////////std::cout<<"char data : "<<(*data)<<std::endl;
                }
            }
            else {
                data = nullptr;
            }
        }
        /**
         * \fn void operator(T& data, D...)
         * \brief read a pointer to a fundamental type from the archive.
         * \param T& the data to read.
         * \param D... used for SFINAE.
         */
        void ITextArchive::operator() (unsigned char*& data) {
            long long int id;
            buffer >> id;
            char space;
            buffer.get(space);
            ////////std::cout<<"read pointer to unsigned char id : "<<id<<std::endl;
            if (id != -1) {
                std::map<long long int, std::string>::iterator it = adresses.find(id);
                if (it != adresses.end()) {
                    std::istringstream iss(it->second);
                    std::vector<std::string> parts = split(iss.str(), "*");
                    data = reinterpret_cast<unsigned char*> (conversionStringULong(parts[1]));
                }
                else {
                    data = new unsigned char();
                    std::ostringstream oss;
                    oss << typeid(*data).name() << "*" << reinterpret_cast<unsigned long long int>(data);
                    std::pair<long long int, std::string> newAddress(id, oss.str());
                    adresses.insert(newAddress);
                    nbDeserialized++;
                    buffer >> ((char&)*data);
                    char space;
                    buffer.get(space);
                    ////////std::cout<<"unsigned char data"<<(*data)<<std::endl;
                }
            }
            else {
                data = nullptr;
            }
        }
    }
}