
#ifndef r_disco_r_devices_h
#define r_disco_r_devices_h

#include "r_disco/r_camera.h"
#include "r_disco/r_stream_config.h"
#include "r_db/r_sqlite_conn.h"
#include "r_utils/r_work_q.h"
#include <vector>
#include <thread>
#include <string>
#include <map>

namespace r_disco
{

enum r_devices_cmd_type
{
    INSERT_OR_UPDATE_DEVICES = 0,
    GET_CAMERA_BY_ID = 1,
    GET_ALL_CAMERAS = 2,
    GET_ASSIGNED_CAMERAS = 3,
    SAVE_CAMERA = 4,
    REMOVE_CAMERA = 5,
    GET_MODIFIED_CAMERAS = 6,
    GET_ASSIGNED_CAMERAS_ADDED = 7,
    GET_ASSIGNED_CAMERAS_REMOVED = 8,
    GET_CREDENTIALS_BY_ID = 9
};

struct r_devices_cmd
{
    r_devices_cmd_type type;
    std::vector<std::pair<r_stream_config, std::string>> configs;
    std::vector<r_camera> cameras;
    std::string id;
};

struct r_devices_cmd_result
{
    std::vector<r_camera> cameras;
    std::pair<r_utils::r_nullable<std::string>, r_utils::r_nullable<std::string>> credentials;
};

class r_devices
{
public:
    r_devices(const std::string& top_dir);
    ~r_devices() noexcept;

    void start();
    void stop();

    void insert_or_update_devices(const std::vector<std::pair<r_stream_config, std::string>>& stream_configs);
    r_utils::r_nullable<r_camera> get_camera_by_id(const std::string& id);
    std::vector<r_camera> get_all_cameras();
    std::vector<r_camera> get_assigned_cameras();
    void save_camera(const r_camera& camera);
    void remove_camera(const r_camera& camera);
    void assign_camera(r_camera& camera);

    std::vector<r_camera> get_modified_cameras(const std::vector<r_camera>& cameras);
    std::vector<r_camera> get_assigned_cameras_added(const std::vector<r_camera>& cameras);
    std::vector<r_camera> get_assigned_cameras_removed(const std::vector<r_camera>& cameras);

    std::pair<r_utils::r_nullable<std::string>, r_utils::r_nullable<std::string>> get_credentials(const std::string& id);

private:
    void _entry_point();

    r_db::r_sqlite_conn _open_or_create_db(const std::string& top_dir) const;
    void _upgrade_db(const r_db::r_sqlite_conn& conn) const;

    int _get_db_version(const r_db::r_sqlite_conn& conn) const;
    void _set_db_version(const r_db::r_sqlite_conn& conn, int version) const;

    std::string _create_insert_or_update_query(const r_db::r_sqlite_conn& conn, const r_stream_config& stream_config, const std::string& hash) const;
    r_camera _create_camera(const std::map<std::string, r_utils::r_nullable<std::string>>& row) const;

    r_devices_cmd_result _insert_or_update_devices(const r_db::r_sqlite_conn& conn, const std::vector<std::pair<r_stream_config, std::string>>& stream_configs) const;
    r_devices_cmd_result _get_camera_by_id(const r_db::r_sqlite_conn& conn, const std::string& id) const;
    r_devices_cmd_result _get_all_cameras(const r_db::r_sqlite_conn& conn) const;
    r_devices_cmd_result _get_assigned_cameras(const r_db::r_sqlite_conn& conn) const;
    r_devices_cmd_result _save_camera(const r_db::r_sqlite_conn& conn, const r_camera& camera) const;
    r_devices_cmd_result _remove_camera(const r_db::r_sqlite_conn& conn, const r_camera& camera) const;
    r_devices_cmd_result _get_modified_cameras(const r_db::r_sqlite_conn& conn, const std::vector<r_camera>& cameras) const;
    r_devices_cmd_result _get_assigned_cameras_added(const r_db::r_sqlite_conn& conn, const std::vector<r_camera>& cameras) const;
    r_devices_cmd_result _get_assigned_cameras_removed(const r_db::r_sqlite_conn& conn, const std::vector<r_camera>& cameras) const;
    r_devices_cmd_result _get_credentials(const r_db::r_sqlite_conn& conn, const std::string& id);

    std::thread _th;
    bool _running;
    std::string _top_dir;
    r_utils::r_work_q<r_devices_cmd, r_devices_cmd_result> _db_work_q;
};

}

#endif
