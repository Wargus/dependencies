/*
  Copyright © 2022, 2024, 2026 OOTA, Masato
  License: CC-BY-SA-3.0
  See https://creativecommons.org/licenses/by-sa/3.0/legalcode for license details.
*/
#include <catch2/catch_test_macros.hpp>
#include <string>

#include "spiritless_po/MetadataParser.h"

using namespace std;
using namespace spiritless_po;


TEST_CASE( "Normal Metadata", "[MetadataParser]" ) {
    const string test_data = R"(Project-Id-Version: test-data
Report-Msgid-Bugs-To: https://github.com/oo13/spiritless_po/issues
POT-Creation-Date: 2022-12-11 12:34+0900
PO-Revision-Date: 2022-12-11 19:87+0000
Last-Translator: tester <tester@example.com>
Language-Team: Japanese
Language: ja
MIME-Version: 1.0
Content-Type: text/plain; charset=UTF-8
Content-Transfer-Encoding: 8bit
Plural-Forms: nplurals=1; plural=0;
X-Revision: 1.1
)";
    auto metadata = MetadataParser::Parse(test_data);
    REQUIRE( metadata.size() == 12 );
    REQUIRE( metadata["Project-Id-Version"] ==  "test-data" );
    REQUIRE( metadata["Report-Msgid-Bugs-To"] ==  "https://github.com/oo13/spiritless_po/issues" );
    REQUIRE( metadata["POT-Creation-Date"] ==  "2022-12-11 12:34+0900" );
    REQUIRE( metadata["PO-Revision-Date"] ==  "2022-12-11 19:87+0000" );
    REQUIRE( metadata["Last-Translator"] ==  "tester <tester@example.com>" );
    REQUIRE( metadata["Language-Team"] ==  "Japanese" );
    REQUIRE( metadata["Language"] ==  "ja" );
    REQUIRE( metadata["MIME-Version"] ==  "1.0" );
    REQUIRE( metadata["Content-Type"] ==  "text/plain; charset=UTF-8" );
    REQUIRE( metadata["Content-Transfer-Encoding"] ==  "8bit" );
    REQUIRE( metadata["Plural-Forms"] ==  "nplurals=1; plural=0;" );
    REQUIRE( metadata["X-Revision"] ==  "1.1" );
}


TEST_CASE( "No NL at the end in Metadata", "[MetadataParser]" ) {
    const string test_data = R"(Project-Id-Version: test-data no NL
Report-Msgid-Bugs-To: /dev/null)";
    auto metadata = MetadataParser::Parse(test_data);
    REQUIRE( metadata.size() == 2 );
    REQUIRE( metadata["Project-Id-Version"] ==  "test-data no NL" );
    REQUIRE( metadata["Report-Msgid-Bugs-To"] ==  "/dev/null" );
}


TEST_CASE( "Spaces in Metadata", "[MetadataParser]" ) {
    const string test_data = R"(Project-Id-Version: test-data spaces
POT-Creation-Date : 2022-12-11 12:34+0900
PO-Revision-Date:   2022-12-11 19:87+0000
Last-Translator: tester <tester@example.com>   
Language-Team: Japanese )";
    auto metadata = MetadataParser::Parse(test_data);
    REQUIRE( metadata.size() == 5 );
    REQUIRE( metadata["Project-Id-Version"] ==  "test-data spaces" );
    REQUIRE( metadata["POT-Creation-Date "] ==  "2022-12-11 12:34+0900" );
    REQUIRE( metadata["PO-Revision-Date"] ==  "2022-12-11 19:87+0000" );
    REQUIRE( metadata["Last-Translator"] ==  "tester <tester@example.com>   " );
    REQUIRE( metadata["Language-Team"] ==  "Japanese " );
}


