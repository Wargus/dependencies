/** PO text parser.
    \file PoParser.h
    \author OOTA, Masato
    \copyright Copyright © 2019, 2022, 2026  OOTA, Masato
    \par License Boost
    \parblock
      This program is distributed under the Boost Software License Version 1.0.
      You can get the license file at “https://www.boost.org/LICENSE_1_0.txt”.
    \endparblock
*/

#ifndef SPIRITLESS_PO_PO_PARSER_H_
#define SPIRITLESS_PO_PO_PARSER_H_

#include "Common.h"

#include <cstddef>
#include <cstdlib>
#include <limits>
#include <locale>
#include <string>
#include <utility>
#include <vector>

namespace spiritless_po {
    /** This class is a parser for the text that contains the PO entries. */
    class PoParser {
    public:
        /** Type of a PO entry.

            - msgid and msgstr are uncertain when error is not empty.
            - msgstr.size() > 0 when error is empty.
            - msgstr[0] is an empty string if the entry is fuzzy.
        */
        struct PoEntryT {
            std::string msgid; /**< msgid ( + CONTEXT_SEPARATOR + msgctxt if msgctxt exists.) */
            std::vector<std::string> msgstr; /**< msgstr, or msgstr[n] if the entry is for msgid_plural. */
            std::string error; /**< The messages that describe the error in the parsing. */
        };

        /** Parse the text that contains the PO entries.
            \tparam INP A type of an input iterator.
            \tparam Sentinel A type of a sentinel.
            \param [in] begin The beginning of the text to parse.
            \param [in] end The end of the text to parse.
            \return The result of the parsing.
        */
        template <typename INP, typename Sentinel>
        static std::vector<PoEntryT> GetEntries(INP &&begin, Sentinel &&end);

    private:
        // Reading location type.
        class LocationT {
        public:
            explicit LocationT(std::size_t line = 1, std::size_t column = 1);
            LocationT(const LocationT &a) = default;
            LocationT &operator=(const LocationT &a) = default;

            void Next(char cur_c);
            std::size_t GetLine() const noexcept;
            std::size_t GetColumn() const noexcept;
            std::string ToString() const;

        private:
            std::size_t lineNumber;
            std::size_t columnNumber;
        };

        // Char iterator, tracking the location of the source text.
        template <typename INP, typename Sentinel>
        class CharFeeder {
        public:
            CharFeeder(INP &it, Sentinel &end, const LocationT &startLoc);

            bool IsEnd() const;
            bool IsNotEnd() const;
            char Get() const;
            void Next();
            const LocationT &GetLocation() const noexcept;

        private:
            INP &curIt;
            Sentinel &endIt;
            LocationT loc;
        };

        // Parse Error in PO file.
        class PoParseError : public std::runtime_error {
        public:
            PoParseError(const std::string &whatArg, const LocationT &errorLoc);
            PoParseError(const char *whatArg, const LocationT &errorLoc);

            // Get the error location.
            const LocationT &GetLocation() const noexcept;

        private:
            LocationT loc;
        };

        // Type of a token.
        enum class TokenT : unsigned char {
            COMMENT,
            FUZZY,
            MSGCTXT,
            MSGID,
            MSGID_PLURAL,
            MSGSTR,
            MSGSTR_PLURAL,
            TEXT,
            ERROR,
            EOT,
            TOTAL,
        };

        // Type of a state.
        enum class StateT : unsigned char {
            END_OF_ENTRY, // Special state, don't consume a token.
            ABORT_ENTRY, // Special state, don't consume a token.
            ERROR_BEFORE_MSGID, // The next msgid should be dropped.
            COMMENT,
            MSGCTXT,
            MSGID,
            MSGID_PLURAL,
            MSGSTR,
            MSGSTR_PLURAL,
            MSGCTXT_TEXT,
            MSGID_TEXT,
            MSGID_PLURAL_TEXT,
            MSGSTR_TEXT,
            MSGSTR_PLURAL_TEXT,
            ERROR,
            EOT,
            TOTAL,
        };

        // State Transition table
        class StateTransTable {
        public:
            StateTransTable() noexcept;
            StateT GetState(StateT state, TokenT token) const noexcept;

        private:
            StateT trans[static_cast<unsigned int>(StateT::TOTAL)][static_cast<unsigned int>(TokenT::TOTAL)];
        };

