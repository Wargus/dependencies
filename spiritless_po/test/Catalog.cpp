/*
  Copyright © 2022, 2024, 2026 OOTA, Masato
  License: CC-BY-SA-3.0
  See https://creativecommons.org/licenses/by-sa/3.0/legalcode for license details.
*/
#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>

#include "spiritless_po.h"

using namespace std;
using namespace spiritless_po;

bool equal(const Catalog::StatisticsT &a, const Catalog::StatisticsT &b)
{
    return a.totalCount == b.totalCount
        && a.metadataCount == b.metadataCount
        && a.translatedCount == b.translatedCount
        && a.discardedCount == b.discardedCount;
}

bool equal(const Catalog::IndexDataT &a, const Catalog::IndexDataT &b)
{
    return a.stringTableIndex == b.stringTableIndex && a.totalPlurals == b.totalPlurals;
}

bool equal(const unordered_map<string, Catalog::IndexDataT>& a, const unordered_map<string, Catalog::IndexDataT>& b)
{
    if (a.size() != b.size()) {
        return false;
    }
    for (auto elm : a) {
        auto it2 = b.find(elm.first);
        if (it2 == b.end() || !equal(elm.second, it2->second)) {
            return false;
        }
    }
    return true;
}

void dump_catalog(const Catalog &catalog)
{
    cout << "String Table:" << endl;
    size_t no = 0;
    for (auto s : catalog.GetStringTable()) {
        cout << no << ": \"" << s << "\"\n";
        ++no;
    }
    cout << "Index:" << endl;
    for (auto idx : catalog.GetIndex()) {
        auto base = idx.second.stringTableIndex;
        auto total = idx.second.totalPlurals;
        auto s = idx.first;
        auto pos = s.find("\04");
        cout << '"';
        if (pos == s.npos) {
            cout << s;
        } else {
            cout << s.substr(0, pos) << "\" / \"";
            cout << s.substr(pos + 1);
        }
        cout << "\": " << base;
        if (total == 1) {
            cout << endl;
        } else {
            cout << " - " << (base + total - 1) << endl;
        }
    }
    cout << "Metadata:" << endl;
    for (auto entry : catalog.GetMetadata()) {
        cout << entry.first << ": " << entry.second << endl;
    }
    cout << "Errors:" << endl;
    for (const auto &s : catalog.GetError()) {
        cerr << s << endl;
    }
}



TEST_CASE( "Default Constructor in Catalog", "[Catalog]" ) {
    Catalog catalog;
    string singular("apple");
    string plural("apples");

    REQUIRE( catalog.GetError().size() == 0 );
    REQUIRE( catalog.GetMetadata().size() == 0 );
    REQUIRE( catalog.GetIndex().size() == 0 );
    REQUIRE( catalog.GetStringTable().size() == 0 );
    REQUIRE( catalog.GetStatistics().totalCount == 0 );
    REQUIRE( catalog.GetStatistics().metadataCount == 0 );
    REQUIRE( catalog.GetStatistics().translatedCount == 0 );
    REQUIRE( catalog.GetStatistics().discardedCount == 0 );
    REQUIRE( &catalog.gettext(singular) == &singular );
    REQUIRE( &catalog.ngettext(singular, plural, 1) == &singular );
    REQUIRE( &catalog.ngettext(singular, plural, 2) == &plural );
    REQUIRE( &catalog.pgettext("context", singular) == &singular );
    REQUIRE( &catalog.npgettext("context", singular, plural, 1) == &singular );
    REQUIRE( &catalog.npgettext("context", singular, plural, 2) == &plural );
}


const string test_data = R"(# translator-comments
#. extracted-comment
#: references
#, flags
#| msgid previous-untranslated-string
msgid ""
msgstr "Project-Id-Version: test-data\n"
"Plural-Forms: nplurals=5; plural=n;\n"

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

msgid "error"
)";

