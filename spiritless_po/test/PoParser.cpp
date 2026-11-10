/*
  Copyright © 2022, 2024, 2026 OOTA, Masato
  License: CC-BY-SA-3.0
  See https://creativecommons.org/licenses/by-sa/3.0/legalcode for license details.
*/
#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <string>

#include "spiritless_po/PoParser.h"

using namespace std;
using namespace spiritless_po;


bool equal(const PoParser::PoEntryT &a, const PoParser::PoEntryT &b)
{
    return a.msgid == b.msgid && a.msgstr == b.msgstr && a.error == b.error;
}

PoParser::PoEntryT create(const string &msgid, const vector<string> &msgstr, const string &error)
{
    PoParser::PoEntryT a;
    a.msgid = msgid;
    a.msgstr = msgstr;
    a.error = error;
    return a;
}

void dump_PO_entry(const vector<PoParser::PoEntryT> &entries)
{
    size_t n = 0;
    for (auto &ent : entries) {
        cout << n << ":\n";
        auto pos = ent.msgid.find("\04");
        if (pos == ent.msgid.npos) {
            cout << "  msgid: \"" << ent.msgid << "\"\n";
        } else {
            cout << "  msgctxt: \"" << ent.msgid.substr(0, pos) << "\"\n";
            cout << "  msgid: \"" << ent.msgid.substr(pos + 1) << "\"\n";
        }
        size_t pn = 0;
        for (auto &s : ent.msgstr) {
            cout << "  msgstr[" << pn << "]: \"" << s << "\"\n";
            ++pn;
        }
        cout << "  error: \"" << ent.error << "\"\n";
        ++n;
    }
}


TEST_CASE( "Normal PO Entries", "[PoParser]" ) {
    const string po_text = R"(# translator-comments
#. extracted-comment
#: references
#, flags
#| msgid previous-untranslated-string
msgid ""
msgstr "Project-Id-Version: test-data\n"

msgid "apples"
msgstr "APPLES"

msgid "bananas"
msgstr "BANANAS"

msgid "corn"
msgid_plural "corns"
msgstr[0] "CORN#0"
msgstr[1] "CORN#1"
msgstr[2] "CORN#2"
msgstr[3] "CORN#3"

msgctxt "food"
msgid "eggs"
msgstr "EGGS"

msgctxt "food"
msgid "garlic"
msgid_plural "garlics"
msgstr[0] "GARLIC#0"
msgstr[1] "GARLIC#1"
msgstr[2] "GARLIC#2"
msgstr[3] "GARLIC#3"
msgstr[4] "GARLIC#4"

msgid "apples"
msgstr "Apples"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 7 );
        REQUIRE( equal(entries[0], create("", { "Project-Id-Version: test-data\n" }, "")) );
        REQUIRE( equal(entries[1], create("apples", { "APPLES" }, "")) );
        REQUIRE( equal(entries[2], create("bananas", { "BANANAS" }, "")) );
        REQUIRE( equal(entries[3], create("corn", { "CORN#0", "CORN#1", "CORN#2", "CORN#3" }, "")) );
        REQUIRE( equal(entries[4], create("food\04eggs", { "EGGS" }, "")) );
        REQUIRE( equal(entries[5], create("food\04garlic", { "GARLIC#0", "GARLIC#1", "GARLIC#2", "GARLIC#3", "GARLIC#4" }, "")) );
        REQUIRE( equal(entries[6], create("apples", { "Apples" }, "")) );
    }
}


TEST_CASE( "Fuzzy PO Entries", "[PoParser]" ) {
    const string po_text = R"(
# fuzzy
msgid "apples"
msgstr "APPLES"

# comment
#, fuzzy
# comment
msgid "bananas"
msgstr "BANANAS"

# comment
#, fuzzy, c-format
# comment
msgid "corn"
msgid_plural "corns"
msgstr[0] "CORN#0"
msgstr[1] "CORN#1"
msgstr[2] "CORN#2"
msgstr[3] "CORN#3"

# comment
#, c-format, fuzzy
# comment
msgctxt "food"
msgid "eggs"
msgstr "EGGS"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 4 );
        REQUIRE( equal(entries[0], create("apples", { "APPLES" }, "")) );
        REQUIRE( equal(entries[1], create("bananas", { "" }, "")) );
        REQUIRE( equal(entries[2], create("corn", { "", "CORN#1", "CORN#2", "CORN#3" }, "")) );
        REQUIRE( equal(entries[3], create("food\04eggs", { "" }, "")) );
    }
}


TEST_CASE( "Comment in PO Entries", "[PoParser]" ) {
    const string po_text = R"(
# comment

msgid "apples"
msgstr "APPLES"

# comment

#, c-format
msgid "bananas"
msgstr "BANANAS"

# comment
msgid "corn"
msgid_plural "corns"
msgstr[0] "CORN#0"
msgstr[1] "CORN#1"
msgstr[2] "CORN#2"
msgstr[3] "CORN#3"

#, fuzzy


msgctxt "food"
msgid "eggs"
msgstr "EGGS"

msgctxt "food"
msgid "garlic"
msgid_plural "garlics"
msgstr[0] "GARLIC#0"
msgstr[1] "GARLIC#1"
msgstr[2] "GARLIC#2"
msgstr[3] "GARLIC#3"
msgstr[4] "GARLIC#4"
# comment

#~ msgid "aaa"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 5 );
        REQUIRE( equal(entries[0], create("apples", { "APPLES" }, "")) );
        REQUIRE( equal(entries[1], create("bananas", { "BANANAS" }, "")) );
        REQUIRE( equal(entries[2], create("corn", { "CORN#0", "CORN#1", "CORN#2", "CORN#3" }, "")) );
        REQUIRE( equal(entries[3], create("food\04eggs", { "" }, "")) );
        REQUIRE( equal(entries[4], create("food\04garlic", { "GARLIC#0", "GARLIC#1", "GARLIC#2", "GARLIC#3", "GARLIC#4" }, "")) );
    }
}


TEST_CASE( "Empty stream in PO Entries", "[PoParser]" ) {
    const string po_text = R"(
# empty
#, empty
# empty
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 0 );
    }
}


TEST_CASE( "Empty text", "[PoParser]" ) {
    const string po_text = R"(
msgctxt ""
msgid "apple"
msgstr "APPLE"

msgctxt "food"
msgid ""
msgstr "hungry"

msgctxt ""
msgid ""
msgstr "void"

msgctxt "food"
msgid "surstromming"
msgstr ""

msgid ""
msgstr ""
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 5 );
        REQUIRE( equal(entries[0], create("\04apple", { "APPLE" }, "")) );
        REQUIRE( equal(entries[1], create("food\04", { "hungry" }, "")) );
        REQUIRE( equal(entries[2], create("\04", { "void" }, "")) );
        REQUIRE( equal(entries[3], create("food\04surstromming", { "" }, "")) );
        REQUIRE( equal(entries[4], create("", { "" }, "")) );
    }
}


TEST_CASE( "Errors in PO Entries (1/2)", "[PoParser]" ) {
    const string po_text = R"(
msgstr "APPLES"

msgid_plural "corns"

msgstr[0] "CORNS#0"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 1 );
        REQUIRE( equal(entries[0], create("", {}, "2,1: Unexpected msgstr.")) );
    }
}


TEST_CASE( "Errors in PO Entries (2/2)", "[PoParser]" ) {
    const string po_text = R"(
msgid "apples"
msgctxt "food"
msgstr "APPLES"

msgid "bananas"
msgstr[0] "BANANAS"

msgid_plural "corns"
msgid "corn"
msgstr[0] "CORNS#0"

msgid "hops"

msgctxt "food"
msgstr "Apples"

msgid "garlic"
msgid_plural "garlics"
msgstr "GARLIC#0"

msgid "eggs")";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 8 );
        REQUIRE( equal(entries[0], create("", {}, "3,1: Unexpected msgctxt (the previous entry is incomplete).")) );
        REQUIRE( equal(entries[1], create("", {}, "4,1: Unexpected msgstr.")) );
        REQUIRE( equal(entries[2], create("", {}, "7,1: Unexpected msgstr[n].")) );
        REQUIRE( equal(entries[3], create("", {}, "11,1: Unexpected msgstr[n].")) );
        REQUIRE( equal(entries[4], create("", {}, "15,1: Unexpected msgctxt (the previous entry is incomplete).")) );
        REQUIRE( equal(entries[5], create("", {}, "16,1: Unexpected msgstr.")) );
        REQUIRE( equal(entries[6], create("", {}, "20,1: Unexpected msgstr.")) );
        REQUIRE( equal(entries[7], create("", {}, "22,13: Unexpected EOT (the previous entry is incomplete).")) );
    }
}


TEST_CASE( "Multi line strings in PO Entries", "[PoParser]" ) {
    const string po_text = R"(
msgid "apples"
msgstr ""
"APPLES"

msgid "bananas"
msgstr "BANANAS\n"
"BANANAS\n"
"BANANAS\n"
"BANANAS\n"
"BANANAS\n"

"BANANAS"

msgid ""
"corn"
msgid_plural "corns\n"
"corns"
msgstr[0] ""
"CORNS#0"
msgstr[1] "CORNS#1\n"
"CORNS#1"

msgid ""
"garlics\n"
"garlics\n"
"garlics"
msgstr ""
"GARLICS\n"
"GARLICS"

msgctxt ""
"food"
msgid "eggs"
msgstr "EGGS"

msgctxt ""
"food\n"
"food"
msgid "hops"
msgstr "HOPS"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 6 );
        REQUIRE( equal(entries[0], create("apples", { "APPLES" }, "")) );
        REQUIRE( equal(entries[1], create("bananas", { "BANANAS\nBANANAS\nBANANAS\nBANANAS\nBANANAS\nBANANAS" }, "")) );
        REQUIRE( equal(entries[2], create("corn", { "CORNS#0", "CORNS#1\nCORNS#1" }, "")) );
        REQUIRE( equal(entries[3], create("garlics\ngarlics\ngarlics", { "GARLICS\nGARLICS" }, "")) );
        REQUIRE( equal(entries[4], create("food\04eggs", { "EGGS" }, "")) );
        REQUIRE( equal(entries[5], create("food\nfood\04hops", { "HOPS" }, "")) );
    }
}


