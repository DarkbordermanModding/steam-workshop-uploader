#ifndef PARSER_HPP
#define PARSER_HPP

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "libsteam/isteamremotestorage.h"
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

    Metadata(string path){
        string directory = getAbsoluteDirectory(path);

        this->app_id = stoull(exec("yq -r \".workshop.app_id\" " + path));
        this->publishedfield_id = stoull(exec("yq -r \".workshop.publishedfield_id\" " + path));
        this->visibility = static_cast<ERemoteStoragePublishedFileVisibility>(stoi(exec("yq -r \".workshop.visibility\" " + path)));
        this->title = exec("yq -r \".workshop.title\" " + path);
        this->description = exec("yq -r \".workshop.description\" " + path);
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
        cout << "description: " << this->description << endl;
        cout << "preview path: " << this->preview_path << endl;
        cout << "content folder: " << this->content_folder << endl;
        cout << "tags: " << this->tags_str << endl << endl;
    }
};
#endif