TEST_CASE( "Constructor(begin, end) in Catalog", "[Catalog]" ) {
    Catalog catalog(test_data.begin(), test_data.end());
    string singular("ant");
    string plural("ants");

    REQUIRE( catalog.GetError().size() == 1 );
    REQUIRE( catalog.GetMetadata().size() == 2 );
    REQUIRE( catalog.GetIndex().size() == 6 );
    REQUIRE( catalog.GetStringTable().size() == 13 );
    REQUIRE( catalog.GetStatistics().totalCount == 7 );
    REQUIRE( catalog.GetStatistics().metadataCount == 1 );
    REQUIRE( catalog.GetStatistics().translatedCount == 6 );
    REQUIRE( catalog.GetStatistics().discardedCount == 1 );
    REQUIRE( &catalog.gettext(singular) == &singular );
    REQUIRE( &catalog.ngettext(singular, plural, 1) == &singular );
    REQUIRE( &catalog.ngettext(singular, plural, 2) == &plural );
    REQUIRE( &catalog.pgettext("context", singular) == &singular );
    REQUIRE( &catalog.npgettext("context", singular, plural, 1) == &singular );
    REQUIRE( &catalog.npgettext("context", singular, plural, 2) == &plural );
    REQUIRE( catalog.gettext("apples") == "APPLES" );
    REQUIRE( catalog.ngettext("corn", "corns", 1) == "CORN#1" );
    REQUIRE( catalog.ngettext("corn", "corns", 2) == "CORN#2" );
    REQUIRE( catalog.ngettext("corn", "corns", 4) == "CORN#0" );
    REQUIRE( catalog.pgettext("food", "eggs") == "EGGS" );
    REQUIRE( catalog.npgettext("food", "garlic", "garlics", 1) == "GARLIC#1" );
    REQUIRE( catalog.npgettext("food", "garlic", "garlics", 2) == "GARLIC#2" );
    REQUIRE( catalog.npgettext("food", "garlic", "garlics", 4) == "GARLIC#4" );
}


TEST_CASE( "Constructor(is) in Catalog", "[Catalog]" ) {
    istringstream is(test_data);
    Catalog catalog(is);
    string singular("ant");
    string plural("ants");

    REQUIRE( catalog.GetError().size() == 1 );
    REQUIRE( catalog.GetMetadata().size() == 2 );
    REQUIRE( catalog.GetIndex().size() == 6 );
    REQUIRE( catalog.GetStringTable().size() == 13 );
    REQUIRE( catalog.GetStatistics().totalCount == 7 );
    REQUIRE( catalog.GetStatistics().metadataCount == 1 );
    REQUIRE( catalog.GetStatistics().translatedCount == 6 );
    REQUIRE( catalog.GetStatistics().discardedCount == 1 );
    REQUIRE( &catalog.gettext(singular) == &singular );
    REQUIRE( &catalog.ngettext(singular, plural, 1) == &singular );
    REQUIRE( &catalog.ngettext(singular, plural, 2) == &plural );
    REQUIRE( &catalog.pgettext("context", singular) == &singular );
    REQUIRE( &catalog.npgettext("context", singular, plural, 1) == &singular );
    REQUIRE( &catalog.npgettext("context", singular, plural, 2) == &plural );
    REQUIRE( catalog.gettext("apples") == "APPLES" );
    REQUIRE( catalog.ngettext("corn", "corns", 1) == "CORN#1" );
    REQUIRE( catalog.ngettext("corn", "corns", 2) == "CORN#2" );
    REQUIRE( catalog.ngettext("corn", "corns", 4) == "CORN#0" );
    REQUIRE( catalog.pgettext("food", "eggs") == "EGGS" );
    REQUIRE( catalog.npgettext("food", "garlic", "garlics", 1) == "GARLIC#1" );
    REQUIRE( catalog.npgettext("food", "garlic", "garlics", 2) == "GARLIC#2" );
    REQUIRE( catalog.npgettext("food", "garlic", "garlics", 4) == "GARLIC#4" );
}


TEST_CASE( "Copy constructor in Catalog", "[Catalog]" ) {
    Catalog catalog(test_data.begin(), test_data.end());
    Catalog catalog2(catalog);

    REQUIRE( catalog.GetError() == catalog2.GetError() );
    REQUIRE( catalog.GetMetadata() == catalog2.GetMetadata() );
    REQUIRE( equal(catalog.GetIndex(), catalog2.GetIndex()) );
    REQUIRE( catalog.GetStringTable() == catalog2.GetStringTable() );
    REQUIRE( equal(catalog.GetStatistics(), catalog2.GetStatistics()) );
}


TEST_CASE( "Move constructor in Catalog", "[Catalog]" ) {
    Catalog catalog(test_data.begin(), test_data.end());
    Catalog catalog2(catalog);
    Catalog catalog3(std::move(catalog2));

    REQUIRE( catalog.GetError() == catalog3.GetError() );
    REQUIRE( catalog.GetMetadata() == catalog3.GetMetadata() );
    REQUIRE( equal(catalog.GetIndex(), catalog3.GetIndex()) );
    REQUIRE( catalog.GetStringTable() == catalog3.GetStringTable() );
    REQUIRE( equal(catalog.GetStatistics(), catalog3.GetStatistics()) );
}


TEST_CASE( "Assign operator in Catalog", "[Catalog]" ) {
    Catalog catalog(test_data.begin(), test_data.end());
    Catalog catalog2;
    catalog2 = catalog;

    REQUIRE( catalog.GetError() == catalog2.GetError() );
    REQUIRE( catalog.GetMetadata() == catalog2.GetMetadata() );
    REQUIRE( equal(catalog.GetIndex(), catalog2.GetIndex()) );
    REQUIRE( catalog.GetStringTable() == catalog2.GetStringTable() );
    REQUIRE( equal(catalog.GetStatistics(), catalog2.GetStatistics()) );
}