TEST_CASE( "Escape Sequence", "[PoParser]" ) {
    const string po_text = R"(
msgid "apple"
msgstr "a\\b\ac\bd\fe\ng\rh\ti\vjzk\033l\1111m\xABCDEFG"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 1 );
        REQUIRE( equal(entries[0], create("apple", { "a\\b\ac\bd\fe\ng\rh\ti\vjzk\x1bl\x49" "1m\xefG" }, "")) );
    }
}


TEST_CASE( "No quote at beginning of text (1/2)", "[PoParser]" ) {
    const string po_text = R"(
msgid xapples"
msgstr "APPLES"

msgid "bananas"
msgstr "BANANAS"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 2 );
        REQUIRE( equal(entries[0], create("", {}, "2,7: Unknown token.")) );
        REQUIRE( equal(entries[1], create("bananas", { "BANANAS" }, "")) );
    }
}


TEST_CASE( "No quote at beginning of text (2/2)", "[PoParser]" ) {
    const string po_text = R"(
msgid ""
xapples"
msgstr "APPLES"

msgid "bananas"
msgstr "BANANAS"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 2 );
        REQUIRE( equal(entries[0], create("", {}, "3,1: Unknown token.")) );
        REQUIRE( equal(entries[1], create("bananas", { "BANANAS" }, "")) );
    }
}


TEST_CASE( "EOF in PO Entries (1/28)", "[PoParser]" ) {
    const string test_data_eof_1 = R"(

# comment
#, flag
msgctxt "ctxt\140"
"ctxt"
msgid "id\x40"
"id"
msgid_plural "id_plural"
"id_plural"
msgstr[0] "str[0]\t"
"str[0]"
)";
    auto entries = PoParser::GetEntries(test_data_eof_1.begin(), test_data_eof_1.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 1 );
        REQUIRE( equal(entries[0], create("ctxt\140ctxt\04id\x40id", { "str[0]\tstr[0]" }, "")) );
    }
}


TEST_CASE( "EOF in PO Entries (2/28)", "[PoParser]" ) {
    const string po_text = R"(

# comment
#, flag
msgctxt "ctxt\140"
"ctxt"
msgid "id\x40"
"id"
msgid_plural "id_plural"
"id_plural"
msgstr[0] "str[0]\t"
"str[0]")";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 1 );
        REQUIRE( equal(entries[0], create("ctxt\140ctxt\04id\x40id", { "str[0]\tstr[0]" }, "")) );
    }
}


TEST_CASE( "EOF in PO Entries (3/28)", "[PoParser]" ) {
    const string po_text = R"(

# comment
#, flag
msgctxt "ctxt\140"
"ctxt"
msgid "id\x40"
"id"
msgid_plural "id_plural"
"id_plural"
msgstr[0] "str[0]\t"
"str[0])";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 1 );
        REQUIRE( equal(entries[0], create("", {}, "12,8: Closing double quotation mark is expected.")) );
    }
}


TEST_CASE( "EOF in PO Entries (4/28)", "[PoParser]" ) {
    const string po_text = R"(

# comment
#, flag
msgctxt "ctxt\140"
"ctxt"
msgid "id\x40"
"id"
msgid_plural "id_plural"
"id_plural"
msgstr[0] "str[0]\t"
")";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 1 );
        REQUIRE( equal(entries[0], create("", {}, "12,2: Closing double quotation mark is expected.")) );
    }
}


TEST_CASE( "EOF in PO Entries (5/28)", "[PoParser]" ) {
    const string po_text = R"(

# comment
#, flag
msgctxt "ctxt\140"
"ctxt"
msgid "id\x40"
"id"
msgid_plural "id_plural"
"id_plural"
msgstr[0] "str[0]\t)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 1 );
        REQUIRE( equal(entries[0], create("", {}, "11,20: Closing double quotation mark is expected.")) );
    }
}


TEST_CASE( "EOF in PO Entries (6/28)", "[PoParser]" ) {
    const string po_text = R"(

# comment
#, flag
msgctxt "ctxt\140"
"ctxt"
msgid "id\x40"
"id"
msgid_plural "id_plural"
"id_plural"
msgstr[0] "str[0]\)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 1 );
        REQUIRE( equal(entries[0], create("", {}, "11,19: Invalid escape sequence.")) );
    }
}


TEST_CASE( "EOF in PO Entries (7/28)", "[PoParser]" ) {
    const string po_text = R"(

# comment
#, flag
msgctxt "ctxt\140"
"ctxt"
msgid "id\x40"
"id"
msgid_plural "id_plural"
"id_plural"
msgstr[0] )";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 1 );
        REQUIRE( equal(entries[0], create("", {}, "11,11: Unexpected EOT (the previous entry is incomplete).")) );
    }
}


TEST_CASE( "EOF in PO Entries (8/28)", "[PoParser]" ) {
    const string po_text = R"(

# comment
#, flag
msgctxt "ctxt\140"
"ctxt"
msgid "id\x40"
"id"
msgid_plural "id_plural"
"id_plural"
msgstr[0)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 1 );
        REQUIRE( equal(entries[0], create("", {}, "11,9: ']' is expected.")) );
    }
}


TEST_CASE( "EOF in PO Entries (9/28)", "[PoParser]" ) {
    const string po_text = R"(

# comment
#, flag
msgctxt "ctxt\140"
"ctxt"
msgid "id\x40"
"id"
msgid_plural "id_plural"
"id_plural"
msgstr[)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 1 );
        REQUIRE( equal(entries[0], create("", {}, "11,8: '0'..'9' is expected.")) );
    }
}


TEST_CASE( "EOF in PO Entries (10/28)", "[PoParser]" ) {
    const string po_text = R"(

# comment
#, flag
msgctxt "ctxt\140"
"ctxt"
msgid "id\x40"
"id"
msgid_plural "id_plural"
"id_plural"
msgstr)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 1 );
        REQUIRE( equal(entries[0], create("", {}, "11,1: Unexpected msgstr.")) );
    }
}


TEST_CASE( "EOF in PO Entries (11/28)", "[PoParser]" ) {
    const string po_text = R"(

# comment
#, flag
msgctxt "ctxt\140"
"ctxt"
msgid "id\x40"
"id"
msgid_plural "id_plural"
"id_plural"
ms)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 1 );
        REQUIRE( equal(entries[0], create("", {}, "11,1: Unknown keyword.")) );
    }
}


TEST_CASE( "EOF in PO Entries (12/28)", "[PoParser]" ) {
    const string po_text = R"(

# comment
#, flag
msgctxt "ctxt\140"
"ctxt"
msgid "id\x40"
"id"
msgid_plural "id_plural"
"id_plural"
m)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 1 );
        REQUIRE( equal(entries[0], create("", {}, "11,1: Unknown keyword.")) );
    }
}


TEST_CASE( "EOF in PO Entries (13/28)", "[PoParser]" ) {
    const string po_text = R"(

# comment
#, flag
msgctxt "ctxt\140"
"ctxt"
msgid "id\x40"
"id"
msgid_plural "id_plural")";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 1 );
        REQUIRE( equal(entries[0], create("", {}, "9,25: Unexpected EOT (the previous entry is incomplete).")) );
    }
}


TEST_CASE( "EOF in PO Entries (14/28)", "[PoParser]" ) {
    const string po_text = R"(

# comment
#, flag
msgctxt "ctxt\140"
"ctxt"
msgid "id\x40"
"id"
msgid_plural "id_plural)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 1 );
        REQUIRE( equal(entries[0], create("", {}, "9,24: Closing double quotation mark is expected.")) );
    }
}


TEST_CASE( "EOF in PO Entries (15/28)", "[PoParser]" ) {
    const string po_text = R"(

# comment
#, flag
msgctxt "ctxt\140"
"ctxt"
msgid "id\x40"
"id"
msgid_plura)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 1 );
        REQUIRE( equal(entries[0], create("", {}, "9,1: Unknown keyword.")) );
    }
}


TEST_CASE( "EOF in PO Entries (16/28)", "[PoParser]" ) {
    const string po_text = R"(

# comment
#, flag
msgctxt "ctxt\140"
"ctxt"
msgid "id\x40"
"id"
msgstr "str)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 1 );
        REQUIRE( equal(entries[0], create("", {}, "9,12: Closing double quotation mark is expected.")) );
    }
}


TEST_CASE( "EOF in PO Entries (17/28)", "[PoParser]" ) {
    const string po_text = R"(

# comment
#, flag
msgctxt "ctxt\140"
"ctxt"
msgid "id\x40"
"id"
msgst)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 1 );
        REQUIRE( equal(entries[0], create("", {}, "9,1: Unknown keyword.")) );
    }
}


TEST_CASE( "EOF in PO Entries (18/28)", "[PoParser]" ) {
    const string po_text = R"(

# comment
#, flag
msgctxt "ctxt\140"
"ctxt"
msgid "id\x40)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 1 );
        REQUIRE( equal(entries[0], create("", {}, "7,14: Closing double quotation mark is expected.")) );
    }
}


TEST_CASE( "EOF in PO Entries (19/28)", "[PoParser]" ) {
    const string po_text = R"(

# comment
#, flag
msgctxt "ctxt\140"
"ctxt"
msgid "id\x)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 1 );
        REQUIRE( equal(entries[0], create("", {}, "7,12: [0-9A-Fa-f] is expected.")) );
    }
}


TEST_CASE( "EOF in PO Entries (20/28)", "[PoParser]" ) {
    const string po_text = R"(

# comment
#, flag
msgctxt "ctxt\140"
"ctxt"
msgi)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 1 );
        REQUIRE( equal(entries[0], create("", {}, "7,1: Unknown keyword.")) );
    }
}