TEST_CASE( "Duplicate keys in Metadata", "[MetadataParser]" ) {
    const string test_data = R"(Project-Id-Version: test-data duplicate
POT-Creation-Date: 2022-12-11 12:34+0900
POT-Creation-Date: 2022-12-11 19:87+0000
)";
    auto metadata = MetadataParser::Parse(test_data);
    REQUIRE( metadata.size() == 2 );
    REQUIRE( metadata["Project-Id-Version"] ==  "test-data duplicate" );
    REQUIRE( metadata["POT-Creation-Date"] ==  "2022-12-11 12:34+0900" );
}


TEST_CASE( "Empty keys in Metadata", "[MetadataParser]" ) {
    const string test_data = R"(Project-Id-Version: test-data empty
: null value
POT-Creation-Date: 2022-12-11 19:87+0000
)";
    auto metadata = MetadataParser::Parse(test_data);
    REQUIRE( metadata.size() == 2 );
    REQUIRE( metadata["Project-Id-Version"] ==  "test-data empty" );
    REQUIRE( metadata["POT-Creation-Date"] ==  "2022-12-11 19:87+0000" );
}


TEST_CASE( "Space only value in Metadata", "[MetadataParser]" ) {
    const string test_data = R"(Project-Id-Version: test-data space only value
Empty Value:
While Space Only:                
POT-Creation-Date: 2022-12-11 19:87+0000
)";
    auto metadata = MetadataParser::Parse(test_data);
    REQUIRE( metadata.size() == 4 );
    REQUIRE( metadata["Project-Id-Version"] ==  "test-data space only value" );
    REQUIRE( metadata["Empty Value"] == "" );
    REQUIRE( metadata["While Space Only"] == "" );
    REQUIRE( metadata["POT-Creation-Date"] ==  "2022-12-11 19:87+0000" );
}


TEST_CASE( "Incomplete key in Metadata", "[MetadataParser]" ) {
const string test_data = R"(Project-Id-Version: test-data incomplete key
Incomplete 
POT-Creation-Date: 2022-12-11 19:87+0000
)";
    auto metadata = MetadataParser::Parse(test_data);
    REQUIRE( metadata.size() == 2 );
    REQUIRE( metadata["Project-Id-Version"] ==  "test-data incomplete key" );
    REQUIRE( metadata["POT-Creation-Date"] ==  "2022-12-11 19:87+0000" );
}



TEST_CASE( "GetNPlurals empty", "[MetadataParser]" ) {
    const string test_data = "";
    unsigned long nplurals = 99;
    MetadataParser::GetNPlurals(test_data, nplurals);
    REQUIRE( nplurals == 99 );
}


TEST_CASE( "GetNPlurals normal", "[MetadataParser]" ) {
    const string test_data = "text\n"
        "nplurals=5\n";
    unsigned long nplurals = 99;
    MetadataParser::GetNPlurals(test_data, nplurals);
    REQUIRE( nplurals == 5 );
}


TEST_CASE( "GetNPlurals EOT", "[MetadataParser]" ) {
    const string test_data = "text\n"
        "nplurals=";
    unsigned long nplurals = 99;
    MetadataParser::GetNPlurals(test_data, nplurals);
    REQUIRE( nplurals == 99 );
}


TEST_CASE( "GetNPlurals ignore text before nplurals", "[MetadataParser]" ) {
    const string test_data = "text\n"
        "zzznplurals=5";
    unsigned long nplurals = 99;
    MetadataParser::GetNPlurals(test_data, nplurals);
    REQUIRE( nplurals == 5 );
}


TEST_CASE( "GetNPlurals skip white spaces", "[MetadataParser]" ) {
    const string test_data = "text\n"
        "zzznplurals= \t\v\r\n"
        "\f5";
    unsigned long nplurals = 99;
    MetadataParser::GetNPlurals(test_data, nplurals);
    REQUIRE( nplurals == 5 );
}


TEST_CASE( "GetNPlurals end of digits 1", "[MetadataParser]" ) {
    const string test_data = "text\n"
        "zzznplurals=50x100";
    unsigned long nplurals = 99;
    MetadataParser::GetNPlurals(test_data, nplurals);
    REQUIRE( nplurals == 50 );
}