TEST_CASE( "Move assign operator in Catalog", "[Catalog]" ) {
    Catalog catalog(test_data.begin(), test_data.end());
    Catalog catalog2(catalog);
    Catalog catalog3;
    catalog3 = std::move(catalog2);

    REQUIRE( catalog.GetError() == catalog3.GetError() );
    REQUIRE( catalog.GetMetadata() == catalog3.GetMetadata() );
    REQUIRE( equal(catalog.GetIndex(), catalog3.GetIndex()) );
    REQUIRE( catalog.GetStringTable() == catalog3.GetStringTable() );
    REQUIRE( equal(catalog.GetStatistics(), catalog3.GetStatistics()) );
}


TEST_CASE( "Catalog::Clear()", "[Catalog]" ) {
    Catalog catalog(test_data.begin(), test_data.end());
    REQUIRE( catalog.GetError().size() == 1 );
    REQUIRE( catalog.GetMetadata().size() == 2 );
    REQUIRE( catalog.GetIndex().size() == 6 );
    REQUIRE( catalog.GetStringTable().size() == 13 );
    REQUIRE( catalog.GetStatistics().totalCount == 7 );
    REQUIRE( catalog.GetStatistics().metadataCount == 1 );
    REQUIRE( catalog.GetStatistics().translatedCount == 6 );
    REQUIRE( catalog.GetStatistics().discardedCount == 1 );
    catalog.Clear();
    REQUIRE( catalog.GetError().size() == 0 );
    REQUIRE( catalog.GetMetadata().size() == 0 );
    REQUIRE( catalog.GetIndex().size() == 0 );
    REQUIRE( catalog.GetStringTable().size() == 0 );
    REQUIRE( equal(catalog.GetStatistics(), Catalog::StatisticsT{}) );
}


const string test_data_2 = R"(# translator-comments
#. extracted-comment
#: references
#, flags
#| msgid previous-untranslated-string
msgid ""
msgstr "Project-Id-Version: test-data-2\n"
"MIME-Version: 1.0\n"
"Plural-Forms: nplurals=3; plural=(n+1) % 3;\n"

msgid "apples"
msgstr "Apples2"

msgid "bananas"
msgstr "Bananas"

msgid "corn"
msgid_plural "corns"
msgstr[0] "Corn#0"
msgstr[1] "Corn#1"
msgstr[2] "Corn#2"
msgstr[3] "Corn#3"

msgctxt "food"
msgid "eggs"
msgstr "Eggs"

msgctxt "food"
msgid "garlic"
msgid_plural "garlics"
msgstr[0] "Garlic#0"
msgstr[1] "Garlic#1"
msgstr[2] "Garlic#2"
msgstr[3] "Garlic#3"
msgstr[4] "Garlic#4"

msgid "error2"

msgid "mangoes"
msgstr "MANGOES"

msgid "oat"
msgid_plural "oats"
msgstr[0] "OAT#0"
msgstr[1] "OAT#1"
msgstr[2] "OAT#2"

msgctxt "food"
msgid "pickles"
msgstr "PICKLES"

msgid "error3"

msgctxt "food"
msgid "raisin"
msgid_plural "raisins"
msgstr[0] "RAISIN#0"
msgstr[1] "RAISIN#1"
msgstr[2] "RAISIN#2"
)";

TEST_CASE( "Catalog::Add(begin, end)", "[Catalog]" ) {
    Catalog catalog(test_data.begin(), test_data.end());
    catalog.Add(test_data_2.begin(), test_data_2.end());

    REQUIRE( catalog.GetError().size() == 3 );
    REQUIRE( catalog.GetMetadata().size() == 2 );
    REQUIRE( catalog.GetIndex().size() == 10 );
    REQUIRE( catalog.GetStringTable().size() == 21 );
    REQUIRE( catalog.GetStatistics().totalCount == 17 );
    REQUIRE( catalog.GetStatistics().metadataCount == 2 );
    REQUIRE( catalog.GetStatistics().translatedCount == 10 );
    REQUIRE( catalog.GetStatistics().discardedCount == 7 );
    REQUIRE( catalog.gettext("apples") == "APPLES" );
    REQUIRE( catalog.ngettext("corn", "corns", 1) == "CORN#1" );
    REQUIRE( catalog.ngettext("corn", "corns", 2) == "CORN#2" );
    REQUIRE( catalog.ngettext("corn", "corns", 4) == "CORN#0" );
    REQUIRE( catalog.pgettext("food", "eggs") == "EGGS" );
    REQUIRE( catalog.npgettext("food", "garlic", "garlics", 1) == "GARLIC#1" );
    REQUIRE( catalog.npgettext("food", "garlic", "garlics", 2) == "GARLIC#2" );
    REQUIRE( catalog.npgettext("food", "garlic", "garlics", 4) == "GARLIC#4" );
    REQUIRE( catalog.gettext("mangoes") == "MANGOES" );
    REQUIRE( catalog.ngettext("oat", "oats", 1) == "OAT#1" );
    REQUIRE( catalog.ngettext("oat", "oats", 2) == "OAT#2" );
    REQUIRE( catalog.ngettext("oat", "oats", 4) == "OAT#0" );
    REQUIRE( catalog.pgettext("food", "pickles") == "PICKLES" );
    REQUIRE( catalog.npgettext("food", "raisin", "raisins", 1) == "RAISIN#1" );
    REQUIRE( catalog.npgettext("food", "raisin", "raisins", 2) == "RAISIN#2" );
    REQUIRE( catalog.npgettext("food", "raisin", "raisins", 4) == "RAISIN#0" );
}


