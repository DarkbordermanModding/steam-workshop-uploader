#ifndef STEAMWORKSHOP_HPP
#define STEAMWORKSHOP_HPP

#include <fstream>
#include <iostream>

#include "libsteam/steam_api.h"
#include "libsteam/steam_gameserver.h"
#include "parser.hpp"
#include "utilities.hpp"

class Workshop
{
    public:
        CCallResult <Workshop, SubmitItemUpdateResult_t> m_SubmitItemUpdateResult;
        CCallResult <Workshop, CreateItemResult_t> m_CreateItemResult;
        SubmitItemUpdateResult_t SubmitItemUpdateResult;
        CreateItemResult_t CreateItemResult;
        int create_status = 0;
        int created_publishfield = 0;
        int submit_status = 0;

        void StartCreate(int app_id){
            SteamAPICall_t callback = SteamUGC()->CreateItem(app_id, static_cast<EWorkshopFileType>(0));
            m_CreateItemResult.Set(callback, this, &Workshop::OnGetCreateStatus);
        }

        void StartUpdate(Metadata &metadata){
            if(metadata.publishedfield_id == 0){
                metadata.publishedfield_id = created_publishfield;
            }
            UGCUpdateHandle_t handle = SteamUGC()->StartItemUpdate(metadata.app_id, metadata.publishedfield_id);
            SteamUGC()->SetItemTitle(handle, metadata.title.c_str());
            SteamUGC()->SetItemDescription(handle, metadata.description.c_str());
            SteamUGC()->SetItemVisibility(handle, metadata.visibility);
            SteamUGC()->SetItemTags(handle, &metadata.tags);
            // WARN: steam_appid.txt MUST MATCH APP_ID
            SteamUGC()->SetItemPreview(handle, metadata.preview_path.c_str());
            SteamUGC()->SetItemContent(handle, metadata.content_folder.c_str());
            SteamAPICall_t callback = SteamUGC()->SubmitItemUpdate(handle, NULL);
            m_SubmitItemUpdateResult.Set(callback, this, &Workshop::OnGetUploadStatus);
        }

        void display_remote_storage_quota(){
            unsigned long long int total_bytes, available_bytes;
            SteamRemoteStorage()->GetQuota(&total_bytes, &available_bytes);
            cout << endl << "Available Cloud storage: " << available_bytes / 1024 << " KB" << endl;
            cout << "Total Cloud storage: " << total_bytes / 1024 << " KB" << endl;
        }

        void display_result(){
            if(this-> SubmitItemUpdateResult.m_eResult == 1){
                std::cout << "Upload success" << endl;
                std::cout << "Publishedfield ID: " << this->SubmitItemUpdateResult.m_nPublishedFileId << endl;
                std::cout << "Web page URL: https://steamcommunity.com/sharedfiles/filedetails/?id=" + to_string(this->SubmitItemUpdateResult.m_nPublishedFileId) << endl;
                std::cout << "If this is newly created workshop item, remember to update publishedfield_id" << endl;
            }
            else{
                std::cout << "Upload enounter failures" << endl;
                std::cout << "Upload result: " << this-> SubmitItemUpdateResult.m_eResult << endl;
            }
        }

    private:
        void OnGetCreateStatus(CreateItemResult_t *result, bool bIOFailure){
            if(!bIOFailure){
                this -> CreateItemResult = *result;
                this -> created_publishfield = result->m_nPublishedFileId;
                this -> create_status = 1;
            }else{
                this -> create_status = -1;
            }
        }

        void OnGetUploadStatus(SubmitItemUpdateResult_t *result, bool bIOFailure){
            if(!bIOFailure){
                this-> SubmitItemUpdateResult = *result;
                this-> submit_status = 1;
            }
            else{
                this->submit_status = -1;
            }
        }
};
#endif
