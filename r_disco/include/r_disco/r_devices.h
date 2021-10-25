
#ifndef r_disco_r_devices_h
#define r_disco_r_devices_h

#include "r_disco/r_camera.h"
#include "r_disco/r_stream_config.h"
#include "r_db/r_sqlite_conn.h"
#include "r_utils/r_work_q.h"
#include <vector>
#include <thread>
#include <string>

namespace r_disco
{

enum r_devices_cmd_type
{
    INSERT_OR_UPDATE_DEVICES = 0,
    GET_ALL_CAMERAS = 1,
    GET_ASSIGNED_CAMERAS = 2,
    SAVE_CAMERA = 3,
    GET_MODIFIED_CAMERAS = 4,
    GET_ASSIGNED_CAMERAS_ADDED = 5,
    GET_ASSIGNED_CAMERAS_REMOVED = 6
};

struct r_devices_cmd
{
    r_devices_cmd_type type;
    std::vector<std::pair<r_stream_config, std::string>> configs;
    r_camera camera;
};

struct r_devices_cmd_result
{
    std::vector<r_camera> cameras;
};

class r_devices
{
public:
    r_devices(const std::string& top_dir);
    ~r_devices() noexcept;

    void start();
    void stop();

    void insert_or_update_devices(const std::vector<std::pair<r_stream_config, std::string>>& stream_configs);
    std::vector<r_camera> get_all_cameras();
    std::vector<r_camera> get_assigned_cameras();
    void save_camera(const r_camera& camera);

    std::vector<r_camera> get_modified_cameras(const std::vector<r_camera>& cameras);
    std::vector<r_camera> get_assigned_cameras_added(const std::vector<r_camera>& cameras);
    std::vector<r_camera> get_assigned_cameras_removed(const std::vector<r_camera>& cameras);

private:
    void _entry_point();

    r_db::r_sqlite_conn _open_or_create_db(const std::string& top_dir) const;
    void _upgrade_db(const r_db::r_sqlite_conn& conn) const;

    int _get_db_version(const r_db::r_sqlite_conn& conn) const;
    void _set_db_version(const r_db::r_sqlite_conn& conn, int version) const;

    std::string _create_insert_or_update_query(const r_db::r_sqlite_conn& conn, const r_stream_config& stream_config, const std::string& hash) const;
    r_camera _create_camera(const std::map<std::string, r_utils::r_nullable<std::string>>& row) const;

    r_devices_cmd_result _insert_or_update_devices(const r_db::r_sqlite_conn& conn, const std::vector<std::pair<r_stream_config, std::string>>& stream_configs) const;
    r_devices_cmd_result _get_all_cameras(const r_db::r_sqlite_conn& conn) const;
    r_devices_cmd_result _get_assigned_cameras(const r_db::r_sqlite_conn& conn) const;
    r_devices_cmd_result _save_camera(const r_db::r_sqlite_conn& conn, const r_camera& camera) const;

    std::thread _th;
    bool _running;
    std::string _top_dir;
    r_utils::r_work_q<r_devices_cmd, r_devices_cmd_result> _db_work_q;
};

}

#endif