TEST_CASE( "Catalog::Add(is)", "[Catalog]" ) {
    Catalog catalog(test_data.begin(), test_data.end());
    istringstream is(test_data_2);
    catalog.Add(is);

    REQUIRE( catalog.GetError().size() == 3 );
    REQUIRE( catalog.GetMetadata().size() == 2 );
    REQUIRE( catalog.GetIndex().size() == 10 );
    REQUIRE( catalog.GetStringTable().size() == 21 );
    REQUIRE( catalog.GetStatistics().totalCount == 17 );
    REQUIRE( catalog.GetStatistics().metadataCount == 2 );
    REQUIRE( catalog.GetStatistics().translatedCount == 10 );
    REQUIRE( catalog.GetStatistics().discardedCount == 7 );
    REQUIRE( catalog.gettext("apples") == "APPLES" );
    REQUIRE( catalog.ngettext("corn", "corns", 1) == "CORN#1" );
    REQUIRE( catalog.ngettext("corn", "corns", 2) == "CORN#2" );
    REQUIRE( catalog.ngettext("corn", "corns", 4) == "CORN#0" );
    REQUIRE( catalog.pgettext("food", "eggs") == "EGGS" );
    REQUIRE( catalog.npgettext("food", "garlic", "garlics", 1) == "GARLIC#1" );
    REQUIRE( catalog.npgettext("food", "garlic", "garlics", 2) == "GARLIC#2" );
    REQUIRE( catalog.npgettext("food", "garlic", "garlics", 4) == "GARLIC#4" );
    REQUIRE( catalog.gettext("mangoes") == "MANGOES" );
    REQUIRE( catalog.ngettext("oat", "oats", 1) == "OAT#1" );
    REQUIRE( catalog.ngettext("oat", "oats", 2) == "OAT#2" );
    REQUIRE( catalog.ngettext("oat", "oats", 4) == "OAT#0" );
    REQUIRE( catalog.pgettext("food", "pickles") == "PICKLES" );
    REQUIRE( catalog.npgettext("food", "raisin", "raisins", 1) == "RAISIN#1" );
    REQUIRE( catalog.npgettext("food", "raisin", "raisins", 2) == "RAISIN#2" );
    REQUIRE( catalog.npgettext("food", "raisin", "raisins", 4) == "RAISIN#0" );
}


TEST_CASE( "Catalog::Merge()", "[Catalog]" ) {
    Catalog catalog(test_data.begin(), test_data.end());
    Catalog catalog2(test_data_2.begin(), test_data_2.end());
    catalog.Merge(catalog2);

    REQUIRE( catalog.GetError().size() == 3 );
    REQUIRE( catalog.GetMetadata().size() == 2 );
    REQUIRE( catalog.GetIndex().size() == 10 );
    REQUIRE( catalog.GetStringTable().size() == 21 );
    REQUIRE( catalog.GetStatistics().totalCount == 17 );
    REQUIRE( catalog.GetStatistics().metadataCount == 2 );
    REQUIRE( catalog.GetStatistics().translatedCount == 10 );
    REQUIRE( catalog.GetStatistics().discardedCount == 7 );
    REQUIRE( catalog.gettext("apples") == "APPLES" );
    REQUIRE( catalog.ngettext("corn", "corns", 1) == "CORN#1" );
    REQUIRE( catalog.ngettext("corn", "corns", 2) == "CORN#2" );
    REQUIRE( catalog.ngettext("corn", "corns", 4) == "CORN#0" );
    REQUIRE( catalog.pgettext("food", "eggs") == "EGGS" );
    REQUIRE( catalog.npgettext("food", "garlic", "garlics", 1) == "GARLIC#1" );
    REQUIRE( catalog.npgettext("food", "garlic", "garlics", 2) == "GARLIC#2" );
    REQUIRE( catalog.npgettext("food", "garlic", "garlics", 4) == "GARLIC#4" );
    REQUIRE( catalog.gettext("mangoes") == "MANGOES" );
    REQUIRE( catalog.ngettext("oat", "oats", 1) == "OAT#1" );
    REQUIRE( catalog.ngettext("oat", "oats", 2) == "OAT#2" );
    REQUIRE( catalog.ngettext("oat", "oats", 4) == "OAT#0" );
    REQUIRE( catalog.pgettext("food", "pickles") == "PICKLES" );
    REQUIRE( catalog.npgettext("food", "raisin", "raisins", 1) == "RAISIN#1" );
    REQUIRE( catalog.npgettext("food", "raisin", "raisins", 2) == "RAISIN#2" );
    REQUIRE( catalog.npgettext("food", "raisin", "raisins", 4) == "RAISIN#0" );
}