        PoParser() = delete;
        ~PoParser() = delete;

        // Internal functions
        template <typename INP, typename Sentinel>
        static bool StartsWith(CharFeeder<INP, Sentinel> &it, const char *word);
        static const char *GetTokenName(PoParser::TokenT t);
        template <typename INP, typename Sentinel>
        static void SkipWhiteSpace(PoParser::CharFeeder<INP, Sentinel> &it);
        template <typename INP, typename Sentinel>
        static std::size_t GetNumber(CharFeeder<INP, Sentinel> &it);
        template <typename INP, typename Sentinel>
        static TokenT ParseMsgKeyword(CharFeeder<INP, Sentinel> &it, std::size_t &n_msgstr);
        template <typename INP, typename Sentinel>
        static char GetEscapeXChar(CharFeeder<INP, Sentinel> &it);
        template <typename INP, typename Sentinel>
        static char GetEscape0Char(CharFeeder<INP, Sentinel> &it, char firstC);
        template <typename INP, typename Sentinel>
        static std::string ParseText(CharFeeder<INP, Sentinel> &it);
        template <typename INP, typename Sentinel>
        static TokenT ParseComment(CharFeeder<INP, Sentinel> &it);
        template <typename INP, typename Sentinel>
        static TokenT Lex(CharFeeder<INP, Sentinel> &it, LocationT &loc, std::string &text, std::size_t &n_msgstr);
    };

    inline PoParser::LocationT::LocationT(std::size_t line, std::size_t column)
        : lineNumber(line), columnNumber(column)
    {
    }

    inline void PoParser::LocationT::Next(char cur_c)
    {
        if (cur_c == '\n') {
            ++lineNumber;
            columnNumber = 1;
        } else {
            ++columnNumber;
        }
    }

    inline std::size_t PoParser::LocationT::GetLine() const noexcept
    {
        return lineNumber;
    }

    inline std::size_t PoParser::LocationT::GetColumn() const noexcept
    {
        return columnNumber;
    }

    inline std::string PoParser::LocationT::ToString() const
    {
        return std::to_string(GetLine()) + ',' + std::to_string(GetColumn()) + ": ";
    }

    template <typename INP, typename Sentinel>
    PoParser::CharFeeder<INP, Sentinel>::CharFeeder(INP &it, Sentinel &end, const PoParser::LocationT &startLoc)
        : curIt(it), endIt(end), loc(startLoc)
    {
    }

    template <typename INP, typename Sentinel>
    bool PoParser::CharFeeder<INP, Sentinel>::IsEnd() const
    {
        return curIt == endIt;
    }

    template <typename INP, typename Sentinel>
    bool PoParser::CharFeeder<INP, Sentinel>::IsNotEnd() const
    {
        return curIt != endIt;
    }

    template <typename INP, typename Sentinel>
    char PoParser::CharFeeder<INP, Sentinel>::Get() const
    {
        return IsEnd() ? '\0' : *curIt;
    }

    template <typename INP, typename Sentinel>
    void PoParser::CharFeeder<INP, Sentinel>::Next()
    {
        if (IsNotEnd()) {
            loc.Next(Get());
            ++curIt;
        }
    }

    template <typename INP, typename Sentinel>
    const PoParser::LocationT &PoParser::CharFeeder<INP, Sentinel>::GetLocation() const noexcept
    {
        return loc;
    }

    inline PoParser::PoParseError::PoParseError(const std::string &whatArg, const LocationT &errorLoc)
        : std::runtime_error(whatArg), loc(errorLoc)
    {
    }

    inline PoParser::PoParseError::PoParseError(const char *whatArg, const LocationT &errorLoc)
        : std::runtime_error(whatArg), loc(errorLoc)
    {
    }

    inline const PoParser::LocationT &PoParser::PoParseError::GetLocation() const noexcept
    {
        return loc;
    }

    // Compare word with the string comes from the iterator. (Utility function)
    // Post condition: it.GetLocation() points to the next of word, the first location differed from word, or the end of it.
    template <typename INP, typename Sentinel>
    bool PoParser::StartsWith(CharFeeder<INP, Sentinel> &it, const char *const word)
    {
        const char *p = word;
        while (it.IsNotEnd() && *p != '\0') {
            const char c = it.Get();
            if (c != *p) {
                break;
            }
            it.Next();
            ++p;
        }
        return *p == '\0';
    }

