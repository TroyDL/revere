
#include "r_disco/r_devices.h"
#include "r_utils/r_file.h"
#include "r_utils/r_string_utils.h"
#include "r_utils/r_exception.h"
#include <thread>

using namespace r_disco;
using namespace r_db;
using namespace r_utils;
using namespace r_utils::r_string_utils;
using namespace std;

r_devices::r_devices(const std::string& top_dir) :
    _th(),
    _running(false),
    _top_dir(top_dir)
{
}

r_devices::~r_devices() noexcept
{
    stop();
}

void r_devices::start()
{
    _running = true;
    _th = std::thread(&r_devices::_entry_point, this);
}

void r_devices::stop()
{
    if(_running)
    {
        _running = false;
        _db_work_q.wake();
        _th.join();
    }
}

void r_devices::insert_or_update_devices(const std::vector<r_stream_config>& stream_configs)
{
    r_devices_cmd cmd;
    cmd.type = INSERT_OR_UPDATE_DEVICES;
    cmd.configs = stream_configs;

    _db_work_q.post(cmd);
}

r_nullable<vector<r_camera>> r_devices::get_all_cameras()
{
    r_devices_cmd cmd;
    cmd.type = GET_ALL_CAMERAS;

    return _db_work_q.post(cmd).get().cameras;
}

r_nullable<vector<r_camera>> r_devices::get_assigned_cameras()
{
    r_devices_cmd cmd;
    cmd.type = GET_ASSIGNED_CAMERAS;

    return _db_work_q.post(cmd).get().cameras;
}

void r_devices::save_camera(const r_camera& camera)
{
    r_devices_cmd cmd;
    cmd.type = SAVE_CAMERA;
    cmd.camera = camera;
    _db_work_q.post(cmd).wait();
}

void r_devices::_entry_point()
{
    while(_running)
    {
        auto maybe_cmd = _db_work_q.poll();
        if(!maybe_cmd.is_null())
        {
            auto cmd = std::move(maybe_cmd.take());

            try
            {
                auto conn = _open_or_create_db(_top_dir);

                if(cmd.first.type == INSERT_OR_UPDATE_DEVICES)
                    cmd.second.set_value(_insert_or_update_devices(conn, cmd.first.configs));
                else if(cmd.first.type == GET_ALL_CAMERAS)
                    cmd.second.set_value(_get_all_cameras(conn));
                else if(cmd.first.type == GET_ASSIGNED_CAMERAS)
                    cmd.second.set_value(_get_assigned_cameras(conn));
                else if(cmd.first.type == SAVE_CAMERA)
                    cmd.second.set_value(_save_camera(conn, cmd.first.camera));
                else R_THROW(("Unknown work q command."));
            }
            catch(...)
            {
                try
                {
                    cmd.second.set_exception(std::current_exception());
                }
                catch(...)
                {
                    printf("EXCEPTION in error handling code for work q.");
                    fflush(stdout);
                    R_LOG_ERROR("EXCEPTION in error handling code for work q.");
                }
            }
        }
    }
}

r_sqlite_conn r_devices::_open_or_create_db(const string& top_dir) const
{
    auto db_dir = top_dir + r_fs::PATH_SLASH + "db";
    if(!r_fs::file_exists(db_dir))
        r_fs::mkdir(db_dir);

    auto conn = r_sqlite_conn(db_dir + r_fs::PATH_SLASH + "cameras.db");

    conn.exec(
        "CREATE TABLE IF NOT EXISTS cameras ("
            "id TEXT PRIMARY KEY NOT NULL UNIQUE, "
            "ipv4 TEXT NOT NULL, "
            "rtsp_url TEXT NOT NULL, "
            "rtsp_username TEXT, "
            "rtsp_password TEXT, "
            "video_codec TEXT NOT NULL, "
            "video_parameters TEXT, "
            "video_timebase INTEGER NOT NULL, "
            "audio_codec TEXT NOT NULL, "
            "audio_parameters TEXT, "
            "audio_timebase INTEGER NOT NULL, "
            "state TEXT NOT NULL"
        ");"
    );

    _upgrade_db(conn);

    return conn;
}

void r_devices::_upgrade_db(const r_sqlite_conn& conn) const
{
    auto current_version = _get_db_version(conn);

    switch(current_version)
    {
        // case 0 can serve as an example and model for future upgrades
        // Note: the cases purposefully fall through.
        case 0:
        {
            r_sqlite_transaction(conn,[&](const r_sqlite_conn& conn){
                // perform alter statements here
                _set_db_version(conn, 1);
            });
        }
        default:
            break;
    };
}

int r_devices::_get_db_version(const r_db::r_sqlite_conn& conn) const
{
    auto result = conn.exec("PRAGMA user_version;");
    if(result.empty())
        R_THROW(("Unable to query database version."));

    auto row = result.front();
    if(row.empty())
        R_THROW(("Invalid result from database query while fetching db version."));

    return s_to_int(row.begin()->second.value());
}

void r_devices::_set_db_version(const r_db::r_sqlite_conn& conn, int version) const
{
    conn.exec("PRAGMA user_version=" + to_string(version) + ";");
}