TEST_CASE( "Catalog::ClearError()", "[Catalog]" ) {
    Catalog catalog(test_data.begin(), test_data.end());
    catalog.ClearError();
    REQUIRE( catalog.GetError().size() == 0 );
    REQUIRE( catalog.GetMetadata().size() == 2 );
    REQUIRE( catalog.GetIndex().size() == 6 );
    REQUIRE( catalog.GetStringTable().size() == 13 );
    REQUIRE( catalog.GetStatistics().totalCount == 7 );
    REQUIRE( catalog.GetStatistics().metadataCount == 1 );
    REQUIRE( catalog.GetStatistics().translatedCount == 6 );
    REQUIRE( catalog.GetStatistics().discardedCount == 1 );

    catalog.Add(test_data_2.begin(), test_data_2.end());
    REQUIRE( catalog.GetError().size() == 2 );
    REQUIRE( catalog.GetMetadata().size() == 2 );
    REQUIRE( catalog.GetIndex().size() == 10 );
    REQUIRE( catalog.GetStringTable().size() == 21 );
    REQUIRE( catalog.GetStatistics().totalCount == 17 );
    REQUIRE( catalog.GetStatistics().metadataCount == 2 );
    REQUIRE( catalog.GetStatistics().translatedCount == 10 );
    REQUIRE( catalog.GetStatistics().discardedCount == 7 );

    catalog.ClearError();
    REQUIRE( catalog.GetError().size() == 0 );
    REQUIRE( catalog.GetMetadata().size() == 2 );
    REQUIRE( catalog.GetIndex().size() == 10 );
    REQUIRE( catalog.GetStringTable().size() == 21 );
    REQUIRE( catalog.GetStatistics().totalCount == 17 );
    REQUIRE( catalog.GetStatistics().metadataCount == 2 );
    REQUIRE( catalog.GetStatistics().translatedCount == 10 );
    REQUIRE( catalog.GetStatistics().discardedCount == 7 );
}


TEST_CASE( "Catalog::GetError()", "[Catalog]" ) {
    Catalog catalog(test_data.begin(), test_data.end());
    catalog.Add(test_data_2.begin(), test_data_2.end());
    vector<string> expected_errors { "40,1: Unexpected EOT (the previous entry is incomplete).", "39,1: Unexpected msgid (the previous entry is incomplete).", "54,1: Unexpected msgctxt (the previous entry is incomplete)." };
    REQUIRE( catalog.GetError() ==  expected_errors );
}


TEST_CASE( "Catalog::ClearStatistics()", "[Catalog]" ) {
    Catalog catalog(test_data.begin(), test_data.end());
    REQUIRE( catalog.GetError().size() == 1 );
    REQUIRE( catalog.GetMetadata().size() == 2 );
    REQUIRE( catalog.GetIndex().size() == 6 );
    REQUIRE( catalog.GetStringTable().size() == 13 );
    REQUIRE( catalog.GetStatistics().totalCount == 7 );
    REQUIRE( catalog.GetStatistics().metadataCount == 1 );
    REQUIRE( catalog.GetStatistics().translatedCount == 6 );
    REQUIRE( catalog.GetStatistics().discardedCount == 1 );
    catalog.ClearStatistics();

    catalog.Add(test_data_2.begin(), test_data_2.end());
    REQUIRE( catalog.GetError().size() == 3 );
    REQUIRE( catalog.GetMetadata().size() == 2 );
    REQUIRE( catalog.GetIndex().size() == 10 );
    REQUIRE( catalog.GetStringTable().size() == 21 );
    REQUIRE( catalog.GetStatistics().totalCount == 10 );
    REQUIRE( catalog.GetStatistics().metadataCount == 1 );
    REQUIRE( catalog.GetStatistics().translatedCount == 4 );
    REQUIRE( catalog.GetStatistics().discardedCount == 6 );
}


const string test_data_3 = R"(
msgid ""
msgstr ""

msgid "noodle"
msgstr "NOODLE"
)";