TEST_CASE( "EOF in PO Entries (21/28)", "[PoParser]" ) {
    const string po_text = R"(

# comment
#, flag
msgctxt "ctxt\140)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 1 );
        REQUIRE( equal(entries[0], create("", {}, "5,18: Closing double quotation mark is expected.")) );
    }
}


TEST_CASE( "EOF in PO Entries (22/28)", "[PoParser]" ) {
    const string po_text = R"(

# comment
#, flag
msgctxt "ctxt\)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 1 );
        REQUIRE( equal(entries[0], create("", {}, "5,15: Invalid escape sequence.")) );
    }
}


TEST_CASE( "EOF in PO Entries (23/28)", "[PoParser]" ) {
    const string po_text = R"(

# comment
#, flag
msgctx)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 1 );
        REQUIRE( equal(entries[0], create("", {}, "5,1: Unknown keyword.")) );
    }
}


TEST_CASE( "EOF in PO Entries (24/28)", "[PoParser]" ) {
    const string po_text = R"(

# comment
#, flag)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 0 );
    }
}


TEST_CASE( "EOF in PO Entries (25/28)", "[PoParser]" ) {
    const string po_text = R"(

# comment
#,)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 0 );
    }
}


TEST_CASE( "EOF in PO Entries (26/28)", "[PoParser]" ) {
    const string po_text = R"(

# comment)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 0 );
    }
}


TEST_CASE( "EOF in PO Entries (27/28)", "[PoParser]" ) {
    const string po_text = R"(

#)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 0 );
    }
}


TEST_CASE( "EOF in PO Entries (28/28)", "[PoParser]" ) {
    const string po_text = R"(
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 0 );
    }
}


TEST_CASE( "EOF in error recovering", "[PoParser]" ) {
    const string po_text = R"(
msgstr "APPLES"
msgstr "BANANAS"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 1 );
        REQUIRE( equal(entries[0], create("", {}, "2,1: Unexpected msgstr.")) );
    }
}



TEST_CASE( "fuzzy empty 1", "[PoParser]" ) {
    const string po_text = R"(
#,
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 0 );
    }
}


TEST_CASE( "fuzzy empty 2", "[PoParser]" ) {
    const string po_text = R"(
#=
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 0 );
    }
}


TEST_CASE( "fuzzy empty 3", "[PoParser]" ) {
    const string po_text = R"(
#,
msgid "a"
msgstr "b"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 1 );
        REQUIRE( equal(entries[0], create("a", { "b" }, "")) );
    }
}


TEST_CASE( "fuzzy empty 4", "[PoParser]" ) {
    const string po_text = R"(
#=
msgid "a"
msgstr "b"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 1 );
        REQUIRE( equal(entries[0], create("a", { "b" }, "")) );
    }
}


TEST_CASE( "fuzzy comma separator 1", "[PoParser]" ) {
    const string po_text = R"(
#, a,b,c,fuzzy,d,
msgid "a"
msgstr "b"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 1 );
        REQUIRE( equal(entries[0], create("a", { "" }, "")) );
    }
}


TEST_CASE( "fuzzy comma separator 2", "[PoParser]" ) {
    const string po_text = R"(
#= a,b,c,fuzzy,d,
msgid "a"
msgstr "b"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 1 );
        REQUIRE( equal(entries[0], create("a", { "" }, "")) );
    }
}


TEST_CASE( "fuzzy comma separator 3", "[PoParser]" ) {
    const string po_text = R"(
#, a,b,c,fuzzy2,d,
msgid "a"
msgstr "b"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 1 );
        REQUIRE( equal(entries[0], create("a", { "b" }, "")) );
    }
}


TEST_CASE( "fuzzy comma separator 4", "[PoParser]" ) {
    const string po_text = R"(
#= a,b,c,fuzzy2,d,
msgid "a"
msgstr "b"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 1 );
        REQUIRE( equal(entries[0], create("a", { "b" }, "")) );
    }
}


TEST_CASE( "fuzzy no separator 1", "[PoParser]" ) {
    const string po_text = R"(
#,fuzzy
msgid "a"
msgstr "b"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 1 );
        REQUIRE( equal(entries[0], create("a", { "" }, "")) );
    }
}


TEST_CASE( "fuzzy no separator 2", "[PoParser]" ) {
    const string po_text = R"(
#=fuzzy
msgid "a"
msgstr "b"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 1 );
        REQUIRE( equal(entries[0], create("a", { "" }, "")) );
    }
}


TEST_CASE( "fuzzy space separator", "[PoParser]" ) {
    const string po_text = R"(
#, a b	c fuzzy	d
msgid "a"
msgstr "b"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 1 );
        REQUIRE( equal(entries[0], create("a", { "" }, "")) );
    }
}


TEST_CASE( "fuzzy multi line", "[PoParser]" ) {
    const string po_text = R"(
#, c-format, fuzzy, rust-format
#, a b c
#, fuzzy2
#, d e f
msgid "a"
msgstr "b"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 1 );
        REQUIRE( equal(entries[0], create("a", { "" }, "")) );
    }
}


TEST_CASE( "fuzzy end state", "[PoParser]" ) {
    const string po_text = R"(
#,
#,,
#, a
#, a,
#, fuzz
#, fuzz,
#, fuzzy
#, fuzzy,
#, fuzzy2
#, fuzzy2,
msgid "a"
msgstr "b"
#, fuzz)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 1 );
        REQUIRE( equal(entries[0], create("a", { "" }, "")) );
    }
}



TEST_CASE( "Lex EOT empty string", "[PoParser]" ) {
    const string po_text = "";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 0 );
    }
}


TEST_CASE( "Lex EOT space string", "[PoParser]" ) {
    const string po_text = " ";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 0 );
    }
}


TEST_CASE( "Lex EOT newline string", "[PoParser]" ) {
    const string po_text = "\n";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 0 );
    }
}


TEST_CASE( "Lex EOT newline and space string", "[PoParser]" ) {
    const string po_text = "\n ";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 0 );
    }
}


TEST_CASE( "Lex EOT comment 1", "[PoParser]" ) {
    const string po_text = "#";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 0 );
    }
}


TEST_CASE( "Lex EOT comment 2", "[PoParser]" ) {
    const string po_text = "#.";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 0 );
    }
}


TEST_CASE( "Lex EOT comment 3", "[PoParser]" ) {
    const string po_text = "#.aaa";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 0 );
    }
}


TEST_CASE( "Lex EOT comment 4", "[PoParser]" ) {
    const string po_text = "#.aaa\n";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 0 );
    }
}


TEST_CASE( "Lex EOT flag comment 1", "[PoParser]" ) {
    const string po_text = "#,";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 0 );
    }
}


TEST_CASE( "Lex EOT flag comment 2", "[PoParser]" ) {
    const string po_text = "#,a";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 0 );
    }
}


TEST_CASE( "Lex EOT flag comment 3", "[PoParser]" ) {
    const string po_text = "#, a";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 0 );
    }
}


TEST_CASE( "Lex EOT flag comment 4", "[PoParser]" ) {
    const string po_text = "#, a ";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 0 );
    }
}


TEST_CASE( "Lex EOT flag comment 5", "[PoParser]" ) {
    const string po_text = "#, a fuzzy";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 0 );
    }
}


TEST_CASE( "Lex EOT flag comment 6", "[PoParser]" ) {
    const string po_text = "#, a fuzzya";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 0 );
    }
}


TEST_CASE( "Lex EOT flag comment 7", "[PoParser]" ) {
    const string po_text = "#, a fuzzy ";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 0 );
    }
}


TEST_CASE( "Lex EOT flag comment 8", "[PoParser]" ) {
    const string po_text = "#, a fuzzya ";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 0 );
    }
}


TEST_CASE( "Lex flag comment", "[PoParser]" ) {
    const string po_text = R"(
#,
#=
#,a
#=a
#,a 
#=a 
#,a fuzz
#=a fuzz
#,a fuzzy
#=a fuzzy
#,a fuzzya
#=a fuzzya
#,a fuzzy 
#=a fuzzy 
#,a fuzzya 
#=a fuzzya 
#,a fuzzya fuzzy
#=a fuzzya fuzzy
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 0 );
    }
}


TEST_CASE( "Lex EOT msg keyword 1", "[PoParser]" ) {
    const string po_text = "m";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 1 );
        REQUIRE( equal(entries[0], create("", {}, "1,1: Unknown keyword.")) );
    }
}


TEST_CASE( "Lex EOT msg keyword 2", "[PoParser]" ) {
    const string po_text = "ms";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 1 );
        REQUIRE( equal(entries[0], create("", {}, "1,1: Unknown keyword.")) );
    }
}


TEST_CASE( "Lex EOT msg keyword 3", "[PoParser]" ) {
    const string po_text = "msg";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 1 );
        REQUIRE( equal(entries[0], create("", {}, "1,1: Unknown keyword.")) );
    }
}


TEST_CASE( "Lex EOT quoted text", "[PoParser]" ) {
    const string po_text = "\"";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 1 );
        REQUIRE( equal(entries[0], create("", {}, "1,2: Closing double quotation mark is expected.")) );
    }
}


TEST_CASE( "Lex EOT msgctxt 1", "[PoParser]" ) {
    const string po_text = "msgc";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 1 );
        REQUIRE( equal(entries[0], create("", {}, "1,1: Unknown keyword.")) );
    }
}


TEST_CASE( "Lex EOT msgctxt 2", "[PoParser]" ) {
    const string po_text = "msgct";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 1 );
        REQUIRE( equal(entries[0], create("", {}, "1,1: Unknown keyword.")) );
    }
}


TEST_CASE( "Lex EOT msgctxt 3", "[PoParser]" ) {
    const string po_text = "msgctx";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 1 );
        REQUIRE( equal(entries[0], create("", {}, "1,1: Unknown keyword.")) );
    }
}