    // Token names for error messages
    inline const char *PoParser::GetTokenName(const PoParser::TokenT t)
    {
        static const char *const name[] = {
            "comment",
            "fuzzy flag comment",
            "msgctxt",
            "msgid",
            "msgid_plural",
            "msgstr",
            "msgstr[n]",
            "quoted text",
            "unknown token",
            "EOT",
            "???",
        };
        return name[static_cast<unsigned int>(t)];
    }

    // Skip white spaces. (Utility function)
    template <typename INP, typename Sentinel>
    // Post condition: it.GetLocation() points to the first non-space character, or the end of it.
    void PoParser::SkipWhiteSpace(PoParser::CharFeeder<INP, Sentinel> &it)
    {
        while (it.IsNotEnd()) {
            const char c = it.Get();
            if (std::isspace(c, std::locale::classic())) {
                it.Next();
            } else {
                break;
            }
        }
    }

    // get a number. (Utility function)
    // Post condition: it.GetLocation() points to the first non-digit character, or the end of it.
    template <typename INP, typename Sentinel>
    std::size_t PoParser::GetNumber(CharFeeder<INP, Sentinel> &it)
    {
        std::string s;
        while (it.IsNotEnd()) {
            const char c = it.Get();
            if (std::isdigit(c, std::locale::classic())) {
                s += c;
                it.Next();
            } else {
                break;
            }
        }
        if (s.empty()) {
            throw PoParseError("'0'..'9' is expected.", it.GetLocation());
        }
        return std::stoul(s);
    }

    // parse msgctxt, msgid, msgid_plural, msgstr, and msgstr[n]
    // Pre condition: it.Get() == 'm'
    // Post condition: it.GetLocation() points to the next of the keyword, the next character except the white spaces, the first location found an error, or the end of it.
    // n_msgstr for msgstr[n_msgstr] when MSGSTR_PLURAL is returned.
    // (n_msgstr is preserved when MSGSTR is returned)
    template <typename INP, typename Sentinel>
    PoParser::TokenT PoParser::ParseMsgKeyword(CharFeeder<INP, Sentinel> &it, std::size_t &n_msgstr)
    {
        const LocationT startLoc = it.GetLocation();
        TokenT token = TokenT::ERROR;
        it.Next();
        if (StartsWith(it, "sg")) {
            const char c = it.Get();
            if (c == 'c') {
                it.Next();
                if (StartsWith(it, "txt")) {
                    token = TokenT::MSGCTXT;
                }
            } else if (c == 'i') {
                it.Next();
                if (it.Get() == 'd') {
                    it.Next();
                    if (it.Get() == '_') {
                        it.Next();
                        if (StartsWith(it, "plural")) {
                            token = TokenT::MSGID_PLURAL;
                        }
                    } else {
                        token = TokenT::MSGID;
                    }
                }
            } else if (c == 's') {
                it.Next();
                if (StartsWith(it, "tr")) {
                    SkipWhiteSpace(it);
                    if (it.Get() != '[') {
                        token = TokenT::MSGSTR;
                    } else {
                        it.Next();
                        SkipWhiteSpace(it);
                        n_msgstr = GetNumber(it);
                        SkipWhiteSpace(it);
                        if (it.Get() == ']') {
                            it.Next();
                            token = TokenT::MSGSTR_PLURAL;
                        } else {
                            throw PoParseError("']' is expected.", it.GetLocation());
                        }
                    }
                }
            }
        }
        if (token == TokenT::ERROR) {
            throw PoParseError("Unknown keyword.", startLoc);
        }
        return token;
    }

