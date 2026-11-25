#include "../lib/md_to_steam.hpp"
#include "test_helpers.hpp"

int main(){
    check("heading levels", md_to_steam("# A\n## B\n### C\n#### D"),
          string("[h1]A[/h1]\n[h2]B[/h2]\n[h3]C[/h3]\n[h3]D[/h3]\n"));
    check("hash without space is text", md_to_steam("#tag"), string("#tag\n"));
    check("bold and italic", md_to_steam("**b** *i* ***bi*** __b2__ _i2_"),
          string("[b]b[/b] [i]i[/i] [b][i]bi[/i][/b] [b]b2[/b] [i]i2[/i]\n"));
    check("strike", md_to_steam("~~gone~~"), string("[strike]gone[/strike]\n"));
    check("link", md_to_steam("[site](https://example.com)"),
          string("[url=https://example.com]site[/url]\n"));
    check("image removed", md_to_steam("before ![alt](a.png) after"), string("before  after\n"));
    check("inline code protected", md_to_steam("`**not bold**`"),
          string("[code]**not bold**[/code]\n"));
    check("fenced code kept raw", md_to_steam("```\n**raw**\n```"),
          string("[code]\n**raw**\n[/code]\n"));
    check("unordered list", md_to_steam("- a\n* b\n+ c"),
          string("[list]\n[*]a\n[*]b\n[*]c\n[/list]\n"));
    check("ordered list", md_to_steam("1. a\n2. b"),
          string("[olist]\n[*]a\n[*]b\n[/olist]\n"));
    check("blockquote", md_to_steam("> one\n> two"), string("[quote]one\ntwo[/quote]\n"));
    check("horizontal rule", md_to_steam("---"), string("[hr][/hr]\n"));
    check("table", md_to_steam("| A | B |\n|---|---|\n| 1 | **2** |"),
          string("[table]\n[tr][th]A[/th][th]B[/th][/tr]\n[tr][td]1[/td][td][b]2[/b][/td][/tr]\n[/table]\n"));
    check("CRLF line endings", md_to_steam("# A\r\ntext\r\n"), string("[h1]A[/h1]\ntext\n"));
    check("blank line kept", md_to_steam("a\n\nb"), string("a\n\nb\n"));
    return finish();
}