TEST_CASE( "GetNPlurals end of digits 2", "[MetadataParser]" ) {
    const string test_data = "text\n"
        "zzznplurals=50 100";
    unsigned long nplurals = 99;
    MetadataParser::GetNPlurals(test_data, nplurals);
    REQUIRE( nplurals == 50 );
}


TEST_CASE( "GetNPlurals end of digits 3", "[MetadataParser]" ) {
    const string test_data = "text\n"
        "zzznplurals=50; 100\n";
    unsigned long nplurals = 99;
    MetadataParser::GetNPlurals(test_data, nplurals);
    REQUIRE( nplurals == 50 );
}


TEST_CASE( "GetNPlurals end of digits 4", "[MetadataParser]" ) {
    const string test_data = "text\n"
        "zzznplurals=50\n"
        "100";
    unsigned long nplurals = 99;
    MetadataParser::GetNPlurals(test_data, nplurals);
    REQUIRE( nplurals == 50 );
}



TEST_CASE( "GetPlural empty", "[MetadataParser]" ) {
    const string test_data = "";
    std::string plural = "zzz";
    MetadataParser::GetPlural(test_data, plural);
    REQUIRE( plural == "zzz" );
}


TEST_CASE( "GetPlural normal", "[MetadataParser]" ) {
    const string test_data = "text\n"
        "plural=yyy;\n";
    std::string plural = "zzz";
    MetadataParser::GetPlural(test_data, plural);
    REQUIRE( plural == "yyy" );
}


TEST_CASE( "GetPlural EOT", "[MetadataParser]" ) {
    const string test_data = "text\n"
        "plural=";
    std::string plural = "zzz";
    MetadataParser::GetPlural(test_data, plural);
    REQUIRE( plural == "" );
}


TEST_CASE( "GetPlural ignore text before plural", "[MetadataParser]" ) {
    const string test_data = "text\n"
        "zzzplural=yyy";
    std::string plural = "zzz";
    MetadataParser::GetPlural(test_data, plural);
    REQUIRE( plural == "yyy" );
}


TEST_CASE( "GetPlural white space 1", "[MetadataParser]" ) {
    const string test_data = "text\n"
        "zzzplural= \t\v\f\ryyy";
    std::string plural = "zzz";
    MetadataParser::GetPlural(test_data, plural);
    REQUIRE( plural == " \t\v\f\ryyy" );
}


TEST_CASE( "GetPlural white space 2", "[MetadataParser]" ) {
    const string test_data = "text\n"
        "zzzplural=\n"
        "yyy";
    std::string plural = "zzz";
    MetadataParser::GetPlural(test_data, plural);
    REQUIRE( plural == "" );
}

TEST_CASE( "GetPlural end of expression 1", "[MetadataParser]" ) {
    const string test_data = "text\n"
        "zzzplural=yyy;xxx";
    std::string plural = "zzz";
    MetadataParser::GetPlural(test_data, plural);
    REQUIRE( plural == "yyy" );
}


TEST_CASE( "GetPlural end of expression 2", "[MetadataParser]" ) {
    const string test_data = "text\n"
        "zzzplural=yyy\n"
        "xxx";
    std::string plural = "zzz";
    MetadataParser::GetPlural(test_data, plural);
    REQUIRE( plural == "yyy" );
}


TEST_CASE( "GetPlural end of expression 3", "[MetadataParser]" ) {
    const string test_data = "text\n"
        "zzzplural=yyy xxx";
    std::string plural = "zzz";
    MetadataParser::GetPlural(test_data, plural);
    REQUIRE( plural == "yyy xxx" );
}


TEST_CASE( "GetPlural end of expression 4", "[MetadataParser]" ) {
    const string test_data = "text\n"
        "zzzplural=yyy\rxxx";
    std::string plural = "zzz";
    MetadataParser::GetPlural(test_data, plural);
    REQUIRE( plural == "yyy\rxxx" );
}