string r_devices::_create_insert_or_update_query(const r_db::r_sqlite_conn& conn, const r_stream_config& stream_config) const
{
    auto result = conn.exec(
        r_string_utils::format(
            "SELECT * FROM cameras WHERE id='%s';",
            stream_config.id.c_str()
        )
    );

    string query;

    if(result.empty())
    {
        query = "INSERT INTO cameras (id, ipv4, rtsp_url, video_codec";
        if(!stream_config.video_parameters.is_null())
            query += ", video_parameters";
        query += ", video_timebase, audio_codec";
        if(!stream_config.audio_parameters.is_null())
            query += ", audio_parameters";
        query += ", audio_timebase, state) VALUES (";

        query += r_string_utils::format(
            "'%s', '%s', '%s', '%s'",
            stream_config.id.c_str(),
            stream_config.ipv4.c_str(),
            stream_config.rtsp_url.c_str(),
            stream_config.video_codec.c_str()
        );

        if(!stream_config.video_parameters.is_null())
            query += r_string_utils::format(", '%s'", stream_config.video_parameters.value().c_str());
        
        query += r_string_utils::format(", %d, '%s'",
            stream_config.video_timebase,
            stream_config.audio_codec.c_str()
        );

        if(!stream_config.audio_parameters.is_null())
            query += r_string_utils::format(", '%s'", stream_config.audio_parameters.value().c_str());
        
        query += r_string_utils::format(", %d, '%s');",
            stream_config.audio_timebase, 
            "discovered"
        );
    }
    else
    {
        query = r_string_utils::format(
            "UPDATE cameras SET "
                "ipv4='%s', "
                "rtsp_url='%s', "
                "video_codec='%s', "
                "%s"
                "video_timebase=%d, "
                "audio_codec='%s', "
                "%s"
                "audio_timebase=%d "
            "WHERE id='%s';",
            stream_config.ipv4.c_str(),
            stream_config.rtsp_url.c_str(),
            stream_config.video_codec.c_str(),
            (!stream_config.video_parameters.is_null())?r_string_utils::format("video_parameters='%s', ", stream_config.video_parameters.value().c_str()).c_str():"",
            stream_config.video_timebase,
            stream_config.audio_codec.c_str(),
            (!stream_config.audio_parameters.is_null())?r_string_utils::format("audio_parameters='%s', ", stream_config.audio_parameters.value().c_str()).c_str():"",
            stream_config.audio_timebase,
            stream_config.id.c_str()
        );
    }

    return query;
}

r_camera r_devices::_create_camera(const map<string, r_nullable<string>>& row) const
{
    r_camera camera;
    camera.id = row.at("id").value();
    camera.ipv4 = row.at("ipv4").value();
    camera.rtsp_url = row.at("rtsp_url").value();
    camera.rtsp_username = row.at("rtsp_username");
    camera.rtsp_password = row.at("rtsp_password");
    camera.video_codec = row.at("video_codec").value();
    camera.video_parameters = row.at("video_parameters");
    camera.video_timebase = s_to_int(row.at("video_timebase").value());
    camera.audio_codec = row.at("audio_codec").value();
    camera.audio_parameters = row.at("audio_parameters");
    camera.audio_timebase = s_to_int(row.at("audio_timebase").value());
    camera.state = row.at("state").value();
    return camera;
}

r_devices_cmd_result r_devices::_insert_or_update_devices(const r_db::r_sqlite_conn& conn, const std::vector<r_stream_config>& stream_configs) const
{
    r_sqlite_transaction(conn, [&](const r_sqlite_conn& conn){
        for(auto& sc : stream_configs)
            conn.exec(_create_insert_or_update_query(conn, sc));
    });

    return r_devices_cmd_result();
}

r_devices_cmd_result r_devices::_get_all_cameras(const r_sqlite_conn& conn) const
{
    r_devices_cmd_result result;

    r_sqlite_transaction(conn, [&](const r_sqlite_conn& conn){
        auto cameras = conn.exec("SELECT * FROM cameras;");
        for(auto& c : cameras)
            result.cameras.push_back(_create_camera(c));
    });

    return result;
}

r_devices_cmd_result r_devices::_get_assigned_cameras(const r_sqlite_conn& conn) const
{
    r_devices_cmd_result result;

    r_sqlite_transaction(conn, [&](const r_sqlite_conn& conn){
        auto cameras = conn.exec("SELECT * FROM cameras WHERE state=\"assigned\";");
        for(auto& c : cameras)
            result.cameras.push_back(_create_camera(c));
    });

    return result;
}

r_devices_cmd_result r_devices::_save_camera(const r_sqlite_conn& conn, const r_camera& camera) const
{
    r_devices_cmd_result result;

    auto query = r_string_utils::format(
            "UPDATE cameras SET "
                "ipv4='%s', "
                "rtsp_url='%s', "
                "%s"
                "%s"
                "video_codec='%s', "
                "%s"
                "video_timebase=%d, "
                "audio_codec='%s', "
                "%s"
                "audio_timebase=%d, "
                "state='%s' "
            "WHERE id='%s';",
            camera.ipv4.c_str(),
            camera.rtsp_url.c_str(),
            (!camera.rtsp_username.is_null())?r_string_utils::format("rtsp_username='%s', ", camera.rtsp_username.value().c_str()).c_str():"",
            (!camera.rtsp_password.is_null())?r_string_utils::format("rtsp_password='%s', ", camera.rtsp_password.value().c_str()).c_str():"",
            camera.video_codec.c_str(),
            (!camera.video_parameters.is_null())?r_string_utils::format("video_parameters='%s', ", camera.video_parameters.value().c_str()).c_str():"",
            camera.video_timebase,
            camera.audio_codec.c_str(),
            (!camera.audio_parameters.is_null())?r_string_utils::format("audio_parameters='%s', ", camera.audio_parameters.value().c_str()).c_str():"",
            camera.audio_timebase,
            camera.state.c_str(),
            camera.id.c_str()
        );

    r_sqlite_transaction(conn, [&](const r_sqlite_conn& conn){
        conn.exec(query);
    });

    return result;
}
