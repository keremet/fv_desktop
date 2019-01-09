#include <string>

#include <cctype>

namespace string_functions
{
    class Exception: public std::exception
    {
		public:
		Exception(const std::string& message): message_(message) {};
		~Exception() throw() {};

		virtual const char* what() const throw()
		{
			return message_.c_str();
		}

		virtual const std::string& message() const throw()
		{
			return message_;
		}

		private:
		std::string message_;
    };


    std::string number_in_words(const std::string& number);
};

