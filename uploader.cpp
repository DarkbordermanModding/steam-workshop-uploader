#include <iostream>

#include "lib/libsteam/steam_api.h"
#include "lib/parser.hpp"
#include "lib/steamworkshop.hpp"

using namespace std;


int main(int argc, char *argv[]){
    if(argc != 2){
        cout << "Only need file path to run";
        return 1;
    }

    Metadata metadata = Metadata(argv[1]);

    metadata.sync_steam_appid_txt();
    metadata.display_workshop_metadata();

    bool status = SteamAPI_Init();
    if(status != 1){
        cout << "Steam init failed";
        return 1;
    }

    Workshop workshop;
    workshop.display_remote_storage_quota();

    cout << "Are you sure to continue?(y/n)" << endl;
    string prompt;
    cin >> prompt;
    if(prompt != "y"){
        cout << "not agree, return" << endl;
        return 0;
    }

    if(metadata.publishedfield_id == 0){
        workshop.StartCreate(metadata.app_id);
        while(true){
            SteamAPI_RunCallbacks();
            Sleep(1000);
            if(workshop.create_status == 1){
                cout << "Create action success" << endl;
                break;
            }
            else if(workshop.create_status == 0){
                cout << "Creating" << endl;
            }
            else if(workshop.create_status == -1){
                cout << "Create action failed" << endl;
                break;
            }
        }
    }

    workshop.StartUpdate(metadata);
    while(true){
        SteamAPI_RunCallbacks();
        Sleep(500);
        if(workshop.submit_status == 1){
            cout << "Upload action success" << endl;
            workshop.display_result();
            break;
        }
        else if(workshop.submit_status == 0){
            cout << "Uploading" << endl;
        }
        else if(workshop.submit_status == -1){
            cout << "Upload action failed" << endl;
            break;
        }
    }
    SteamAPI_Shutdown();
    return 0;
}