    // convert a hexadecimal number to char. (Utility function)
    // Pre condition: it.GetLocation() points to the first character that may be a hexadecimal.
    // Post condition: it.GetLocation() points to the first non-hexadecimal character, or the end of it.
    template <typename INP, typename Sentinel>
    char PoParser::GetEscapeXChar(CharFeeder<INP, Sentinel> &it)
    {
        // The encoding is UTF-8 and the maximum width of hexadecimal is 2.
        char s[3] = {};
        unsigned int idx = 0;
        // The length "\xh..h" is unlimited.
        while (it.IsNotEnd()) {
            const char c = it.Get();
            if (std::isxdigit(c, std::locale::classic())) {
                if (idx >= 2) {
                    s[0] = s[1];
                    s[1] = c;
                } else {
                    s[idx] = c;
                    ++idx;
                }
                it.Next();
            } else {
                break;
            }
        }
        if (idx == 0) {
            throw PoParseError("[0-9A-Fa-f] is expected.", it.GetLocation());
        }

        int c = static_cast<int>(std::strtoul(s, NULL, 16) & 0xFF);
        if (std::numeric_limits<char>::is_signed && c > std::numeric_limits<char>::max()) {
            c = c - 0x100;
        }
        return static_cast<char>(c);
    }

    // convert a octal number to char. (Utility function)
    // Pre condition: it.GetLocation() points to the second character that may be an octal.
    // Post condition: it.GetLocation() points to the first non-octal character, or the end of it.
    template <typename INP, typename Sentinel>
    char PoParser::GetEscape0Char(CharFeeder<INP, Sentinel> &it, char firstC)
    {
        char s[4] = {
            firstC,
        };
        unsigned int idx = 1;
        // The maximum digits number of the escape octal is 3.
        while (it.IsNotEnd()) {
            const char c = it.Get();
            if (std::isdigit(c, std::locale::classic()) && c != '8' && c != '9') {
                s[idx] = c;
                ++idx;
                it.Next();
                if (idx >= 3) {
                    break;
                }
            } else {
                break;
            }
        }

        int c = static_cast<int>(std::strtoul(s, NULL, 8) & 0xFF);
        if (std::numeric_limits<char>::is_signed && c > std::numeric_limits<char>::max()) {
            c = c - 0x100;
        }
        return static_cast<char>(c);
    }

    // parse a quoted text
    // Pre condition: it.Get() == '"'
    // Post condition: it.GetLocation() points to the next of the closing '"', the first location found an error, or the end of it.
    template <typename INP, typename Sentinel>
    std::string PoParser::ParseText(CharFeeder<INP, Sentinel> &it)
    {
        std::string text;
        bool closed = false;
        LocationT errorLoc = it.GetLocation();
        std::string errorMessage;
        bool error = false;
        it.Next();
        while (it.IsNotEnd()) {
            const char c = it.Get();
            const LocationT loc = it.GetLocation();
            it.Next();
            if (c == '"') {
                closed = true;
                break;
            } else if (c == '\n') {
                if (!error) {
                    errorMessage = "The text may not contain a newline.";
                    errorLoc = loc;
                    error = true;
                }
                break;
            } else if (c == '\\') {
                const char c2 = it.Get();
                const LocationT loc2 = it.GetLocation();
                it.Next();
                switch (c2) {
                case 'a':
                    text += '\a';
                    break;
                case 'b':
                    text += '\b';
                    break;
                case 'f':
                    text += '\f';
                    break;
                case 'n':
                    text += '\n';
                    break;
                case 'r':
                    text += '\r';
                    break;
                case 't':
                    text += '\t';
                    break;
                case 'v':
                    text += '\v';
                    break;
                case '\\':
                    text += '\\';
                    break;
                case '"':
                    text += '"';
                    break;
                case '\'':
                    text += '\'';
                    break;
                case '?':
                    text += '?';
                    break;
                case 'x':
                    text += PoParser::GetEscapeXChar(it);
                    break;
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                    text += PoParser::GetEscape0Char(it, c2);
                    break;
                default:
                    if (!error) {
                        errorMessage = "Invalid escape sequence.";
                        errorLoc = loc2;
                        error = true;
                        // try to consume the input characters until '"' is found.
                    }
                }
            } else {
                text += c;
            }
        }
        if (!closed && !error) {
            errorMessage = "Closing double quotation mark is expected.";
            errorLoc = it.GetLocation();
            error = true;
        }
        if (error) {
            throw PoParseError(errorMessage, errorLoc);
        }
        return text;
    }