TEST_CASE( "Metadata Statistics (Add)", "[Catalog]" ) {
    Catalog catalog(test_data.begin(), test_data.end());
    catalog.Add(test_data_3.begin(), test_data_3.end());

    REQUIRE( catalog.GetError().size() == 1 );
    REQUIRE( catalog.GetMetadata().size() == 2 );
    REQUIRE( catalog.GetIndex().size() == 7 );
    REQUIRE( catalog.GetStringTable().size() == 14 );
    REQUIRE( catalog.GetStatistics().totalCount == 9 );
    REQUIRE( catalog.GetStatistics().metadataCount == 1 );
    REQUIRE( catalog.GetStatistics().translatedCount == 7 );
    REQUIRE( catalog.GetStatistics().discardedCount == 1 );

    catalog.Clear();
    REQUIRE( catalog.GetError().size() == 0 );
    REQUIRE( catalog.GetMetadata().size() == 0 );
    REQUIRE( catalog.GetIndex().size() == 0 );
    REQUIRE( catalog.GetStringTable().size() == 0 );
    REQUIRE( catalog.GetStatistics().totalCount == 0 );
    REQUIRE( catalog.GetStatistics().metadataCount == 0 );
    REQUIRE( catalog.GetStatistics().translatedCount == 0 );
    REQUIRE( catalog.GetStatistics().discardedCount == 0 );

    catalog.Add(test_data_3.begin(), test_data_3.end());
    REQUIRE( catalog.GetError().size() == 0 );
    REQUIRE( catalog.GetMetadata().size() == 0 );
    REQUIRE( catalog.GetIndex().size() == 1 );
    REQUIRE( catalog.GetStringTable().size() == 1 );
    REQUIRE( catalog.GetStatistics().totalCount == 2 );
    REQUIRE( catalog.GetStatistics().metadataCount == 0 );
    REQUIRE( catalog.GetStatistics().translatedCount == 1 );
    REQUIRE( catalog.GetStatistics().discardedCount == 0 );

    catalog.Add(test_data.begin(), test_data.end());
    REQUIRE( catalog.GetError().size() == 1 );
    REQUIRE( catalog.GetMetadata().size() == 2 );
    REQUIRE( catalog.GetIndex().size() == 7 );
    REQUIRE( catalog.GetStringTable().size() == 14 );
    REQUIRE( catalog.GetStatistics().totalCount == 9 );
    REQUIRE( catalog.GetStatistics().metadataCount == 1 );
    REQUIRE( catalog.GetStatistics().translatedCount == 7 );
    REQUIRE( catalog.GetStatistics().discardedCount == 1 );
}


TEST_CASE( "Metadata Statistics (Merge)", "[Catalog]" ) {
    Catalog catalog(test_data.begin(), test_data.end());
    Catalog catalog2(test_data_3.begin(), test_data_3.end());
    catalog.Merge(catalog2);

    REQUIRE( catalog.GetError().size() == 1 );
    REQUIRE( catalog.GetMetadata().size() == 2 );
    REQUIRE( catalog.GetIndex().size() == 7 );
    REQUIRE( catalog.GetStringTable().size() == 14 );
    REQUIRE( catalog.GetStatistics().totalCount == 8 );
    REQUIRE( catalog.GetStatistics().metadataCount == 1 );
    REQUIRE( catalog.GetStatistics().translatedCount == 7 );
    REQUIRE( catalog.GetStatistics().discardedCount == 1 );

    catalog.Clear();
    REQUIRE( catalog.GetError().size() == 0 );
    REQUIRE( catalog.GetMetadata().size() == 0 );
    REQUIRE( catalog.GetIndex().size() == 0 );
    REQUIRE( catalog.GetStringTable().size() == 0 );
    REQUIRE( catalog.GetStatistics().totalCount == 0 );
    REQUIRE( catalog.GetStatistics().metadataCount == 0 );
    REQUIRE( catalog.GetStatistics().translatedCount == 0 );
    REQUIRE( catalog.GetStatistics().discardedCount == 0 );

    catalog = catalog2;
    REQUIRE( catalog.GetError().size() == 0 );
    REQUIRE( catalog.GetMetadata().size() == 0 );
    REQUIRE( catalog.GetIndex().size() == 1 );
    REQUIRE( catalog.GetStringTable().size() == 1 );
    REQUIRE( catalog.GetStatistics().totalCount == 2 );
    REQUIRE( catalog.GetStatistics().metadataCount == 0 );
    REQUIRE( catalog.GetStatistics().translatedCount == 1 );
    REQUIRE( catalog.GetStatistics().discardedCount == 0 );

    catalog2.Clear();
    catalog2.Add(test_data.begin(), test_data.end());
    catalog.Merge(catalog2);
    REQUIRE( catalog.GetError().size() == 1 );
    REQUIRE( catalog.GetMetadata().size() == 2 );
    REQUIRE( catalog.GetIndex().size() == 7 );
    REQUIRE( catalog.GetStringTable().size() == 14 );
    REQUIRE( catalog.GetStatistics().totalCount == 8 );
    REQUIRE( catalog.GetStatistics().metadataCount == 1 );
    REQUIRE( catalog.GetStatistics().translatedCount == 7 );
    REQUIRE( catalog.GetStatistics().discardedCount == 0 );
}

