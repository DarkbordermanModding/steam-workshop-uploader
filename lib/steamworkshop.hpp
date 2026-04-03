#ifndef STEAMWORKSHOP_HPP
#define STEAMWORKSHOP_HPP

#include <fstream>
#include <iostream>
#include <vector>

#include "libsteam/steam_api.h"
#include "libsteam/steam_gameserver.h"
#include "parser.hpp"
#include "utilities.hpp"

class Workshop
{
    public:
        CCallResult<Workshop, SubmitItemUpdateResult_t> m_SubmitItemUpdateResult;
        CCallResult<Workshop, CreateItemResult_t> m_CreateItemResult;
        SubmitItemUpdateResult_t SubmitItemUpdateResult;
        CreateItemResult_t CreateItemResult;
        int create_status = 0;
        int created_publishfield = 0;
        int submit_status = 0;

        // Dep query state
        CCallResult<Workshop, SteamUGCQueryCompleted_t> m_ModDepQueryResult;
        CCallResult<Workshop, GetAppDependenciesResult_t> m_DLCDepQueryResult;
        int mod_query_status = 0;   // 0=pending, 1=done, -1=failed
        int dlc_query_status = 0;
        vector<PublishedFileId_t> existing_mod_deps;
        vector<AppId_t> existing_dlc_deps;
        uint32 total_dlc_deps = 0;

        // Dep ops state
        struct DepOp {
            enum Type { REMOVE_MOD, REMOVE_DLC, ADD_MOD, ADD_DLC } type;
            uint64 id;
        };
        vector<DepOp> dep_op_queue;
        size_t dep_op_index = 0;
        int dep_sync_status = 0;    // 0=running, 1=done
        PublishedFileId_t dep_op_publishedfield_id = 0;

        CCallResult<Workshop, RemoveUGCDependencyResult_t> m_RemoveModDepResult;
        CCallResult<Workshop, AddUGCDependencyResult_t> m_AddModDepResult;
        CCallResult<Workshop, RemoveAppDependencyResult_t> m_RemoveDLCDepResult;
        CCallResult<Workshop, AddAppDependencyResult_t> m_AddDLCDepResult;

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

        void StartModDepQuery(PublishedFileId_t fileId) {
            UGCQueryHandle_t qHandle = SteamUGC()->CreateQueryUGCDetailsRequest(&fileId, 1);
            SteamUGC()->SetReturnChildren(qHandle, true);
            SteamAPICall_t call = SteamUGC()->SendQueryUGCRequest(qHandle);
            m_ModDepQueryResult.Set(call, this, &Workshop::OnModDepQuery);
        }

        void StartDLCDepQuery(PublishedFileId_t fileId) {
            SteamAPICall_t call = SteamUGC()->GetAppDependencies(fileId);
            m_DLCDepQueryResult.Set(call, this, &Workshop::OnDLCDepQuery);
        }