    // parse a comment
    // Pre condition: it.GetLocation() points to the next of "#".
    // Post condition: it.GetLocation() points to the next of '\n', or the end of it.
    template <typename INP, typename Sentinel>
    PoParser::TokenT PoParser::ParseComment(CharFeeder<INP, Sentinel> &it)
    {
        TokenT token = TokenT::COMMENT;
        if (it.Get() == ',' || it.Get() == '=') {
            // Flag comment.
            it.Next();
            // The flags are separated by some white spaces and commas.
            int state = 0;
            while (it.IsNotEnd() && it.Get() != '\n') {
                const char c = it.Get();
                switch (state) {
                case 0:
                    // Skip white spaces and comma.
                    if (std::isspace(c, std::locale::classic()) || c == ',') {
                        it.Next();
                    } else {
                        state = 1;
                    }
                    break;
                case 1:
                    // Check the flag.
                    if (StartsWith(it, "fuzzy")) {
                        // It's a fuzzy if the next is the end or '\n'.
                        token = TokenT::FUZZY;
                        state = 2;
                    } else {
                        state = 3;
                    }
                    break;
                case 2:
                    // Check if it is the end of "fuzzy".
                    if (std::isspace(c, std::locale::classic()) || c == ',') {
                        // No need to search it anymore.
                        state = 4;
                    } else {
                        // It was not a fuzzy.
                        token = TokenT::COMMENT;
                        it.Next();
                        state = 3;
                    }
                    break;
                default:
                    // Skip until the next white space or comma.
                    if (std::isspace(c, std::locale::classic()) || c == ',') {
                        state = 0;
                    }
                    it.Next();
                    break;
                }
                if (state == 4) {
                    break;
                }
            }
        }
        // Skip until NL.
        while (it.IsNotEnd()) {
            const char c = it.Get();
            it.Next();
            if (c == '\n') {
                break;
            }
        }
        return token;
    }

    // lexical analyzer
    // Post condition: it.GetLocation() points to the next location of the last fed character.
    // loc is the location of token.
    // text for "text" when TEXT is returned.
    // n_msgstr for msgstr[n_msgstr] when MSGSTR_PLURAL is returned.
    // (n_msgstr is preserved when MSGSTR is returned)
    template <typename INP, typename Sentinel>
    PoParser::TokenT PoParser::Lex(CharFeeder<INP, Sentinel> &it, LocationT &loc, std::string &text, std::size_t &n_msgstr)
    {
        SkipWhiteSpace(it);
        loc = it.GetLocation();
        if (it.IsEnd()) {
            return TokenT::EOT;
        }

        TokenT token = TokenT::ERROR;
        const char c = it.Get();
        if (c == '#') {
            it.Next();
            token = ParseComment(it);
        } else if (c == 'm') {
            token = ParseMsgKeyword(it, n_msgstr);
        } else if (c == '"') {
            text = ParseText(it);
            token = TokenT::TEXT;
        } else {
            throw PoParseError("Unknown token.", it.GetLocation());
        }
        return token;
    }

