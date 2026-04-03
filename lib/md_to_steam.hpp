#ifndef MD_TO_STEAM_HPP
#define MD_TO_STEAM_HPP

#include <regex>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

// Trim leading/trailing whitespace
static string md_trim(const string& s) {
    size_t start = s.find_first_not_of(" \t\r");
    if (start == string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r");
    return s.substr(start, end - start + 1);
}

// Split a table row "| a | b |" into {"a", "b"}
static vector<string> split_table_row(const string& row) {
    vector<string> cells;
    string r = row;
    if (!r.empty() && r.front() == '|') r = r.substr(1);
    if (!r.empty() && r.back() == '|') r.pop_back();
    istringstream ss(r);
    string cell;
    while (getline(ss, cell, '|')) {
        size_t s = cell.find_first_not_of(" \t");
        size_t e = cell.find_last_not_of(" \t");
        cells.push_back(s == string::npos ? "" : cell.substr(s, e - s + 1));
    }
    return cells;
}

// Returns true for table separator rows like "|---|:---:|"
static bool is_table_separator(const string& trimmed) {
    return !trimmed.empty() &&
           trimmed.find('-') != string::npos &&
           trimmed.find_first_not_of("|-: \t") == string::npos;
}

// Returns true for HR lines: 3+ of the same char (-, *, _) with optional spaces
static bool is_hr(const string& trimmed) {
    if (trimmed.size() < 3) return false;
    for (char sep : {'-', '*', '_'}) {
        string allowed = string(1, sep) + " ";
        if (trimmed.find_first_not_of(allowed) == string::npos) {
            size_t cnt = 0;
            for (char c : trimmed) if (c == sep) ++cnt;
            if (cnt >= 3) return true;
        }
    }
    return false;
}

// Apply inline formatting rules, protecting code spans from other rules
static string md_apply_inline(const string& line) {
    // Step 1: extract and protect inline code spans
    vector<string> code_spans;
    string s;
    s.reserve(line.size());
    for (size_t i = 0; i < line.size(); ) {
        if (line[i] == '`') {
            size_t end = line.find('`', i + 1);
            if (end != string::npos) {
                code_spans.push_back("[code]" + line.substr(i + 1, end - i - 1) + "[/code]");
                s += "\x01" + to_string(code_spans.size() - 1) + "\x01";
                i = end + 1;
                continue;
            }
        }
        s += line[i++];
    }

    // Step 2: inline rules (order matters: longer patterns first)
    s = regex_replace(s, regex(R"(\*\*\*(.+?)\*\*\*)"),     "[b][i]$1[/i][/b]");
    s = regex_replace(s, regex(R"(\*\*(.+?)\*\*)"),          "[b]$1[/b]");
    s = regex_replace(s, regex(R"(__(.+?)__)"),               "[b]$1[/b]");
    s = regex_replace(s, regex(R"(\*([^*\n]+?)\*)"),          "[i]$1[/i]");
    s = regex_replace(s, regex(R"(_([^_\n]+?)_)"),            "[i]$1[/i]");
    s = regex_replace(s, regex(R"(~~(.+?)~~)"),               "[strike]$1[/strike]");
    s = regex_replace(s, regex(R"(!\[[^\]]*\]\([^)]*\))"),   "");  // remove images
    s = regex_replace(s, regex(R"(\[([^\]]+)\]\(([^)]+)\))"), "[url=$2]$1[/url]");

    // Step 3: restore code spans
    for (size_t k = 0; k < code_spans.size(); ++k) {
        string placeholder = "\x01" + to_string(k) + "\x01";
        size_t pos = s.find(placeholder);
        if (pos != string::npos)
            s.replace(pos, placeholder.size(), code_spans[k]);
    }

    return s;
}

string md_to_steam(const string& markdown) {
    vector<string> lines;
    {
        istringstream stream(markdown);
        string line;
        while (getline(stream, line)) {
            if (!line.empty() && line.back() == '\r')
                line.pop_back();
            lines.push_back(line);
        }
    }

    string result;
    size_t i = 0;

    while (i < lines.size()) {
        const string& raw = lines[i];
        const string trimmed = md_trim(raw);

        // Fenced code block
        if (trimmed.size() >= 3 && trimmed.substr(0, 3) == "```") {
            result += "[code]\n";
            ++i;
            while (i < lines.size()) {
                string inner = lines[i];
                if (!inner.empty() && inner.back() == '\r') inner.pop_back();
                if (md_trim(inner).substr(0, 3) == "```") { ++i; break; }
                result += inner + "\n";
                ++i;
            }
            result += "[/code]\n";
            continue;
        }

        // Table (header row followed by separator row)
        if (trimmed.find('|') != string::npos &&
            i + 1 < lines.size() &&
            is_table_separator(md_trim(lines[i + 1]))) {

            result += "[table]\n[tr]";
            for (const string& h : split_table_row(trimmed))
                result += "[th]" + md_apply_inline(h) + "[/th]";
            result += "[/tr]\n";
            i += 2; // skip header + separator

            while (i < lines.size()) {
                string dt = md_trim(lines[i]);
                if (dt.find('|') == string::npos) break;
                result += "[tr]";
                for (const string& c : split_table_row(dt))
                    result += "[td]" + md_apply_inline(c) + "[/td]";
                result += "[/tr]\n";
                ++i;
            }
            result += "[/table]\n";
            continue;
        }

        // Horizontal rule
        if (is_hr(trimmed)) {
            result += "[hr][/hr]\n";
            ++i;
            continue;
        }

        // ATX heading
        if (!trimmed.empty() && trimmed[0] == '#') {
            size_t level = 0;
            while (level < trimmed.size() && trimmed[level] == '#') ++level;
            if (level < trimmed.size() && trimmed[level] == ' ') {
                string text = md_apply_inline(md_trim(trimmed.substr(level + 1)));
                if      (level == 1) result += "[h1]" + text + "[/h1]\n";
                else if (level == 2) result += "[h2]" + text + "[/h2]\n";
                else                 result += "[h3]" + text + "[/h3]\n";
                ++i;
                continue;
            }
        }

        // Blockquote
        if (!trimmed.empty() && trimmed[0] == '>') {
            vector<string> qlines;
            while (i < lines.size()) {
                string ql = md_trim(lines[i]);
                if (ql.empty() || ql[0] != '>') break;
                string inner = (ql.size() >= 2 && ql[1] == ' ') ? ql.substr(2) : ql.substr(1);
                qlines.push_back(md_apply_inline(md_trim(inner)));
                ++i;
            }
            result += "[quote]";
            for (size_t j = 0; j < qlines.size(); ++j) {
                if (j > 0) result += "\n";
                result += qlines[j];
            }
            result += "[/quote]\n";
            continue;
        }

        // Unordered list
        if (trimmed.size() >= 2 &&
            (trimmed[0] == '-' || trimmed[0] == '*' || trimmed[0] == '+') &&
            trimmed[1] == ' ') {
            result += "[list]\n";
            while (i < lines.size()) {
                string item = md_trim(lines[i]);
                if (item.size() < 2 ||
                    !((item[0] == '-' || item[0] == '*' || item[0] == '+') && item[1] == ' '))
                    break;
                result += "[*]" + md_apply_inline(item.substr(2)) + "\n";
                ++i;
            }
            result += "[/list]\n";
            continue;
        }

        // Ordered list
        if (!trimmed.empty() && isdigit((unsigned char)trimmed[0])) {
            size_t dot = trimmed.find(". ");
            if (dot != string::npos) {
                bool all_digits = true;
                for (size_t j = 0; j < dot; ++j)
                    if (!isdigit((unsigned char)trimmed[j])) { all_digits = false; break; }
                if (all_digits) {
                    result += "[olist]\n";
                    while (i < lines.size()) {
                        string item = md_trim(lines[i]);
                        size_t d = item.find(". ");
                        if (d == string::npos) break;
                        bool digits = true;
                        for (size_t j = 0; j < d; ++j)
                            if (!isdigit((unsigned char)item[j])) { digits = false; break; }
                        if (!digits) break;
                        result += "[*]" + md_apply_inline(item.substr(d + 2)) + "\n";
                        ++i;
                    }
                    result += "[/olist]\n";
                    continue;
                }
            }
        }

        // Blank line or regular paragraph text
        result += (trimmed.empty() ? "" : md_apply_inline(raw)) + "\n";
        ++i;
    }

    return result;
}

#endif