TEST_CASE( "Lex EOT msgctxt 4", "[PoParser]" ) {
    const string po_text = "msgctxt";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 1 );
        REQUIRE( equal(entries[0], create("", {}, "1,8: Unexpected EOT.")) );
    }
}


TEST_CASE( "Lex EOT msgid 1", "[PoParser]" ) {
    const string po_text = "msgi";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 1 );
        REQUIRE( equal(entries[0], create("", {}, "1,1: Unknown keyword.")) );
    }
}


TEST_CASE( "Lex EOT msgid 2", "[PoParser]" ) {
    const string po_text = "msgid";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 1 );
        REQUIRE( equal(entries[0], create("", {}, "1,6: Unexpected EOT (the previous entry is incomplete).")) );
    }
}


TEST_CASE( "Lex EOT msgid 3", "[PoParser]" ) {
    const string po_text = "msgid ";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 1 );
        REQUIRE( equal(entries[0], create("", {}, "1,7: Unexpected EOT (the previous entry is incomplete).")) );
    }
}


TEST_CASE( "Lex EOT msgid_plural 1", "[PoParser]" ) {
    const string po_text = "msgid_";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 1 );
        REQUIRE( equal(entries[0], create("", {}, "1,1: Unknown keyword.")) );
    }
}


TEST_CASE( "Lex EOT msgid_plural 2", "[PoParser]" ) {
    const string po_text = "msgid_p";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 1 );
        REQUIRE( equal(entries[0], create("", {}, "1,1: Unknown keyword.")) );
    }
}


TEST_CASE( "Lex EOT msgid_plural 3", "[PoParser]" ) {
    const string po_text = "msgid_pl";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 1 );
        REQUIRE( equal(entries[0], create("", {}, "1,1: Unknown keyword.")) );
    }
}


TEST_CASE( "Lex EOT msgid_plural 4", "[PoParser]" ) {
    const string po_text = "msgid_plu";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 1 );
        REQUIRE( equal(entries[0], create("", {}, "1,1: Unknown keyword.")) );
    }
}


TEST_CASE( "Lex EOT msgid_plural 5", "[PoParser]" ) {
    const string po_text = "msgid_plur";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 1 );
        REQUIRE( equal(entries[0], create("", {}, "1,1: Unknown keyword.")) );
    }
}


TEST_CASE( "Lex EOT msgid_plural 6", "[PoParser]" ) {
    const string po_text = "msgid_plura";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 1 );
        REQUIRE( equal(entries[0], create("", {}, "1,1: Unknown keyword.")) );
    }
}


TEST_CASE( "Lex EOT msgid_plural 7", "[PoParser]" ) {
    const string po_text = "msgid_plural";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 1 );
        REQUIRE( equal(entries[0], create("", {}, "1,1: Unexpected msgid_plural.")) );
    }
}


TEST_CASE( "Lex EOT msgstr 1", "[PoParser]" ) {
    const string po_text = "msgs";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 1 );
        REQUIRE( equal(entries[0], create("", {}, "1,1: Unknown keyword.")) );
    }
}


TEST_CASE( "Lex EOT msgstr 2", "[PoParser]" ) {
    const string po_text = "msgst";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 1 );
        REQUIRE( equal(entries[0], create("", {}, "1,1: Unknown keyword.")) );
    }
}


TEST_CASE( "Lex EOT msgstr 3", "[PoParser]" ) {
    const string po_text = "msgstr";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 1 );
        REQUIRE( equal(entries[0], create("", {}, "1,1: Unexpected msgstr.")) );
    }
}


TEST_CASE( "Lex EOT msgstr 4", "[PoParser]" ) {
    const string po_text = "msgstr ";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 1 );
        REQUIRE( equal(entries[0], create("", {}, "1,1: Unexpected msgstr.")) );
    }
}


TEST_CASE( "Lex EOT msgstr[n] 1", "[PoParser]" ) {
    const string po_text = "msgstr[";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 1 );
        REQUIRE( equal(entries[0], create("", {}, "1,8: '0'..'9' is expected.")) );
    }
}


TEST_CASE( "Lex EOT msgstr[n] 2", "[PoParser]" ) {
    const string po_text = "msgstr[ ";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 1 );
        REQUIRE( equal(entries[0], create("", {}, "1,9: '0'..'9' is expected.")) );
    }
}


TEST_CASE( "Lex EOT msgstr[n] 3", "[PoParser]" ) {
    const string po_text = "msgstr[0";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 1 );
        REQUIRE( equal(entries[0], create("", {}, "1,9: ']' is expected.")) );
    }
}


TEST_CASE( "Lex EOT msgstr[n] 4", "[PoParser]" ) {
    const string po_text = "msgstr[0 ";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 1 );
        REQUIRE( equal(entries[0], create("", {}, "1,10: ']' is expected.")) );
    }
}


TEST_CASE( "Lex EOT msgstr[n] 5", "[PoParser]" ) {
    const string po_text = "msgstr[0]";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 1 );
        REQUIRE( equal(entries[0], create("", {}, "1,1: Unexpected msgstr[n].")) );
    }
}


TEST_CASE( "Lex EOT text", "[PoParser]" ) {
    const string po_text = "\"text";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 1 );
        REQUIRE( equal(entries[0], create("", {}, "1,6: Closing double quotation mark is expected.")) );
    }
}


TEST_CASE( "Lex EOT newline in text", "[PoParser]" ) {
    const string po_text = "\"\n";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 1 );
        REQUIRE( equal(entries[0], create("", {}, "1,2: The text may not contain a newline.")) );
    }
}


TEST_CASE( "Lex EOT end of quoted text", "[PoParser]" ) {
    const string po_text = R"("")";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 1 );
        REQUIRE( equal(entries[0], create("", {}, "1,1: Unexpected quoted text.")) );
    }
}


TEST_CASE( "Lex EOT escape char in text", "[PoParser]" ) {
    const string po_text = R"("\)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 1 );
        REQUIRE( equal(entries[0], create("", {}, "1,3: Invalid escape sequence.")) );
    }
}


TEST_CASE( "Lex EOT oct digit in text 1", "[PoParser]" ) {
    const string po_text = R"("\1)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 1 );
        REQUIRE( equal(entries[0], create("", {}, "1,4: Closing double quotation mark is expected.")) );
    }
}


TEST_CASE( "Lex EOT oct digit in text 2", "[PoParser]" ) {
    const string po_text = R"("\11)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 1 );
        REQUIRE( equal(entries[0], create("", {}, "1,5: Closing double quotation mark is expected.")) );
    }
}


TEST_CASE( "Lex EOT oct digit in text 3", "[PoParser]" ) {
    const string po_text = R"("\111)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 1 );
        REQUIRE( equal(entries[0], create("", {}, "1,6: Closing double quotation mark is expected.")) );
    }
}


TEST_CASE( "Lex EOT oct digit in text 4", "[PoParser]" ) {
    const string po_text = R"("\1111)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 1 );
        REQUIRE( equal(entries[0], create("", {}, "1,7: Closing double quotation mark is expected.")) );
    }
}


TEST_CASE( "Lex EOT hex digit in text 1", "[PoParser]" ) {
    const string po_text = R"("\x)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 1 );
        REQUIRE( equal(entries[0], create("", {}, "1,4: [0-9A-Fa-f] is expected.")) );
    }
}


TEST_CASE( "Lex EOT hex digit in text 2", "[PoParser]" ) {
    const string po_text = R"("\xa)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 1 );
        REQUIRE( equal(entries[0], create("", {}, "1,5: Closing double quotation mark is expected.")) );
    }
}


TEST_CASE( "Lex EOT hex digit in text 3", "[PoParser]" ) {
    const string po_text = R"("\xab)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 1 );
        REQUIRE( equal(entries[0], create("", {}, "1,6: Closing double quotation mark is expected.")) );
    }
}


TEST_CASE( "Lex EOT hex digit in text 4", "[PoParser]" ) {
    const string po_text = R"("\xabc)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 1 );
        REQUIRE( equal(entries[0], create("", {}, "1,7: Closing double quotation mark is expected.")) );
    }
}


TEST_CASE( "Lex EOT hex digit in text 5", "[PoParser]" ) {
    const string po_text = R"("\xabcx)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 1 );
        REQUIRE( equal(entries[0], create("", {}, "1,8: Closing double quotation mark is expected.")) );
    }
}



TEST_CASE( "Initial token comment", "[PoParser]" ) {
    const string po_text = R"(# comment
msgid "a"
msgstr "A"

msgid "b"
msgstr "B"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 2 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("b", { "B" }, "")) );
    }
}


TEST_CASE( "Initial token fuzzy comment", "[PoParser]" ) {
    const string po_text = R"(#, fuzzy
msgid "a"
msgstr "A"

msgid "b"
msgstr "B"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 2 );
        REQUIRE( equal(entries[0], create("a", { "" }, "")) );
        REQUIRE( equal(entries[1], create("b", { "B" }, "")) );
    }
}


TEST_CASE( "Initial token msgctxt", "[PoParser]" ) {
    const string po_text = R"(msgctxt "ca"
msgid "a"
msgstr "A"

msgid "b"
msgstr "B"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 2 );
        REQUIRE( equal(entries[0], create("ca" "\x04" "a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("b", { "B" }, "")) );
    }
}


TEST_CASE( "Initial token msgid", "[PoParser]" ) {
    const string po_text = R"(msgid "a"
msgstr "A"

msgid "b"
msgstr "B"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 2 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("b", { "B" }, "")) );
    }
}


TEST_CASE( "Initial token msgid_plural", "[PoParser]" ) {
    const string po_text = R"(msgid_plural "as"
msgstr "A"

msgid "b"
msgstr "B"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 2 );
        REQUIRE( equal(entries[0], create("", {}, "1,1: Unexpected msgid_plural.")) );
        REQUIRE( equal(entries[1], create("b", { "B" }, "")) );
    }
}