        void StartDepOps(PublishedFileId_t fileId, Metadata& metadata) {
            dep_op_publishedfield_id = fileId;

            if (metadata.sync_required_publishedfield_ids && mod_query_status == 1) {
                for (auto& existing : existing_mod_deps) {
                    bool keep = false;
                    for (auto& wanted : metadata.required_publishedfield_ids)
                        if (wanted == existing) { keep = true; break; }
                    if (!keep) dep_op_queue.push_back({DepOp::REMOVE_MOD, existing});
                }
                for (auto& wanted : metadata.required_publishedfield_ids) {
                    bool already = false;
                    for (auto& existing : existing_mod_deps)
                        if (wanted == existing) { already = true; break; }
                    if (!already) dep_op_queue.push_back({DepOp::ADD_MOD, wanted});
                }
            }

            if (metadata.sync_required_app_ids && dlc_query_status == 1) {
                for (auto& existing : existing_dlc_deps) {
                    bool keep = false;
                    for (auto& wanted : metadata.required_app_ids)
                        if ((AppId_t)wanted == existing) { keep = true; break; }
                    if (!keep) dep_op_queue.push_back({DepOp::REMOVE_DLC, existing});
                }
                for (auto& wanted : metadata.required_app_ids) {
                    bool already = false;
                    for (auto& existing : existing_dlc_deps)
                        if ((AppId_t)wanted == existing) { already = true; break; }
                    if (!already) dep_op_queue.push_back({DepOp::ADD_DLC, wanted});
                }
            }

            if (dep_op_queue.empty()) {
                dep_sync_status = 1;
                return;
            }
            ExecuteNextDepOp();
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

        void OnModDepQuery(SteamUGCQueryCompleted_t* result, bool bIOFailure) {
            if (bIOFailure || result->m_eResult != k_EResultOK) {
                mod_query_status = -1;
                return;
            }
            SteamUGCDetails_t details;
            SteamUGC()->GetQueryUGCResult(result->m_handle, 0, &details);
            if (details.m_unNumChildren > 0) {
                existing_mod_deps.resize(details.m_unNumChildren);
                SteamUGC()->GetQueryUGCChildren(result->m_handle, 0, existing_mod_deps.data(), details.m_unNumChildren);
            }
            SteamUGC()->ReleaseQueryUGCRequest(result->m_handle);
            mod_query_status = 1;
        }

        void OnDLCDepQuery(GetAppDependenciesResult_t* result, bool bIOFailure) {
            if (bIOFailure || result->m_eResult != k_EResultOK) {
                dlc_query_status = -1;
                return;
            }
            for (uint32 i = 0; i < result->m_nNumAppDependencies; i++)
                existing_dlc_deps.push_back(result->m_rgAppIDs[i]);
            if (existing_dlc_deps.size() >= result->m_nTotalNumAppDependencies)
                dlc_query_status = 1;
        }

        void ExecuteNextDepOp() {
            if (dep_op_index >= dep_op_queue.size()) {
                dep_sync_status = 1;
                return;
            }
            DepOp& op = dep_op_queue[dep_op_index];
            SteamAPICall_t call;
            switch (op.type) {
                case DepOp::REMOVE_MOD:
                    call = SteamUGC()->RemoveDependency(dep_op_publishedfield_id, op.id);
                    m_RemoveModDepResult.Set(call, this, &Workshop::OnRemoveModDep);
                    break;
                case DepOp::ADD_MOD:
                    call = SteamUGC()->AddDependency(dep_op_publishedfield_id, op.id);
                    m_AddModDepResult.Set(call, this, &Workshop::OnAddModDep);
                    break;
                case DepOp::REMOVE_DLC:
                    call = SteamUGC()->RemoveAppDependency(dep_op_publishedfield_id, (AppId_t)op.id);
                    m_RemoveDLCDepResult.Set(call, this, &Workshop::OnRemoveDLCDep);
                    break;
                case DepOp::ADD_DLC:
                    call = SteamUGC()->AddAppDependency(dep_op_publishedfield_id, (AppId_t)op.id);
                    m_AddDLCDepResult.Set(call, this, &Workshop::OnAddDLCDep);
                    break;
            }
        }

        void OnRemoveModDep(RemoveUGCDependencyResult_t* result, bool bIOFailure) {
            if (bIOFailure || result->m_eResult != k_EResultOK)
                cout << "Warning: failed to remove mod dependency " << dep_op_queue[dep_op_index].id << endl;
            dep_op_index++;
            ExecuteNextDepOp();
        }

        void OnAddModDep(AddUGCDependencyResult_t* result, bool bIOFailure) {
            if (bIOFailure || result->m_eResult != k_EResultOK)
                cout << "Warning: failed to add mod dependency " << dep_op_queue[dep_op_index].id << endl;
            dep_op_index++;
            ExecuteNextDepOp();
        }

        void OnRemoveDLCDep(RemoveAppDependencyResult_t* result, bool bIOFailure) {
            if (bIOFailure || result->m_eResult != k_EResultOK)
                cout << "Warning: failed to remove DLC dependency " << dep_op_queue[dep_op_index].id << endl;
            dep_op_index++;
            ExecuteNextDepOp();
        }

        void OnAddDLCDep(AddAppDependencyResult_t* result, bool bIOFailure) {
            if (bIOFailure || result->m_eResult != k_EResultOK)
                cout << "Warning: failed to add DLC dependency " << dep_op_queue[dep_op_index].id << endl;
            dep_op_index++;
            ExecuteNextDepOp();
        }
};
#endif
