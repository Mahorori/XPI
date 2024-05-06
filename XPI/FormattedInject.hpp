#ifndef CFORMATTED_INJECT_HPP_
#define CFORMATTED_INJECT_HPP_

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_uint.hpp>
#include <boost/phoenix/stl/container.hpp>
#include <boost/phoenix/bind/bind_function.hpp>

#include <string>
#include <vector>

struct COutPacket;

namespace FormattedInject
{
	VOID DoInitPacket(COutPacket *oPacket, BOOL bHidden, WORD n);
	VOID DoEncode1(COutPacket *oPacket, BOOL bHidden, BYTE n);
	VOID DoEncode2(COutPacket *oPacket, BOOL bHidden, WORD n);
	VOID DoEncode4(COutPacket *oPacket, BOOL bHidden, DWORD n);
	VOID DoEncode8(COutPacket *oPacket, BOOL bHidden, ULONGLONG n);
	VOID DoEncodeString(COutPacket *oPacket, BOOL bHidden, std::string& s);
	VOID DoEncodeBuffer(COutPacket *oPacket, BOOL bHidden, std::vector<BYTE>& vb);
}

template <typename Iterator>
struct formatted_packet_grammar : boost::spirit::qi::grammar<Iterator, boost::spirit::ascii::space_type>
{
	formatted_packet_grammar(COutPacket *oPacket, BOOL bHide) : oPacket(oPacket), bHidden(bHide), formatted_packet_grammar::base_type(start)
	{
		QUOTED_STRING_ %= boost::spirit::qi::lexeme['"' >> *(boost::spirit::ascii::char_ - '"') >> '"'];
		ARRAY_ %= '[' >> boost::spirit::qi::lexeme[+BYTE_] >> ']';

		start =
			WORD_[boost::phoenix::bind(&FormattedInject::DoInitPacket, oPacket, bHidden, boost::spirit::_1)]
			>>
			boost::spirit::qi::repeat
			[
				QUOTED_STRING_[boost::phoenix::bind(&FormattedInject::DoEncodeString, oPacket, bHidden, boost::spirit::_1)]
				|
				ULONGLONG_[boost::phoenix::bind(&FormattedInject::DoEncode8, oPacket, bHidden, boost::spirit::_1)]
				|
				DWORD_[boost::phoenix::bind(&FormattedInject::DoEncode4, oPacket, bHidden, boost::spirit::_1)]
				|
				WORD_[boost::phoenix::bind(&FormattedInject::DoEncode2, oPacket, bHidden, boost::spirit::_1)]
				|
				BYTE_[boost::phoenix::bind(&FormattedInject::DoEncode1, oPacket, bHidden, boost::spirit::_1)]
				|
				ARRAY_[boost::phoenix::bind(&FormattedInject::DoEncodeBuffer, oPacket, bHidden, boost::spirit::_1)]
			];
	};

	boost::spirit::qi::rule<Iterator, boost::spirit::ascii::space_type> start;
	boost::spirit::qi::rule<Iterator, std::string(), boost::spirit::ascii::space_type> QUOTED_STRING_;
	boost::spirit::qi::rule<Iterator, std::vector<BYTE>(), boost::spirit::ascii::space_type> ARRAY_;
	boost::spirit::qi::uint_parser<ULONGLONG, 16, 16, 16> ULONGLONG_;
	boost::spirit::qi::uint_parser<DWORD, 16, 8, 8> DWORD_;
	boost::spirit::qi::uint_parser<WORD, 16, 4, 4> WORD_;
	boost::spirit::qi::uint_parser<BYTE, 16, 2, 2> BYTE_;

	COutPacket *oPacket;
	BOOL bHidden;
};

typedef std::string::const_iterator iterator_type;
typedef formatted_packet_grammar<iterator_type> formatted_packet_parser;

#endif // CFORMATTED_INJECT_HPP_