TEST_CASE( "Initial token msgstr", "[PoParser]" ) {
    const string po_text = R"(msgstr "A"

msgid "b"
msgstr "B"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 2 );
        REQUIRE( equal(entries[0], create("", {}, "1,1: Unexpected msgstr.")) );
        REQUIRE( equal(entries[1], create("b", { "B" }, "")) );
    }
}


TEST_CASE( "Initial token msgstr[n]", "[PoParser]" ) {
    const string po_text = R"(msgstr[0] "A"

msgid "b"
msgstr "B"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 2 );
        REQUIRE( equal(entries[0], create("", {}, "1,1: Unexpected msgstr[n].")) );
        REQUIRE( equal(entries[1], create("b", { "B" }, "")) );
    }
}


TEST_CASE( "Initial token quoted text", "[PoParser]" ) {
    const string po_text = R"("A"

msgid "a"
msgstr "A"

msgid "b"
msgstr "B"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 2 );
        REQUIRE( equal(entries[0], create("", {}, "1,1: Unexpected quoted text.")) );
        REQUIRE( equal(entries[1], create("b", { "B" }, "")) );
    }
}


TEST_CASE( "Initial token error", "[PoParser]" ) {
    const string po_text = R"(error_token

msgid "a"
msgstr "A"

msgid "b"
msgstr "B"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 2 );
        REQUIRE( equal(entries[0], create("", {}, "1,1: Unknown token.")) );
        REQUIRE( equal(entries[1], create("b", { "B" }, "")) );
    }
}


TEST_CASE( "Initial token EOT", "[PoParser]" ) {
    const string po_text = R"()";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 0 );
    }
}



// State ERROR_BEFORE_MSGID
TEST_CASE( "Input COMMENT in state ERROR_BEFORE_MSGID", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

#
error_before_msgid
# comment
msgid "b"
msgstr "B"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "6,1: Unknown token.")) );
        REQUIRE( equal(entries[2], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input FUZZY in state ERROR_BEFORE_MSGID", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

#
error_before_msgid
#, fuzzy
msgid "b"
msgstr "B"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "6,1: Unknown token.")) );
        REQUIRE( equal(entries[2], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input MSGCTXT in state ERROR_BEFORE_MSGID", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

#
error_before_msgid
msgctxt "cb"
msgid "b"
msgstr "B"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "6,1: Unknown token.")) );
        REQUIRE( equal(entries[2], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input MSGID in state ERROR_BEFORE_MSGID", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

#
error_before_msgid
msgid "b"
msgstr "B"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "6,1: Unknown token.")) );
        REQUIRE( equal(entries[2], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input MSGID_PLURAL in state ERROR_BEFORE_MSGID", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

#
error_before_msgid
msgid_plural "bp"
msgstr "B"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "6,1: Unknown token.")) );
        REQUIRE( equal(entries[2], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input MSGSTR in state ERROR_BEFORE_MSGID", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

#
error_before_msgid
msgstr "B"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "6,1: Unknown token.")) );
        REQUIRE( equal(entries[2], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input MSGSTR_PLURAL in state ERROR_BEFORE_MSGID", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

#
error_before_msgid
msgstr[0] "B"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "6,1: Unknown token.")) );
        REQUIRE( equal(entries[2], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input TEXT in state ERROR_BEFORE_MSGID", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

#
error_before_msgid
"text"
msgid "b"
msgstr "B"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "6,1: Unknown token.")) );
        REQUIRE( equal(entries[2], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input ERROR in state ERROR_BEFORE_MSGID", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

#
error_before_msgid
error_token
msgid "b"
msgstr "B"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "6,1: Unknown token.")) );
        REQUIRE( equal(entries[2], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input EOT in state ERROR_BEFORE_MSGID", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

#
error_before_msgid
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 2 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "6,1: Unknown token.")) );
    }
}


// State COMMENT
TEST_CASE( "Input COMMENT in state COMMENT", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

# comment1
# comment2
msgid "b"
msgstr "B"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("b", { "B" }, "")) );
        REQUIRE( equal(entries[2], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input FUZZY in state COMMENT", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

# comment
#, fuzzy
msgid "b"
msgstr "B"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("b", { "" }, "")) );
        REQUIRE( equal(entries[2], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input MSGCTXT in state COMMENT", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

# comment
msgctxt "cb"
msgid "b"
msgstr "B"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("cb" "\x04" "b", { "B" }, "")) );
        REQUIRE( equal(entries[2], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input MSGID in state COMMENT", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

# comment
msgid "b"
msgstr "B"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("b", { "B" }, "")) );
        REQUIRE( equal(entries[2], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input MSGID_PLURAL in state COMMENT", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

# comment
msgid_plural "bp"
msgstr "B"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "6,1: Unexpected msgid_plural.")) );
        REQUIRE( equal(entries[2], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input MSGSTR in state COMMENT", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

# comment
msgstr "B"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "6,1: Unexpected msgstr.")) );
        REQUIRE( equal(entries[2], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input MSGSTR_PLURAL in state COMMENT", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

# comment
msgstr[0] "B"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "6,1: Unexpected msgstr[n].")) );
        REQUIRE( equal(entries[2], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input TEXT in state COMMENT", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

# comment
"text"
msgid "b"
msgstr "B"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "6,1: Unexpected quoted text.")) );
        REQUIRE( equal(entries[2], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input ERROR in state COMMENT", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

# comment
error_token
msgid "b"
msgstr "B"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "6,1: Unknown token.")) );
        REQUIRE( equal(entries[2], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input EOT in state COMMENT", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

# comment
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 1 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
    }
}


// State MSGCTXT
TEST_CASE( "Input COMMENT in state MSGCTXT", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgctxt
# comment
msgid "b"
msgstr "B"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) { 
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "6,1: Unexpected comment.")) );
        REQUIRE( equal(entries[2], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input FUZZY in state MSGCTXT", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgctxt
#, fuzzy
msgid "b"
msgstr "B"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "6,1: Unexpected fuzzy flag comment.")) );
        REQUIRE( equal(entries[2], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input MSGCTXT in state MSGCTXT", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgctxt
msgctxt "cb"
msgid "b"
msgstr "B"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "6,1: Unexpected msgctxt.")) );
        REQUIRE( equal(entries[2], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input MSGID in state MSGCTXT", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgctxt
msgid "b"
msgstr "B"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "6,1: Unexpected msgid.")) );
        REQUIRE( equal(entries[2], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input MSGID_PLURAL in state MSGCTXT", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgctxt
msgid_plural "bp"
msgstr "B"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "6,1: Unexpected msgid_plural.")) );
        REQUIRE( equal(entries[2], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input MSGSTR in state MSGCTXT", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgctxt
msgstr "B"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "6,1: Unexpected msgstr.")) );
        REQUIRE( equal(entries[2], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input MSGSTR_PLURAL in state MSGCTXT", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgctxt
msgstr[0] "B"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "6,1: Unexpected msgstr[n].")) );
        REQUIRE( equal(entries[2], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input TEXT in state MSGCTXT", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgctxt
"text"
msgid "b"
msgstr "B"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("text" "\x04" "b", { "B" }, "")) );
        REQUIRE( equal(entries[2], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input ERROR in state MSGCTXT", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgctxt
error_token
msgid "b"
msgstr "B"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "6,1: Unknown token.")) );
        REQUIRE( equal(entries[2], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input EOT in state MSGCTXT", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgctxt
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 2 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "6,1: Unexpected EOT.")) );
    }
}


// State MSGID
TEST_CASE( "Input COMMENT in state MSGID", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid
# comment
msgid "b"
msgstr "B"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 4 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "6,1: Unexpected comment (the previous entry is incomplete).")) );
        REQUIRE( equal(entries[2], create("b", { "B" }, "")) );
        REQUIRE( equal(entries[3], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input FUZZY in state MSGID", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid
#, fuzzy
msgid "b"
msgstr "B"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 4 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "6,1: Unexpected fuzzy flag comment (the previous entry is incomplete).")) );
        REQUIRE( equal(entries[2], create("b", { "" }, "")) );
        REQUIRE( equal(entries[3], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input MSGCTXT in state MSGID", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid
msgctxt "cb"
msgid "b"
msgstr "B"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 4 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "6,1: Unexpected msgctxt (the previous entry is incomplete).")) );
        REQUIRE( equal(entries[2], create("cb" "\x04" "b", { "B" }, "")) );
        REQUIRE( equal(entries[3], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input MSGID in state MSGID", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid
msgid "b"
msgstr "B"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 4 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "6,1: Unexpected msgid (the previous entry is incomplete).")) );
        REQUIRE( equal(entries[2], create("b", { "B" }, "")) );
        REQUIRE( equal(entries[3], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input MSGID_PLURAL in state MSGID", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid
msgid_plural "bp"
msgstr "B"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "6,1: Unexpected msgid_plural.")) );
        REQUIRE( equal(entries[2], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input MSGSTR in state MSGID", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid
msgstr "B"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "6,1: Unexpected msgstr.")) );
        REQUIRE( equal(entries[2], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input MSGSTR_PLURAL in state MSGID", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid
msgstr[0] "B"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "6,1: Unexpected msgstr[n].")) );
        REQUIRE( equal(entries[2], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input TEXT in state MSGID", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid
"text"
msgstr "B"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("text", { "B" }, "")) );
        REQUIRE( equal(entries[2], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input ERROR in state MSGID", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid
error_token
msgstr "B"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "6,1: Unknown token.")) );
        REQUIRE( equal(entries[2], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input EOT in state MSGID", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 2 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "6,1: Unexpected EOT (the previous entry is incomplete).")) );
    }
}


