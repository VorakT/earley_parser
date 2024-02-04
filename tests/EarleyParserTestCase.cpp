#include "../earley_parser/EarleyParser.h"
#include <gtest/gtest.h>


TEST(ParserTestCase, FitExceptionTest) {
	EarleyParser parser;
	ASSERT_THROW(parser.fit("SABC", "abc", {"S - AB"}, 'S'), std::invalid_argument);
	ASSERT_THROW(parser.fit("SABC", "abc", {"SS -> AB"}, 'S'), std::invalid_argument);
	ASSERT_THROW(parser.fit("SABC", "abc", {"a -> AB"}, 'S'), std::invalid_argument);
	ASSERT_THROW(parser.fit("SABC", "abc", {"S -> d"}, 'S'), std::invalid_argument);
	ASSERT_THROW(parser.fit("SABC", "abc", {"S -> AB"}, 'a'), std::invalid_argument);
}


TEST(ParserTestCase, CheckPredictTest) {
	EarleyParser parser;
	parser.fit("S", "ab", {"S-> aSbS", "S ->"}, 'S');
	ASSERT_EQ(parser.predict(""), true);
	ASSERT_EQ(parser.predict("aababb"), true);
	ASSERT_EQ(parser.predict("aaabbb"), true);
	ASSERT_EQ(parser.predict("ababab"), true);
	ASSERT_EQ(parser.predict("aabbba"), false);
	ASSERT_EQ(parser.predict("aabbabaab"), false);
	ASSERT_EQ(parser.predict("aaa"), false);
	EarleyParser parser2;
	parser2.fit("AB", "abc", {"B -> baabA", "B -> A", "B -> cBcb", "B ->", "B -> Bac", "A -> A", "B -> bBBBc", "A -> aBbAAc"}, 'A');
	ASSERT_EQ(parser2.predict("abca"), false);
	ASSERT_EQ(parser2.predict("acaaab"), false);
	ASSERT_EQ(parser2.predict("bbbaaaaaaabbbbbbbbbbbbbbb"), false);
}
