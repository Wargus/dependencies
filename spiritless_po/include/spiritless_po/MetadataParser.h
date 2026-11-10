/** Metadata parser.
    \file MetadataParser.h
    \author OOTA, Masato
    \copyright Copyright © 2019, 2022, 2026 OOTA, Masato
    \par License Boost
    \parblock
      This program is distributed under the Boost Software License Version 1.0.
      You can get the license file at “https://www.boost.org/LICENSE_1_0.txt”.
    \endparblock
*/

#ifndef SPIRITLESS_PO_METADATA_PARSER_H_
#define SPIRITLESS_PO_METADATA_PARSER_H_

#include <locale>
#include <string>
#include <unordered_map>

namespace spiritless_po {
    namespace MetadataParser {
        /** The type of the metadata.

            map[key] == value
        */
        typedef std::unordered_map<std::string, std::string> MapT;

        /** Parse a metadata text.
            \param [in] metadataString The source text of the metadata.
            \return The map of the metadata.

            This function parses a metadata text and set the keys and the values to the map.

            The metadata text consists of the lines that have a key and a value, such as "key: value\n", more exactly it can be expressed the regex "^(.+): *(.+)\n?$" (key = $1, value = $2), of course, '\\n' is necessary except for the last line. It's compatible with po_header_field() in GNU libgettextpo.

            If some lines have the same keys, the first line is registered.
        */
        inline MapT Parse(const std::string &metadataString)
        {
            MapT map;
            enum { KEY, SPACE, VALUE } stat = KEY;
            std::string key;
            std::string value;
            for (const char c : metadataString) {
                if (stat == KEY) {
                    if (c == ':') {
                        stat = SPACE;
                    } else if (c == '\n') {
                        key.clear();
                    } else {
                        key += c;
                    }
                } else if ((stat == SPACE && c != ' ') || stat == VALUE) {
                    stat = VALUE;
                    if (c == '\n') {
                        stat = KEY;
                        if (!key.empty()) {
                            map.emplace(key, value);
                            key.clear();
                        }
                        value.clear();
                    } else {
                        value += c;
                    }
                }
            }
            if (!key.empty()) {
                map.emplace(key, value);
            }
            return map;
        }

        /** Get the value for "nplurals=".
            \param [in] metadataString The source text of the metadata.
            \param [out] value the value. (don't change if the value of "nplurals=" is not found.)
            \note Just search "nplurals=" and don't care about the key "Plural-Forms", because GNU gettext tools ignore it.
        */
        inline void GetNPlurals(const std::string &metadataString, unsigned long &value)
        {
            const std::string key("nplurals=");
            const std::size_t start = metadataString.find(key);
            if (start == metadataString.npos) {
                return;
            }
            const std::size_t len = metadataString.size();
            std::string s;
            std::size_t i = start + key.size();
            for ( ; i < len; ++i) {
                const char c = metadataString[i];
                if (!std::isspace(c, std::locale::classic())) {
                    // Skip the white spaces.
                    break;
                }
            }
            for (; i < len; ++i) {
                const char c = metadataString[i];
                if (std::isdigit(c, std::locale::classic())) {
                    s += c;
                } else {
                    // Ignore the non-digit characters.
                    break;
                }
            }
            if (!s.empty()) {
                value = std::stoul(s);
            }
        }

        /** Get the value for "plural=".
            \param [in] metadataString The source text of the metadata.
            \param [out] value the value. (don't change if "plural=" is not found.)
            \note The value for "plural=" may contain the white spaces except '\n', but the parser for the plural expression causes an error if the value has a white space except ' ' and '\t'.
            \note Just search "plural=" and don't care about the key "Plural-Forms", because GNU gettext tools ignore it.
        */
        inline void GetPlural(const std::string &metadataString, std::string &value)
        {
            const std::string key("plural=");
            const std::size_t start = metadataString.find(key);
            if (start == metadataString.npos) {
                return;
            }
            const std::size_t len = metadataString.size();
            value.clear();
            for (std::size_t i = start + key.size(); i < len; ++i) {
                const char c = metadataString[i];
                if (c != ';' && c != '\n') {
                    value += c;
                } else {
                    break;
                }
            }
        }
    } // namespace MetadataParser
} // namespace spiritless_po

#endif // SPIRITLESS_PO_METADATA_PARSER_H_