    inline PoParser::StateTransTable::StateTransTable() noexcept
    {
        // C++11 doesn't allow the for statement in the constexpr function...

        for (std::size_t i = 0; i < static_cast<std::size_t>(StateT::TOTAL); ++i) {
            for (std::size_t j = 0; j < static_cast<std::size_t>(TokenT::TOTAL); ++j) {
                const StateT st = static_cast<StateT>(i);
                const TokenT tk = static_cast<TokenT>(j);
                if (tk == TokenT::COMMENT || tk == TokenT::FUZZY || tk == TokenT::MSGCTXT || tk == TokenT::MSGID || tk == TokenT::EOT) {
                    // The token suggests the end of the current entry and the token is the start of the next entry.
                    if ((StateT::MSGID <= st && st <= StateT::MSGSTR_PLURAL) || st == StateT::MSGID_TEXT || st == StateT::MSGID_PLURAL_TEXT || st == StateT::ERROR) {
                        // The current entry is almost finished, but not completed.
                        trans[i][j] = StateT::ABORT_ENTRY;
                    } else if (st == StateT::MSGSTR_TEXT || st == StateT::MSGSTR_PLURAL_TEXT) {
                        // The current entry is finished.
                        trans[i][j] = StateT::END_OF_ENTRY;
                    } else {
                        // The token belongs to the current entry.
                        trans[i][j] = StateT::ERROR_BEFORE_MSGID;
                    }
                } else if (st == StateT::ERROR || tk == TokenT::MSGID || tk == TokenT::MSGID_PLURAL || tk == TokenT::MSGSTR || tk == TokenT::MSGSTR_PLURAL) {
                    // The next starting token belongs to the next entry.
                    // Try to find the start token of the next entry to recover.
                    trans[i][j] = StateT::ERROR;
                } else {
                    // The token belongs to the current entry.
                    trans[i][j] = StateT::ERROR_BEFORE_MSGID;
                }
            }
        }

        // transition table
        constexpr struct {
            StateT cur;
            TokenT token;
            StateT next;
        } valid_def[] = {
            {StateT::COMMENT, TokenT::COMMENT, StateT::COMMENT},
            {StateT::COMMENT, TokenT::FUZZY, StateT::COMMENT},
            {StateT::COMMENT, TokenT::MSGCTXT, StateT::MSGCTXT},
            {StateT::COMMENT, TokenT::MSGID, StateT::MSGID},
            {StateT::COMMENT, TokenT::EOT, StateT::EOT},
            {StateT::MSGCTXT, TokenT::TEXT, StateT::MSGCTXT_TEXT},
            {StateT::MSGCTXT_TEXT, TokenT::TEXT, StateT::MSGCTXT_TEXT},
            {StateT::MSGCTXT_TEXT, TokenT::MSGID, StateT::MSGID},
            {StateT::MSGID, TokenT::TEXT, StateT::MSGID_TEXT},
            {StateT::MSGID, TokenT::ERROR, StateT::ERROR},
            {StateT::MSGID_TEXT, TokenT::TEXT, StateT::MSGID_TEXT},
            {StateT::MSGID_TEXT, TokenT::MSGID_PLURAL, StateT::MSGID_PLURAL},
            {StateT::MSGID_TEXT, TokenT::MSGSTR, StateT::MSGSTR},
            {StateT::MSGID_TEXT, TokenT::ERROR, StateT::ERROR},
            {StateT::MSGID_PLURAL, TokenT::TEXT, StateT::MSGID_PLURAL_TEXT},
            {StateT::MSGID_PLURAL, TokenT::ERROR, StateT::ERROR},
            {StateT::MSGID_PLURAL_TEXT, TokenT::TEXT, StateT::MSGID_PLURAL_TEXT},
            {StateT::MSGID_PLURAL_TEXT, TokenT::MSGSTR_PLURAL, StateT::MSGSTR_PLURAL},
            {StateT::MSGID_PLURAL_TEXT, TokenT::ERROR, StateT::ERROR},
            {StateT::MSGSTR, TokenT::TEXT, StateT::MSGSTR_TEXT},
            {StateT::MSGSTR, TokenT::ERROR, StateT::ERROR},
            {StateT::MSGSTR_TEXT, TokenT::TEXT, StateT::MSGSTR_TEXT},
            {StateT::MSGSTR_TEXT, TokenT::ERROR, StateT::ERROR},
            {StateT::MSGSTR_PLURAL, TokenT::TEXT, StateT::MSGSTR_PLURAL_TEXT},
            {StateT::MSGSTR_PLURAL, TokenT::ERROR, StateT::ERROR},
            {StateT::MSGSTR_PLURAL_TEXT, TokenT::TEXT, StateT::MSGSTR_PLURAL_TEXT},
            {StateT::MSGSTR_PLURAL_TEXT, TokenT::MSGSTR_PLURAL, StateT::MSGSTR_PLURAL},
            {StateT::MSGSTR_PLURAL_TEXT, TokenT::ERROR, StateT::ERROR},
            {StateT::END_OF_ENTRY, TokenT::COMMENT, StateT::COMMENT},
            {StateT::END_OF_ENTRY, TokenT::FUZZY, StateT::COMMENT},
            {StateT::END_OF_ENTRY, TokenT::MSGCTXT, StateT::MSGCTXT},
            {StateT::END_OF_ENTRY, TokenT::MSGID, StateT::MSGID},
            {StateT::END_OF_ENTRY, TokenT::EOT, StateT::EOT},
            {StateT::ABORT_ENTRY, TokenT::COMMENT, StateT::COMMENT},
            {StateT::ABORT_ENTRY, TokenT::FUZZY, StateT::COMMENT},
            {StateT::ABORT_ENTRY, TokenT::MSGCTXT, StateT::MSGCTXT},
            {StateT::ABORT_ENTRY, TokenT::MSGID, StateT::MSGID},
            {StateT::ABORT_ENTRY, TokenT::EOT, StateT::EOT},
            {StateT::ERROR_BEFORE_MSGID, TokenT::EOT, StateT::ABORT_ENTRY},
        };
        for (const auto &it : valid_def) {
            trans[static_cast<unsigned int>(it.cur)][static_cast<unsigned int>(it.token)] = it.next;
        }
    }

