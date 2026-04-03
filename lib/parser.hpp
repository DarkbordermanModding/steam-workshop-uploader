#ifndef PARSER_HPP
#define PARSER_HPP

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "libsteam/isteamremotestorage.h"
#include "md_to_steam.hpp"
#include "utilities.hpp"

using namespace std;

class Metadata {
    public:
        // Steam update fields
        unsigned long app_id;
        unsigned long publishedfield_id;
        string title;
        string description;
        string preview_path;
        string content_folder;
        ERemoteStoragePublishedFileVisibility visibility;
        SteamParamStringArray_t tags;
        // Store tag string for display
        string tags_str;
        // Dependency sync fields
        bool sync_required_publishedfield_ids = false;
        vector<PublishedFileId_t> required_publishedfield_ids;
        bool sync_required_app_ids = false;
        vector<AppId_t> required_app_ids;

    Metadata(string path){
        string directory = getAbsoluteDirectory(path);

        this->app_id = stoull(exec("yq -r \".workshop.app_id\" " + path));
        this->publishedfield_id = stoull(exec("yq -r \".workshop.publishedfield_id\" " + path));
        this->visibility = static_cast<ERemoteStoragePublishedFileVisibility>(stoi(exec("yq -r \".workshop.visibility\" " + path)));
        this->title = exec("yq -r \".workshop.title\" " + path);

        string desc_path_raw = exec("yq -r \".workshop.description_path\" " + path);
        if (desc_path_raw != "null" && !desc_path_raw.empty()) {
            string desc_path = getAbsolutePath(directory, desc_path_raw);
            ifstream desc_file(desc_path);
            if (!desc_file.is_open()) {
                throw runtime_error("description_path not found: " + desc_path);
            }
            string md_content((istreambuf_iterator<char>(desc_file)), istreambuf_iterator<char>());
            this->description = md_to_steam(md_content);
        } else {
            this->description = exec("yq -r \".workshop.description\" " + path);
        }

        this->content_folder = getAbsolutePath(directory, exec("yq -r \".workshop.content_folder\" " + path));
        this->preview_path = getAbsolutePath(directory, exec("yq -r \".workshop.preview_path\" " + path));

        // Split tag string to vector<string>
        this->tags_str = exec("yq -r \".workshop.tags | join(\\\",\\\")\" " + path);
        vector<string> tags = split_string(this->tags_str, ',');
        // Then convert to Steamworks SDK structure
        const char** m_ppStrings = new const char*[tags.size()];
        for (size_t i = 0; i < tags.size(); ++i) {
            m_ppStrings[i] = tags[i].c_str();
        }

        this->tags.m_ppStrings = m_ppStrings;
        this->tags.m_nNumStrings = tags.size();

        string req_items_check = exec("yq -r \".workshop.required_publishedfield_ids\" " + path);
        if (req_items_check != "null") {
            this->sync_required_publishedfield_ids = true;
            string ids_str = exec("yq -r \".workshop.required_publishedfield_ids | join(\\\",\\\")\" " + path);
            if (!ids_str.empty()) {
                for (auto& s : split_string(ids_str, ','))
                    if (!s.empty()) this->required_publishedfield_ids.push_back(stoull(s));
            }
        }

        string req_apps_check = exec("yq -r \".workshop.required_app_ids\" " + path);
        if (req_apps_check != "null") {
            this->sync_required_app_ids = true;
            string ids_str = exec("yq -r \".workshop.required_app_ids | join(\\\",\\\")\" " + path);
            if (!ids_str.empty()) {
                for (auto& s : split_string(ids_str, ','))
                    if (!s.empty()) this->required_app_ids.push_back(stoul(s));
            }
        }
    }

    void sync_steam_appid_txt(){
        cout << "Sync app_id.txt from yml, create if not exist" << endl;
        ofstream outFile("steam_appid.txt");
        outFile << this->app_id;
        outFile.close();
    }

    void display_workshop_metadata(){
        cout << "app_id: " << this->app_id << endl;
        cout << "publishfield_id: " << this->publishedfield_id << endl;
        cout << "visibility: " << this->visibility << endl;
        cout << "title: " << this->title << endl;
        string safe_desc;
        for (unsigned char c : this->description)
            safe_desc += (c < 0x80) ? (char)c : '?';
        cout << "description: " << safe_desc << endl;
        cout << "preview path: " << this->preview_path << endl;
        cout << "content folder: " << this->content_folder << endl;
        cout << "tags: " << this->tags_str << endl;
        if (this->sync_required_publishedfield_ids) {
            cout << "required publishedfield ids: ";
            for (size_t i = 0; i < this->required_publishedfield_ids.size(); i++) {
                if (i > 0) cout << ",";
                cout << this->required_publishedfield_ids[i];
            }
            cout << endl;
        } else {
            cout << "required publishedfield ids: (unmodified)" << endl;
        }
        if (this->sync_required_app_ids) {
            cout << "required app ids: ";
            for (size_t i = 0; i < this->required_app_ids.size(); i++) {
                if (i > 0) cout << ",";
                cout << this->required_app_ids[i];
            }
            cout << endl;
        } else {
            cout << "required app ids: (unmodified)" << endl;
        }
        cout << endl;
    }
};
#endif