// State MSGID_PLURAL
TEST_CASE( "Input COMMENT in state MSGID_PLURAL", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid "b"
msgid_plural
# comment
msgstr[0] "B"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 4 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "7,1: Unexpected comment (the previous entry is incomplete).")) );
        REQUIRE( equal(entries[2], create("", {}, "8,1: Unexpected msgstr[n].")) );
        REQUIRE( equal(entries[3], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input FUZZY in state MSGID_PLURAL", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid "b"
msgid_plural
#, fuzzy
msgstr[0] "B"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 4 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "7,1: Unexpected fuzzy flag comment (the previous entry is incomplete).")) );
        REQUIRE( equal(entries[2], create("", {}, "8,1: Unexpected msgstr[n].")) );
        REQUIRE( equal(entries[3], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input MSGCTXT in state MSGID_PLURAL", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid "b"
msgid_plural
msgctxt "cb"
msgstr[0] "B"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 4 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "7,1: Unexpected msgctxt (the previous entry is incomplete).")) );
        REQUIRE( equal(entries[2], create("", {}, "8,1: Unexpected msgstr[n].")) );
        REQUIRE( equal(entries[3], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input MSGID in state MSGID_PLURAL", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid "b"
msgid_plural
msgid "b2"
msgstr "B"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 4 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "7,1: Unexpected msgid (the previous entry is incomplete).")) );
        REQUIRE( equal(entries[2], create("b2", { "B" }, "")) );
        REQUIRE( equal(entries[3], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input MSGID_PLURAL in state MSGID_PLURAL", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid "b"
msgid_plural
msgid_plural "bp"
msgstr "B"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "7,1: Unexpected msgid_plural.")) );
        REQUIRE( equal(entries[2], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input MSGSTR in state MSGID_PLURAL", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid "b"
msgid_plural
msgstr "B"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "7,1: Unexpected msgstr.")) );
        REQUIRE( equal(entries[2], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input MSGSTR_PLURAL in state MSGID_PLURAL", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid "b"
msgid_plural
msgstr[0] "B"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "7,1: Unexpected msgstr[n].")) );
        REQUIRE( equal(entries[2], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input TEXT in state MSGID_PLURAL", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid "b"
msgid_plural
"text"
msgstr[0] "B"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("b", { "B" }, "")) );
        REQUIRE( equal(entries[2], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input ERROR in state MSGID_PLURAL", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid "b"
msgid_plural
error_token
msgstr[0] "B"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "7,1: Unknown token.")) );
        REQUIRE( equal(entries[2], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input EOT in state MSGID_PLURAL", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid "b"
msgid_plural
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 2 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "7,1: Unexpected EOT (the previous entry is incomplete).")) );
    }
}


// State MSGSTR
TEST_CASE( "Input COMMENT in state MSGSTR", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid "b"
msgstr
# comment
"B"
msgid "b2"
msgstr "B2"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 4 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "7,1: Unexpected comment (the previous entry is incomplete).")) );
        REQUIRE( equal(entries[2], create("", {}, "8,1: Unexpected quoted text.")) );
        REQUIRE( equal(entries[3], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input FUZZY in state MSGSTR", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid "b"
msgstr
#, fuzzy
"B"
msgid "b2"
msgstr "B2"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 4 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "7,1: Unexpected fuzzy flag comment (the previous entry is incomplete).")) );
        REQUIRE( equal(entries[2], create("", {}, "8,1: Unexpected quoted text.")) );
        REQUIRE( equal(entries[3], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input MSGCTXT in state MSGSTR", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid "b"
msgstr
msgctxt "cb"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "7,1: Unexpected msgctxt (the previous entry is incomplete).")) );
        REQUIRE( equal(entries[2], create("cb" "\x04" "c", { "C" }, "")) );
    }
}


TEST_CASE( "Input MSGID in state MSGSTR", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid "b"
msgstr
msgid "b2"
msgstr "B"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 4 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "7,1: Unexpected msgid (the previous entry is incomplete).")) );
        REQUIRE( equal(entries[2], create("b2", { "B" }, "")) );
        REQUIRE( equal(entries[3], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input MSGID_PLURAL in state MSGSTR", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid "b"
msgstr
msgid_plural "bp"
msgstr "B"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "7,1: Unexpected msgid_plural.")) );
        REQUIRE( equal(entries[2], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input MSGSTR in state MSGSTR", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid "b"
msgstr
msgstr "B"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "7,1: Unexpected msgstr.")) );
        REQUIRE( equal(entries[2], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input MSGSTR_PLURAL in state MSGSTR", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid "b"
msgstr
msgstr[0] "B"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "7,1: Unexpected msgstr[n].")) );
        REQUIRE( equal(entries[2], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input TEXT in state MSGSTR", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid "b"
msgstr
"text"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("b", { "text" }, "")) );
        REQUIRE( equal(entries[2], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input ERROR in state MSGSTR", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid "b"
msgstr
error_token

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "7,1: Unknown token.")) );
        REQUIRE( equal(entries[2], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input EOT in state MSGSTR", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid "b"
msgstr
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 2 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "7,1: Unexpected EOT (the previous entry is incomplete).")) );
    }
}


// State MSGSTR_PLURAL
TEST_CASE( "Input COMMENT in state MSGSTR_PLURAL", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid "b"
msgid_plural "bp"
msgstr[0]
# comment
"B"
msgid "b2"
msgstr "B2"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 4 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "8,1: Unexpected comment (the previous entry is incomplete).")) );
        REQUIRE( equal(entries[2], create("", {}, "9,1: Unexpected quoted text.")) );
        REQUIRE( equal(entries[3], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input FUZZY in state MSGSTR_PLURAL", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid "b"
msgid_plural "bp"
msgstr[0]
#, fuzzy
"B"
msgid "b2"
msgstr "B2"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 4 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "8,1: Unexpected fuzzy flag comment (the previous entry is incomplete).")) );
        REQUIRE( equal(entries[2], create("", {}, "9,1: Unexpected quoted text.")) );
        REQUIRE( equal(entries[3], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input MSGCTXT in state MSGSTR_PLURAL", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid "b"
msgid_plural "bp"
msgstr[0]
msgctxt "cb"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "8,1: Unexpected msgctxt (the previous entry is incomplete).")) );
        REQUIRE( equal(entries[2], create("cb" "\x04" "c", { "C" }, "")) );
    }
}


TEST_CASE( "Input MSGID in state MSGSTR_PLURAL", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid "b"
msgid_plural "bp"
msgstr[0]
msgid "b2"
msgstr "B"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 4 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "8,1: Unexpected msgid (the previous entry is incomplete).")) );
        REQUIRE( equal(entries[2], create("b2", { "B" }, "")) );
        REQUIRE( equal(entries[3], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input MSGID_PLURAL in state MSGSTR_PLURAL", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid "b"
msgid_plural "bp"
msgstr[0]
msgid_plural "bp"
msgstr "B"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "8,1: Unexpected msgid_plural.")) );
        REQUIRE( equal(entries[2], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input MSGSTR in state MSGSTR_PLURAL", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid "b"
msgid_plural "bp"
msgstr[0]
msgstr "B"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "8,1: Unexpected msgstr.")) );
        REQUIRE( equal(entries[2], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input MSGSTR_PLURAL in state MSGSTR_PLURAL", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid "b"
msgid_plural "b["
msgstr[0]
msgstr[1] "B"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "8,1: Unexpected msgstr[n].")) );
        REQUIRE( equal(entries[2], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input TEXT in state MSGSTR_PLURAL", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid "b"
msgid_plural "bp"
msgstr[0]
"text"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("b", { "text" }, "")) );
        REQUIRE( equal(entries[2], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input ERROR in state MSGSTR_PLURAL", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid "b"
msgid_plural "bp"
msgstr[0]
error_token

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "8,1: Unknown token.")) );
        REQUIRE( equal(entries[2], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input EOT in state MSGSTR_PLURAL", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid "b"
msgid_plural "bp"
msgstr[0]
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 2 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "8,1: Unexpected EOT (the previous entry is incomplete).")) );
    }
}


// State MSGCTXT_TEXT
TEST_CASE( "Input COMMENT in state MSGCTXT_TEXT", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgctxt "cb"
# comment
msgid "b"
msgstr "B"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "6,1: Unexpected comment.")) );
        REQUIRE( equal(entries[2], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input FUZZY in state MSGCTXT_TEXT", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgctxt "cb"
#, fuzzy
msgid "b"
msgstr "B"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "6,1: Unexpected fuzzy flag comment.")) );
        REQUIRE( equal(entries[2], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input MSGCTXT in state MSGCTXT_TEXT", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgctxt "cb"
msgctxt "cb2"
msgid "b"
msgstr "B"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "6,1: Unexpected msgctxt.")) );
        REQUIRE( equal(entries[2], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input MSGID in state MSGCTXT_TEXT", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgctxt "cb"
msgid "b"
msgstr "B"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("cb" "\x04" "b", { "B" }, "")) );
        REQUIRE( equal(entries[2], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input MSGID_PLURAL in state MSGCTXT_TEXT", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgctxt "cb"
msgid_plural "bp"
msgstr[0] "B"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "6,1: Unexpected msgid_plural.")) );
        REQUIRE( equal(entries[2], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input MSGSTR in state MSGCTXT_TEXT", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgctxt "cb"
msgstr "B"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "6,1: Unexpected msgstr.")) );
        REQUIRE( equal(entries[2], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input MSGSTR_PLURAL in state MSGCTXT_TEXT", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgctxt "cb"
msgstr[0] "B"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "6,1: Unexpected msgstr[n].")) );
        REQUIRE( equal(entries[2], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input TEXT in state MSGCTXT_TEXT", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgctxt "cb"
"text"
msgid "b"
msgstr "B"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("cbtext" "\x04" "b", { "B" }, "")) );
        REQUIRE( equal(entries[2], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input ERROR in state MSGCTXT_TEXT", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgctxt "cb"
error_token
msgid "b"
msgstr "B"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "6,1: Unknown token.")) );
        REQUIRE( equal(entries[2], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input EOT in state MSGCTXT_TEXT", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgctxt "cb"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 2 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "6,1: Unexpected EOT.")) );
    }
}


