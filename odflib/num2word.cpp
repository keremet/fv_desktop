#include <string>
#include <iostream>
#include "num2word.h"

#include <cctype>

namespace string_functions
{
    static const std::string minus_sign = "минус";
    static const size_t max_number_categories = 12;
    static const std::string number_categories[][3] = {
		{"тысяча", "тысячи", "тысяч"},
		{"миллион",  "миллиона", "миллионов"},
			{"миллиард", "миллиарда", "миллиардов"}
    };
    static const std::string low_numbers[][10] = {
		{"ноль", "один", "два", "три", "четыре", "пять", "шесть", "семь", "восемь", "девять"},
		{"десять", "одиннадцать", "двенадцать", "тринадцать", "четырнадцать", "пятнадцать", "шестнадцать", "семнадцать", "восемнадцать", "девятнадцать"},
		{"", "", "двадцать", "тридцать", "сорок", "пятьдесят", "шестьдесят", "семьдесят", "восемьдесят", "девяносто"},
		{"", "сто", "двести", "триста", "четыреста", "пятьсот", "шестьсот", "семьсот", "восемьсот", "десятьсот"},
			{"", "одна", "две", low_numbers[0][3], low_numbers[0][4], low_numbers[0][5], low_numbers[0][6], low_numbers[0][7], low_numbers[0][8], low_numbers[0][9]}
    };

    std::string tripple_in_words(std::string::const_iterator start, int trippleNo)
    {
		std::string res;
		bool hasSpace = false;

		if ( !isdigit(*start) || !isdigit(*(start+1)) || !isdigit(*(start+2)))
			throw Exception("Entered number is invalid");

		int digit = *start++ - '0';
		if (digit != 0)
			res += low_numbers[3][digit] + " ";

		digit = *start++ - '0';
		if (digit == 1)
		{
			digit = *start - '0';
			return res += low_numbers[1][digit];
		}
		else if (digit != 0)
			res += low_numbers[2][digit] + " ";

		digit = *start - '0';
		if (digit != 0)
			res += low_numbers[ trippleNo == 2 ? 4 : 0 ][digit] + " ";

		return res.substr(0, res.length() - 1);
    }

    std::string number_in_words(const std::string& number)
    {
		if (number.length() > max_number_categories)
			throw Exception("Number is too large for conversion");

		std::string num(number);
		std::string result;

		if (num[0] == '-')
		{
			result = minus_sign;
			num = num.substr(1);
		}

		while (num.length() % 3 != 0)
			num.insert(num.begin(), '0');

		size_t numTripples = num.length() / 3;
		std::string::const_iterator i;

		for (i=num.begin(); i!=num.end(); i+=3, --numTripples)
		{
			bool trippleIsNull = *i=='0' && *(i+1)=='0' && *(i+2)=='0';
			bool trippleHasTeenTens = *(i+1) == '1';

			if(!result.empty())
				result += " ";
			result += tripple_in_words(i, numTripples);

			if (numTripples > 1 && !trippleIsNull)
			{
			result += " ";

			if (trippleHasTeenTens)
				result += number_categories[numTripples-2][2];
			else
			{
				switch ( *(i+2))
				{
					case '1':
						result += number_categories[numTripples-2][0];
						break;
					case '2': case '3': case '4':
						result += number_categories[numTripples-2][1];
						break;
					case '0': case '5': case '6': case '7': case '8': case '9':
						result += number_categories[numTripples-2][2];
						break;
					default:
						throw Exception("Entered number is invalid");
				}
			}

//			result += " ";
			}
		}

		if (result.empty() || result == minus_sign)
			return low_numbers[0][0];

		return (result[result.length()-1]!=' ')?result:(result.substr(0, result.length() - 1));
    }
};
/*
int main(int argc, char** argv)
{
    std::string in;

    try
    {
	if (argc == 1)
	{
	    std::cout << "Enter number: ";
	    std::cin >> in;

	    std::cout << in << " in words: " << string_functions::number_in_words(in) << std::endl;
	}
	else
	{
	    for (char** p = argv+1; *p!=0; ++p)
	    {
		in = *p;
		std::cout << in << " in words: " << string_functions::number_in_words( std::string(in) ) << std::endl;
	    }
	}
    }
    catch (std::exception& e)
    {
	std::cerr << in << ": " << e.what() << std::endl;
	return 1;
    }

    return 0;
}
*/