namespace {
    class TestInputEnd {
    public:
        TestInputEnd() = default;
        TestInputEnd(const TestInputEnd& a) = default;
        TestInputEnd &operator=(const TestInputEnd& a) = default;
    };

    class TestInputIterator {
    public:
        TestInputIterator() = default;
        explicit TestInputIterator(const char* s);
        TestInputIterator(const TestInputIterator &a) = delete;
        TestInputIterator(TestInputIterator &&a) = default;
        ~TestInputIterator() noexcept = default;
        TestInputIterator &operator=(const TestInputIterator &a) = delete;
        TestInputIterator &operator=(TestInputIterator &&a) = default;
        TestInputIterator &operator++();
        char operator*() const;

        using difference_type = int;
        using value_type = char;
        using iterator_concept = std::input_iterator_tag;
    private:
        const char *p;
    };

    TestInputIterator::TestInputIterator(const char *s)
        : p{s}
    {
    }

    TestInputIterator &TestInputIterator::operator++()
    {
        ++p;
        return *this;
    }

    char TestInputIterator::operator*() const
    {
        return *p;
    }

    bool operator==(const TestInputIterator &i, const TestInputEnd &e)
    {
        (void)e; // unused
        return *i == '\0';
    }

    bool operator!=(const TestInputIterator &i, const TestInputEnd &e)
    {
        (void)e; // unused
        return *i != '\0';
    }
}

TEST_CASE( "Constructor by C++20 Input Iterator (non copyable)", "[Catalog]" ) {
    TestInputIterator b{test_data.c_str()};
    TestInputEnd e;
    Catalog catalog(b, e);

    REQUIRE( catalog.GetError().size() == 1 );
    REQUIRE( catalog.GetMetadata().size() == 2 );
    REQUIRE( catalog.GetIndex().size() == 6 );
    REQUIRE( catalog.GetStringTable().size() == 13 );
    REQUIRE( catalog.GetStatistics().totalCount == 7 );
    REQUIRE( catalog.GetStatistics().metadataCount == 1 );
    REQUIRE( catalog.GetStatistics().translatedCount == 6 );
    REQUIRE( catalog.GetStatistics().discardedCount == 1 );
}

TEST_CASE( "Add by C++20 Input Iterator (non copyable)", "[Catalog]" ) {
    TestInputIterator b{test_data.c_str()};
    TestInputEnd e;
    Catalog catalog;
    catalog.Add(b, e);

    REQUIRE( catalog.GetError().size() == 1 );
    REQUIRE( catalog.GetMetadata().size() == 2 );
    REQUIRE( catalog.GetIndex().size() == 6 );
    REQUIRE( catalog.GetStringTable().size() == 13 );
    REQUIRE( catalog.GetStatistics().totalCount == 7 );
    REQUIRE( catalog.GetStatistics().metadataCount == 1 );
    REQUIRE( catalog.GetStatistics().translatedCount == 6 );
    REQUIRE( catalog.GetStatistics().discardedCount == 1 );
}



TEST_CASE( "default nplurals is 2 (1/2)", "[Catalog]" ) {
    const string po_text = R"(msgid ""
msgstr ""
"Plural-Forms: plural=n%4;\n"

msgid "a"
msgid_plural "as"
msgstr[0] "A0"
msgstr[1] "A1"
msgstr[2] "A2"
msgstr[3] "A3"
)";
    Catalog catalog(po_text.begin(), po_text.end());
    REQUIRE( catalog.GetError().size() == 0 );
    REQUIRE( catalog.ngettext("a", "as", 0) == "A0" );
    REQUIRE( catalog.ngettext("a", "as", 1) == "A1" );
    REQUIRE( catalog.ngettext("a", "as", 2) == "A0" );
    REQUIRE( catalog.ngettext("a", "as", 3) == "A0" );
    REQUIRE( catalog.ngettext("a", "as", 4) == "A0" );
    REQUIRE( catalog.ngettext("a", "as", 5) == "A1" );
}