// State MSGID_TEXT
TEST_CASE( "Input COMMENT in state MSGID_TEXT", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid "b"
# comment
msgstr "B"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 4 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "6,1: Unexpected comment (the previous entry is incomplete).")) );
        REQUIRE( equal(entries[2], create("", {}, "7,1: Unexpected msgstr.")) );
        REQUIRE( equal(entries[3], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input FUZZY in state MSGID_TEXT", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid "b"
#, fuzzy
msgstr "B"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 4 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "6,1: Unexpected fuzzy flag comment (the previous entry is incomplete).")) );
        REQUIRE( equal(entries[2], create("", {}, "7,1: Unexpected msgstr.")) );
        REQUIRE( equal(entries[3], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input MSGCTXT in state MSGID_TEXT", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid "b"
msgctxt "cb"
msgstr "B"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 4 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "6,1: Unexpected msgctxt (the previous entry is incomplete).")) );
        REQUIRE( equal(entries[2], create("", {}, "7,1: Unexpected msgstr.")) );
        REQUIRE( equal(entries[3], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input MSGID in state MSGID_TEXT", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid "b"
msgid "b2"
msgstr "B"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 4 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "6,1: Unexpected msgid (the previous entry is incomplete).")) );
        REQUIRE( equal(entries[2], create("b2", { "B" }, "")) );
        REQUIRE( equal(entries[3], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input MSGID_PLURAL in state MSGID_TEXT", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid "b"
msgid_plural "bp"
msgstr[0] "B"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("b", { "B" }, "")) );
        REQUIRE( equal(entries[2], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input MSGSTR in state MSGID_TEXT", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid "b"
msgstr "B"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("b", { "B" }, "")) );
        REQUIRE( equal(entries[2], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input MSGSTR_PLURAL in state MSGID_TEXT", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid "b"
msgstr[0] "B"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "6,1: Unexpected msgstr[n].")) );
        REQUIRE( equal(entries[2], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input TEXT in state MSGID_TEXT", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid "b"
"text"
msgstr "B"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("btext", { "B" }, "")) );
        REQUIRE( equal(entries[2], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input ERROR in state MSGID_TEXT", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid "b"
error_token
msgstr "B"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "6,1: Unknown token.")) );
        REQUIRE( equal(entries[2], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input EOT in state MSGID_TEXT", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid "b"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 2 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "6,1: Unexpected EOT (the previous entry is incomplete).")) );
    }
}


// State MSGID_PLURAL_TEXT
TEST_CASE( "Input COMMENT in state MSGID_PLURAL_TEXT", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid "b"
msgid_plural "bp"
# comment
msgstr[0] "B"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 4 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "7,1: Unexpected comment (the previous entry is incomplete).")) );
        REQUIRE( equal(entries[2], create("", {}, "8,1: Unexpected msgstr[n].")) );
        REQUIRE( equal(entries[3], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input FUZZY in state MSGID_PLURAL_TEXT", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid "b"
msgid_plural "bp"
#, fuzzy
msgstr[0] "B"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 4 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "7,1: Unexpected fuzzy flag comment (the previous entry is incomplete).")) );
        REQUIRE( equal(entries[2], create("", {}, "8,1: Unexpected msgstr[n].")) );
        REQUIRE( equal(entries[3], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input MSGCTXT in state MSGID_PLURAL_TEXT", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid "b"
msgid_plural "bp"
msgctxt "cb"
msgstr[0] "B"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 4 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "7,1: Unexpected msgctxt (the previous entry is incomplete).")) );
        REQUIRE( equal(entries[2], create("", {}, "8,1: Unexpected msgstr[n].")) );
        REQUIRE( equal(entries[3], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input MSGID in state MSGID_PLURAL_TEXT", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid "b"
msgid_plural "bp"
msgid "b2"
msgstr "B"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 4 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "7,1: Unexpected msgid (the previous entry is incomplete).")) );
        REQUIRE( equal(entries[2], create("b2", { "B" }, "")) );
        REQUIRE( equal(entries[3], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input MSGID_PLURAL in state MSGID_PLURAL_TEXT", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid "b"
msgid_plural "bp"
msgid_plural "bp2"
msgstr[0] "B"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "7,1: Unexpected msgid_plural.")) );
        REQUIRE( equal(entries[2], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input MSGSTR in state MSGID_PLURAL_TEXT", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid "b"
msgid_plural "bp"
msgstr "B"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "7,1: Unexpected msgstr.")) );
        REQUIRE( equal(entries[2], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input MSGSTR_PLURAL in state MSGID_PLURAL_TEXT", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid "b"
msgid_plural "bp"
msgstr[0] "B"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("b", { "B" }, "")) );
        REQUIRE( equal(entries[2], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input TEXT in state MSGID_PLURAL_TEXT", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid "b"
msgid_plural "bp"
"text"
msgstr[0] "B"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("b", { "B" }, "")) );
        REQUIRE( equal(entries[2], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input ERROR in state MSGID_PLURAL_TEXT", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid "b"
msgid_plural "bp"
error_token
msgstr[0] "B"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "7,1: Unknown token.")) );
        REQUIRE( equal(entries[2], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input EOT in state MSGID_PLURAL_TEXT", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid "b"
msgid_plural "bp"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 2 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "7,1: Unexpected EOT (the previous entry is incomplete).")) );
    }
}


// State MSGSTR_TEXT
TEST_CASE( "Input COMMENT in state MSGSTR_TEXT", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid "b"
msgstr "B"
# comment

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("b", { "B" }, "")) );
        REQUIRE( equal(entries[2], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input FUZZY in state MSGSTR_TEXT", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid "b"
msgstr "B"
#, fuzzy

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("b", { "B" }, "")) );
        REQUIRE( equal(entries[2], create("c", { "" }, "")) );
    }
}


TEST_CASE( "Input MSGCTXT in state MSGSTR_TEXT", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid "b"
msgstr "B"
msgctxt "cb"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("b", { "B" }, "")) );
        REQUIRE( equal(entries[2], create("cb" "\x04" "c", { "C" }, "")) );
    }
}


TEST_CASE( "Input MSGID in state MSGSTR_TEXT", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid "b"
msgstr "B"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("b", { "B" }, "")) );
        REQUIRE( equal(entries[2], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input MSGID_PLURAL in state MSGSTR_TEXT", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid "b"
msgstr "B"
msgid_plural "bp"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "7,1: Unexpected msgid_plural.")) );
        REQUIRE( equal(entries[2], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input MSGSTR in state MSGSTR_TEXT", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid "b"
msgstr "B"
msgstr "B2"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "7,1: Unexpected msgstr.")) );
        REQUIRE( equal(entries[2], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input MSGSTR_PLURAL in state MSGSTR_TEXT", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid "b"
msgstr "B"
msgstr[0] "B2"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "7,1: Unexpected msgstr[n].")) );
        REQUIRE( equal(entries[2], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input TEXT in state MSGSTR_TEXT", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid "b"
msgstr "B"
"text"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("b", { "Btext" }, "")) );
        REQUIRE( equal(entries[2], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input ERROR in state MSGSTR_TEXT", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid "b"
msgstr "B"
error_token

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "7,1: Unknown token.")) );
        REQUIRE( equal(entries[2], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input EOT in state MSGSTR_TEXT", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid "b"
msgstr "B"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 2 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("b", { "B" }, "")) );
    }
}


// State MSGSTR_PLURAL_TEXT
TEST_CASE( "Input COMMENT in state MSGSTR_PLURAL_TEXT", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid "b"
msgid_plural "bp"
msgstr[0] "B"
# comment

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("b", { "B" }, "")) );
        REQUIRE( equal(entries[2], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input FUZZY in state MSGSTR_PLURAL_TEXT", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid "b"
msgid_plural "bp"
msgstr[0] "B"
#, fuzzy

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("b", { "B" }, "")) );
        REQUIRE( equal(entries[2], create("c", { "" }, "")) );
    }
}


TEST_CASE( "Input MSGCTXT in state MSGSTR_PLURAL_TEXT", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid "b"
msgid_plural "bp"
msgstr[0] "B"
msgctxt "cb"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("b", { "B" }, "")) );
        REQUIRE( equal(entries[2], create("cb" "\x04" "c", { "C" }, "")) );
    }
}


TEST_CASE( "Input MSGID in state MSGSTR_PLURAL_TEXT", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid "b"
msgid_plural "bp"
msgstr[0] "B"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("b", { "B" }, "")) );
        REQUIRE( equal(entries[2], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input MSGID_PLURAL in state MSGSTR_PLURAL_TEXT", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid "b"
msgid_plural "b"
msgstr[0] "B"
msgid_plural "bp"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "8,1: Unexpected msgid_plural.")) );
        REQUIRE( equal(entries[2], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input MSGSTR in state MSGSTR_PLURAL_TEXT", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid "b"
msgid_plural "bp"
msgstr[0] "B"
msgstr "B2"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "8,1: Unexpected msgstr.")) );
        REQUIRE( equal(entries[2], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input MSGSTR_PLURAL in state MSGSTR_PLURAL_TEXT", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid "b"
msgid_plural "bp"
msgstr[0] "B"
msgstr[1] "B2"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("b", { "B", "B2" }, "")) );
        REQUIRE( equal(entries[2], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input TEXT in state MSGSTR_PLURAL_TEXT", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid "b"
msgid_plural "bp"
msgstr[0] "B"
"text"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("b", { "Btext" }, "")) );
        REQUIRE( equal(entries[2], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input ERROR in state MSGSTR_PLURAL_TEXT", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid "b"
msgid_plural "bp"
msgstr[0] "B"
error_token

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "8,1: Unknown token.")) );
        REQUIRE( equal(entries[2], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input EOT in state MSGSTR_PLURAL_TEXT", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid "b"
msgid_plural "bp"
msgstr[0] "B"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 2 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("b", { "B" }, "")) );
    }
}