    inline PoParser::StateT PoParser::StateTransTable::GetState(StateT state, TokenT token) const noexcept
    {
        return trans[static_cast<unsigned int>(state)][static_cast<unsigned int>(token)];
    }

    // Parse all PO entries.
    template <typename INP, typename Sentinel>
    std::vector<PoParser::PoEntryT> PoParser::GetEntries(INP &&begin, Sentinel &&end)
    {
        static const PoParser::StateTransTable transTable;
        std::vector<PoEntryT> entries;
        CharFeeder<INP, Sentinel> it(begin, end, LocationT());
        StateT state = StateT::END_OF_ENTRY;
        bool fuzzy = false;
        bool hasMsgctxt = false;
        PoEntryT curEntry;
        std::string text;
        while (state != StateT::EOT) {
            TokenT token = TokenT::ERROR;
            LocationT loc;
            std::size_t n_msgstr = 0;
            try {
                token = PoParser::Lex(it, loc, text, n_msgstr);
            } catch (PoParseError &e) {
                token = TokenT::ERROR;
                if (curEntry.error.empty()) {
                    // Report only the error that causes an error.
                    const LocationT &eloc = e.GetLocation();
                    curEntry.error = eloc.ToString() + e.what();
                }
            }
            state = transTable.GetState(state, token);
            if (state == StateT::END_OF_ENTRY || state == StateT::ABORT_ENTRY) {
                if (state == StateT::ABORT_ENTRY && curEntry.error.empty()) {
                    // Report only the error that causes an error.
                    curEntry.error = loc.ToString() + "Unexpected " + GetTokenName(token) + " (the previous entry is incomplete).";
                }
                // register the current entry
                if (!curEntry.error.empty()) {
                    curEntry.msgid.clear();
                    curEntry.msgstr.clear();
                } else if (fuzzy) {
                    curEntry.msgstr[0].clear();
                }
                entries.push_back(curEntry);
                // initialize the new entry
                curEntry.msgid.clear();
                curEntry.msgstr.clear();
                curEntry.error.clear();
                fuzzy = false;
                hasMsgctxt = false;
                state = transTable.GetState(state, token);
            }
            switch (state) {
            case StateT::ERROR:
            case StateT::ERROR_BEFORE_MSGID:
                // Report only the error that causes an error.
                if (curEntry.error.empty()) {
                    curEntry.error = loc.ToString() + "Unexpected " + GetTokenName(token) + '.';
                }
                if (token == TokenT::ERROR) {
                    // Try to recover lexical error
                    while (it.IsNotEnd() && it.Get() != '\n') {
                        it.Next();
                    }
                }
                break;
            case StateT::COMMENT:
                fuzzy |= token == TokenT::FUZZY;
                break;
            case StateT::MSGCTXT_TEXT:
            case StateT::MSGID_TEXT:
                curEntry.msgid += text;
                break;
            case StateT::MSGCTXT:
                hasMsgctxt = true;
                break;
            case StateT::MSGID:
                if (hasMsgctxt) {
                    curEntry.msgid += CONTEXT_SEPARATOR;
                    hasMsgctxt = false;
                }
                break;
            case StateT::MSGSTR:
            case StateT::MSGSTR_PLURAL:
                if (n_msgstr != curEntry.msgstr.size()) {
                    curEntry.error = loc.ToString() + "Invalid n in msgstr[n]; n should be " + std::to_string(curEntry.msgstr.size()) + " but " + std::to_string(n_msgstr) + '.';
                    state = StateT::ERROR;
                } else {
                    curEntry.msgstr.emplace_back("");
                }
                break;
            case StateT::MSGSTR_TEXT:
            case StateT::MSGSTR_PLURAL_TEXT:
                curEntry.msgstr.back() += text;
                break;
            default:
                // do nothing
                break;
            }
        }
        return entries;
    }
} // namespace spiritless_po

#endif // SPIRITLESS_PO_PO_PARSER_H_
