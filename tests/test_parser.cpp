// Needs yq on PATH; run from the repo root
#include "../lib/parser.hpp"
#include "test_helpers.hpp"

const string FIXTURES = "tests/fixtures/";

string tag(const Metadata &m, int i){
    return m.tags.m_ppStrings[i];
}

bool throws(const string &file){
    try{ Metadata m(FIXTURES + file); }
    catch(const runtime_error &){ return true; }
    return false;
}

int main(){
    {
        Metadata m(FIXTURES + "full.yml");
        check("full: app_id", m.app_id, 480UL);
        check("full: publishedfield_id", (unsigned long long)m.publishedfield_id, 3844278080ULL);
        check("full: visibility", (int)m.visibility, 2);
        check("full: preview is absolute", m.preview_path, getAbsolutePath(getAbsoluteDirectory(FIXTURES + "full.yml"), "preview.png"));
        check("full: previews synced", m.sync_additional_previews, true);
        check("full: preview count", m.additional_previews.size(), (size_t)2);
        check("full: tags synced", m.sync_tags, true);
        check("full: tag count", m.tags.m_nNumStrings, 2);
        check("full: tag with spaces", tag(m, 0), string("Quality of Life"));
        check("full: tag 2", tag(m, 1), string("UI"));
        check("full: locale count", m.localizations.size(), (size_t)2);
        check("full: description_path converted", m.localizations["english"].description,
              string("[h1]Heading[/h1]\n\nSome [b]bold[/b] text.\n"));
        check("full: utf-8 title", m.localizations["schinese"].title, string("中文标题"));
        check("full: plain description", m.localizations["schinese"].description, string("中文描述"));
        check("full: required items synced", m.sync_required_publishedfield_ids, true);
        check("full: required items", m.required_publishedfield_ids.size() == 2
              && m.required_publishedfield_ids[0] == 111 && m.required_publishedfield_ids[1] == 222, true);
        check("full: required apps", m.required_app_ids.size() == 1 && m.required_app_ids[0] == 12345, true);
    }
    {
        Metadata m(FIXTURES + "minimal.yml");
        check("minimal: legacy root key works", m.app_id, 480UL);
        check("minimal: tags unmodified", m.sync_tags, false);
        check("minimal: previews unmodified", m.sync_additional_previews, false);
        check("minimal: required items unmodified", m.sync_required_publishedfield_ids, false);
        check("minimal: required apps unmodified", m.sync_required_app_ids, false);
    }
    {
        Metadata m(FIXTURES + "empty_lists.yml");
        check("empty: tags synced", m.sync_tags, true);
        check("empty: no tags", m.tags.m_nNumStrings, 0);
        check("empty: previews synced", m.sync_additional_previews, true);
        check("empty: no previews", m.additional_previews.size(), (size_t)0);
        check("empty: required items synced", m.sync_required_publishedfield_ids, true);
        check("empty: no required items", m.required_publishedfield_ids.size(), (size_t)0);
        check("empty: required apps synced", m.sync_required_app_ids, true);
        check("empty: no required apps", m.required_app_ids.size(), (size_t)0);
    }
    {
        Metadata m(FIXTURES + "large_id.yml");
        check("large: publishedfield_id over 32 bits", (unsigned long long)m.publishedfield_id, 5000000000ULL);
        check("large: required item over 32 bits", (unsigned long long)m.required_publishedfield_ids[0], 5000000001ULL);
    }
    check("both root keys rejected", throws("both_roots.yml"), true);
    check("missing root key rejected", throws("no_root.yml"), true);
    return finish();
}