// State ERROR
TEST_CASE( "Input COMMENT in state ERROR", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid error_state
# comment

msgid "b"
msgstr "B"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 4 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "5,7: Unknown token.")) );
        REQUIRE( equal(entries[2], create("b", { "B" }, "")) );
        REQUIRE( equal(entries[3], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input FUZZY in state ERROR", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid error_state
#, fuzzy

msgid "b"
msgstr "B"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 4 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "5,7: Unknown token.")) );
        REQUIRE( equal(entries[2], create("b", { "" }, "")) );
        REQUIRE( equal(entries[3], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input MSGCTXT in state ERROR", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid error_state

msgctxt "cb"
msgid "b"
msgstr "B"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 4 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "5,7: Unknown token.")) );
        REQUIRE( equal(entries[2], create("cb" "\x04" "b", { "B" }, "")) );
        REQUIRE( equal(entries[3], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input MSGID in state ERROR", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid error_state

msgid "b"
msgstr "B"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 4 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "5,7: Unknown token.")) );
        REQUIRE( equal(entries[2], create("b", { "B" }, "")) );
        REQUIRE( equal(entries[3], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input MSGID_PLURAL in state ERROR", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid error_state

msgid_plural "bp"
msgstr[0] "B"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "5,7: Unknown token.")) );
        REQUIRE( equal(entries[2], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input MSGSTR in state ERROR", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid error_state
msgstr "B"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "5,7: Unknown token.")) );
        REQUIRE( equal(entries[2], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input MSGSTR_PLURAL in state ERROR", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid error_state
msgstr[0] "B"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "5,7: Unknown token.")) );
        REQUIRE( equal(entries[2], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input TEXT in state ERROR", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid error_state
"text"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "5,7: Unknown token.")) );
        REQUIRE( equal(entries[2], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input ERROR in state ERROR", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid error_state
error_token

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "5,7: Unknown token.")) );
        REQUIRE( equal(entries[2], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "Input EOT in state ERROR", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid error_state
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 2 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "5,7: Unknown token.")) );
    }
}



TEST_CASE( "Duplicate entry", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid "b"
msgstr "B"

msgctxt "context"
msgid "a"
msgstr "cA"

msgid "b"
msgstr "B2"

msgctxt ""
msgid "b"
msgstr "B3"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        // PoParser doesn't care the duplicate entries.
        REQUIRE( entries.size() == 5 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("b", { "B" }, "")) );
        REQUIRE( equal(entries[2], create("context" "\x04" "a", { "cA" }, "")) );
        REQUIRE( equal(entries[3], create("b", { "B2" }, "")) );
        REQUIRE( equal(entries[4], create("" "\x04" "b", { "B3" }, "")) );
    }
}


TEST_CASE( "Inconsistent n for msgstr[n]", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid "b1"
msgid_plural "bp1"
msgstr[0] "B0"
msgstr[1] "B1"
msgstr[2] "B2"

msgid "c1"
msgid_plural "cp1"
msgstr[1] "C1"
msgstr[2] "C2"

msgid "d1"
msgid_plural "dp1"
msgstr[0] "D0"
msgstr[2] "D2"

msgid "e1"
msgid_plural "ep1"
msgstr[0] "E0"
msgstr[0] "E1"

msgid "f"
msgstr "F"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        // PoParser doesn't care the duplicate entries.
        REQUIRE( entries.size() == 6 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("b1", { "B0", "B1", "B2" }, "")) );
        REQUIRE( equal(entries[2], create("", {}, "13,1: Invalid n in msgstr[n]; n should be 0 but 1.")) );
        REQUIRE( equal(entries[3], create("", {}, "19,1: Invalid n in msgstr[n]; n should be 1 but 2.")) );
        REQUIRE( equal(entries[4], create("", {}, "24,1: Invalid n in msgstr[n]; n should be 1 but 0.")) );
        REQUIRE( equal(entries[5], create("f", { "F" }, "")) );
    }
}


TEST_CASE( "newline in text", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid "b"
msgstr ""
"newline error
next line"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "7,15: The text may not contain a newline.")) );
        REQUIRE( equal(entries[2], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "newline and EOT in text", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid "b"
msgstr ""
"newline error
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 2 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "7,15: The text may not contain a newline.")) );
    }
}


TEST_CASE( "escape EOT in text", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid "b"
msgstr ""
"\)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 2 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "7,3: Invalid escape sequence.")) );
    }
}


TEST_CASE( "escape sequences in text", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid "b"
msgstr "1\a2\b3\f4\n5\r6\t7\v8\\9\'A\"B\?C"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("b", { "1\a2\b3\f4\n5\r6\t7\v8\\9\'A\"B?C" }, "")) );
        REQUIRE( equal(entries[2], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "invalid escape sequence in text", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid "b"
msgstr "1\c2"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "6,11: Invalid escape sequence.")) );
        REQUIRE( equal(entries[2], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "escape sequence octal in text", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid "b"
msgstr "x\7y"
"x\17y"
"x\117y"
"x\1171y"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("b", { "x" "\x07" "yx" "\x0F" "yx" "\x4F" "yx" "\x4F" "1y" }, "")) );
        REQUIRE( equal(entries[2], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "escape sequence octal and EOT in text 1", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid "b"
msgstr "x\1)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 2 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "6,12: Closing double quotation mark is expected.")) );
    }
}


TEST_CASE( "escape sequence octal and EOT in text 2", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid "b"
msgstr "x\11)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 2 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "6,13: Closing double quotation mark is expected.")) );
    }
}


TEST_CASE( "escape sequence octal and EOT in text 3", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid "b"
msgstr "x\111)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 2 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "6,14: Closing double quotation mark is expected.")) );
    }
}


TEST_CASE( "escape sequence octal and EOT in text 4", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid "b"
msgstr "x\1111)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 2 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "6,15: Closing double quotation mark is expected.")) );
    }
}


TEST_CASE( "escape sequence hex in text", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid "b01"
msgstr "x\xy"

msgid "b02"
msgstr "x\x"

msgid "b"
msgstr "x\xay"
"x\xaby"
"x\xaBcy"
"x\x0123456789ABCDEEFabcdefy"

msgid "c"
msgstr "C"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 5 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "6,12: [0-9A-Fa-f] is expected.")) );
        REQUIRE( equal(entries[2], create("", {}, "9,12: [0-9A-Fa-f] is expected.")) );
        REQUIRE( equal(entries[3], create("b", { "x" "\x0A" "yx" "\xAB" "yx" "\xBC" "yx" "\xEF" "y" }, "")) );
        REQUIRE( equal(entries[4], create("c", { "C" }, "")) );
    }
}


TEST_CASE( "escape sequence hex and EOT in text 0", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid "b"
msgstr "x\x)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 2 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "6,12: [0-9A-Fa-f] is expected.")) );
    }
}


TEST_CASE( "escape sequence hex and EOT in text 1", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid "b"
msgstr "x\xA)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 2 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "6,13: Closing double quotation mark is expected.")) );
    }
}


TEST_CASE( "escape sequence hex and EOT in text 2", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid "b"
msgstr "x\xAb)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 2 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "6,14: Closing double quotation mark is expected.")) );
    }
}


TEST_CASE( "escape sequence hex and EOT in text 3", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid "b"
msgstr "x\xAbC)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 2 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "6,15: Closing double quotation mark is expected.")) );
    }
}


TEST_CASE( "escape sequence hex and EOT in text 4", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "A"

msgid "b"
msgstr "x\x0123456789ABCDEFabcdef)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 2 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("", {}, "6,34: Closing double quotation mark is expected.")) );
    }
}


TEST_CASE( "escape sequence hex utf-8 encoding", "[PoParser]" ) {
    const string po_text = R"(
msgid "a"
msgstr "あ\xe3\x81" "\x84" "0"

msgid "b"
msgstr "B\343\201\2061\343\201\210"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 2 );
        REQUIRE( equal(entries[0], create("a", { "あい0" }, "")) );
        REQUIRE( equal(entries[1], create("b", { "Bう1え" }, "")) );
    }
}


TEST_CASE( "no white space", "[PoParser]" ) {
    const string po_text = R"(msgid"a"msgstr"A"#comment
#:a:10 b:20
#,fuzzy
#|msgctxt"c""p""b"msgid"p""b"
msgctxt"c""b"msgid"b"msgstr"B"#|msgctxt"c""p""c"msgid"p""c"msgid_plural"p""c""p"
msgctxt"c""c"msgid"c"msgid_plural"c""p"msgstr[0]"C""0"msgstr[1]"C""1"#~#|msgid"pd"
#~msgid"d"msgstr"D"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("cb" "\x04" "b", { "" }, "")) );
        REQUIRE( equal(entries[2], create("cc" "\x04" "c", { "C0", "C1" }, "")) );
    }
}


TEST_CASE( "too many white spaces", "[PoParser]" ) {
    const string po_text = R"(   	
        	 msgid       "a"
	        msgstr    "A"




        #  comment



                 	#:   	a:10 	b:20

        #,   fuzzy


        #|   msgctxt"c" 	


        	#| "p"

        #|	"b"

    #|	msgid


        #| "p"

        #| "b" 	





        		msgctxt

        "c"	

        	"b"


        msgid

        "b"

        msgstr


        "B"

        #|     msgctxt
        #| 	"c" 	 "p"	

        #|	"c"




        #| msgid	"p"


        #|	"c"
        #|			msgid_plural
        #|
        #|
        #|
        #| 	"p"

        #| "c"
        #|		"p"



        msgctxt			"c"
        "c"

        msgid
        	"c"

        			msgid_plural	"c"	"p"


        msgstr 	  [    	0  	]
        "C"	"0"

        msgstr
  	      
        [
    	    
        1
        	
        ]
        "C"


        "1"

        #~		#|	msgid	"pd"	



   #~   msgid
        #~  "d"        	msgstr
        #~ 			"D"
)";
    auto entries = PoParser::GetEntries(po_text.begin(), po_text.end());
    SECTION( "entries" ) {
        REQUIRE( entries.size() == 3 );
        REQUIRE( equal(entries[0], create("a", { "A" }, "")) );
        REQUIRE( equal(entries[1], create("cb" "\x04" "b", { "" }, "")) );
        REQUIRE( equal(entries[2], create("cc" "\x04" "c", { "C0", "C1" }, "")) );
    }
}