TEST_CASE( "default nplurals is 2 (2/2)", "[Catalog]" ) {
    const string po_text = R"(msgid ""
msgstr ""
"Plural-Forms: nplurals=x; plural=n%4;\n"

msgid "a"
msgid_plural "as"
msgstr[0] "A0"
msgstr[1] "A1"
msgstr[2] "A2"
msgstr[3] "A3"
)";
    Catalog catalog(po_text.begin(), po_text.end());
    REQUIRE( catalog.GetError().size() == 0 );
    REQUIRE( catalog.ngettext("a", "as", 0) == "A0" );
    REQUIRE( catalog.ngettext("a", "as", 1) == "A1" );
    REQUIRE( catalog.ngettext("a", "as", 2) == "A0" );
    REQUIRE( catalog.ngettext("a", "as", 3) == "A0" );
    REQUIRE( catalog.ngettext("a", "as", 4) == "A0" );
    REQUIRE( catalog.ngettext("a", "as", 5) == "A1" );
}

TEST_CASE( "nplurals=0 means 1", "[Catalog]" ) {
    const string po_text = R"(msgid ""
msgstr ""
"Plural-Forms: nplurals=0; plural=n%4;\n"

msgid "a"
msgid_plural "as"
msgstr[0] "A0"
msgstr[1] "A1"
msgstr[2] "A2"
msgstr[3] "A3"
)";
    Catalog catalog(po_text.begin(), po_text.end());
    REQUIRE( catalog.GetError().size() == 1 );
    REQUIRE( catalog.GetError()[0] == "nplurals must be more than 0; ignored." );
    REQUIRE( catalog.ngettext("a", "as", 0) == "A0" );
    REQUIRE( catalog.ngettext("a", "as", 1) == "A0" );
    REQUIRE( catalog.ngettext("a", "as", 2) == "A0" );
    REQUIRE( catalog.ngettext("a", "as", 3) == "A0" );
    REQUIRE( catalog.ngettext("a", "as", 4) == "A0" );
    REQUIRE( catalog.ngettext("a", "as", 5) == "A0" );
}

TEST_CASE( "default plural= is n!=1", "[Catalog]" ) {
    const string po_text = R"(msgid ""
msgstr ""
"Plural-Forms: nplurals=4;\n"

msgid "a"
msgid_plural "as"
msgstr[0] "A0"
msgstr[1] "A1"
msgstr[2] "A2"
msgstr[3] "A3"
)";
    Catalog catalog(po_text.begin(), po_text.end());
    REQUIRE( catalog.GetError().size() == 0 );
    REQUIRE( catalog.ngettext("a", "as", 0) == "A1" );
    REQUIRE( catalog.ngettext("a", "as", 1) == "A0" );
    REQUIRE( catalog.ngettext("a", "as", 2) == "A1" );
    REQUIRE( catalog.ngettext("a", "as", 3) == "A1" );
    REQUIRE( catalog.ngettext("a", "as", 4) == "A1" );
    REQUIRE( catalog.ngettext("a", "as", 5) == "A1" );
}

TEST_CASE( "plural expression error causes fallback into n!=1", "[Catalog]" ) {
    const string po_text = R"(msgid ""
msgstr ""
"Plural-Forms: nplurals=4; plural=n%4x\n"

msgid "a"
msgid_plural "as"
msgstr[0] "A0"
msgstr[1] "A1"
msgstr[2] "A2"
msgstr[3] "A3"
)";
    Catalog catalog(po_text.begin(), po_text.end());
    REQUIRE( catalog.GetError().size() == 1 );
    REQUIRE( catalog.GetError()[0] == "Column#4 in the plural expression \"n%4x\": Parse error: Invalid character is detected." );
    REQUIRE( catalog.ngettext("a", "as", 0) == "A1" );
    REQUIRE( catalog.ngettext("a", "as", 1) == "A0" );
    REQUIRE( catalog.ngettext("a", "as", 2) == "A1" );
    REQUIRE( catalog.ngettext("a", "as", 3) == "A1" );
    REQUIRE( catalog.ngettext("a", "as", 4) == "A1" );
    REQUIRE( catalog.ngettext("a", "as", 5) == "A1" );
}

TEST_CASE( "normal nplurals= and plurals=", "[Catalog]" ) {
    const string po_text = R"(msgid ""
msgstr ""
"Plural-Forms: nplurals=4; plural=n%4\n"

msgid "a"
msgid_plural "as"
msgstr[0] "A0"
msgstr[1] "A1"
msgstr[2] "A2"
msgstr[3] "A3"
)";
    Catalog catalog(po_text.begin(), po_text.end());
    REQUIRE( catalog.GetError().size() == 0 );
    REQUIRE( catalog.ngettext("a", "as", 0) == "A0" );
    REQUIRE( catalog.ngettext("a", "as", 1) == "A1" );
    REQUIRE( catalog.ngettext("a", "as", 2) == "A2" );
    REQUIRE( catalog.ngettext("a", "as", 3) == "A3" );
    REQUIRE( catalog.ngettext("a", "as", 4) == "A0" );
    REQUIRE( catalog.ngettext("a", "as", 5) == "A1" );
}
