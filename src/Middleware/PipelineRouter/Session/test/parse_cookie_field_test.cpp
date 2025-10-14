#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "../../../../Lib/StringOps/StringOps.cpp"
#include "../SessionMiddleware.cpp"

struct TestCase {
	std::string name;
	std::string input;
	std::map< std::string, std::string > expect;
};

static int failures = 0;

static void assertEqual(const std::string &name,
						const std::map< std::string, std::string > &got,
						const std::map< std::string, std::string > &expect) {
	bool ok = got == expect;
	if (!ok) {
		std::cerr << "[FAIL] " << name << "\n";
		std::cerr << "  Expected:" << std::endl;
		for (std::map< std::string, std::string >::const_iterator it =
				 expect.begin();
			 it != expect.end(); ++it) {
			std::cerr << "    {" << it->first << ": '" << it->second << "'}\n";
		}
		std::cerr << "  Got:" << std::endl;
		for (std::map< std::string, std::string >::const_iterator it =
				 got.begin();
			 it != got.end(); ++it) {
			std::cerr << "    {" << it->first << ": '" << it->second << "'}\n";
		}
		++failures;
	} else {
		std::cout << "[PASS] " << name << "\n";
	}
}

int main() {
	std::vector< TestCase > cases;

	// Simple single pair
	cases.push_back(TestCase());
	cases.back().name = "simple key=value";
	cases.back().input = "a=b";
	cases.back().expect["a"] = "b";

	// Multiple pairs with spaces and semicolons
	cases.push_back(TestCase());
	cases.back().name = "multiple pairs with spaces";
	cases.back().input = " foo=bar ; baz = qux ;z=1 ";
	cases.back().expect["foo"] = "bar";
	cases.back().expect["baz"] = "qux";
	cases.back().expect["z"] = "1";

	// Missing value
	cases.push_back(TestCase());
	cases.back().name = "missing value";
	cases.back().input = "empty=;novalue";
	cases.back().expect["empty"] = "";
	cases.back().expect["novalue"] = "";

	// Quoted value with semicolon inside quotes
	cases.push_back(TestCase());
	cases.back().name = "quoted value with semicolon";
	cases.back().input = "k=\"v1;v2\"; x=y";
	cases.back().expect["k"] = "v1;v2";
	cases.back().expect["x"] = "y";

	// Escaped quote inside quoted value
	cases.push_back(TestCase());
	cases.back().name = "escaped quote in quotes";
	cases.back().input = "q=\"say\\\"hi\\\"\""; // "say\"hi\""
	cases.back().expect["q"] = "say\"hi\"";

	// Backslashes preserved when trailing escape at end
	cases.push_back(TestCase());
	cases.back().name = "trailing backslash preserved";
	cases.back().input = "b=foo\\"; // value ends with single backslash
	cases.back().expect["b"] = "foo\\";

	// Mixed quoted and unquoted with spaces to trim
	cases.push_back(TestCase());
	cases.back().name = "mixed quoted and unquoted with trims";
	cases.back().input = " a = 1 ; b= \" two \" ; c =3 ";
	cases.back().expect["a"] = "1";
	cases.back().expect["b"] = " two ";
	cases.back().expect["c"] = "3";

	// Empty segments and extra semicolons
	cases.push_back(TestCase());
	cases.back().name = "empty segments";
	cases.back().input = ";;a=1;;b=2;;";
	cases.back().expect["a"] = "1";
	cases.back().expect["b"] = "2";

	// Value with escaped backslash in unquoted value
	cases.push_back(TestCase());
	cases.back().name = "unquoted escaped backslash";
	cases.back().input = "p=path\\to\\file";
	cases.back().expect["p"] =
		"pathtofile"; // unescapeCookieValue removes escapes

	// Quoted value that includes escaped semicolon (treated as normal char due
	// to unescape) and comma
	cases.push_back(TestCase());
	cases.back().name = "quoted with escaped semicolon";
	cases.back().input = "s=\"a\\;b\"; t=u";
	cases.back().expect["s"] = "a;b";
	cases.back().expect["t"] = "u";

	for (size_t i = 0; i < cases.size(); ++i) {
		const std::map< std::string, std::string > got =
			parseCookieField(cases[i].input);
		assertEqual(cases[i].name, got, cases[i].expect);
	}

	if (failures == 0) {
		std::cout << "\nAll tests passed!\n";
		return 0;
	}
	std::cerr << "\n" << failures << " test(s) failed." << std::endl;
	return 1;
}
