
#include "r_disco/r_devices.h"
#include "r_utils/r_file.h"
#include "r_utils/r_string_utils.h"
#include "r_utils/r_exception.h"
#include "r_utils/r_functional.h"
#include <thread>
#include <algorithm>
#include <iterator>

using namespace r_disco;
using namespace r_db;
using namespace r_utils;
using namespace r_utils::r_string_utils;
using namespace r_utils::r_funky;
using namespace std;

r_devices::r_devices(const string& top_dir) :
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
    _th = thread(&r_devices::_entry_point, this);
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

void r_devices::insert_or_update_devices(const vector<pair<r_stream_config, string>>& stream_configs)
{
    r_devices_cmd cmd;
    cmd.type = INSERT_OR_UPDATE_DEVICES;
    cmd.configs = stream_configs;

    _db_work_q.post(cmd);
}

r_nullable<r_camera> r_devices::get_camera_by_id(const string& id)
{
    r_devices_cmd cmd;
    cmd.type = GET_CAMERA_BY_ID;
    cmd.id = id;

    auto result = _db_work_q.post(cmd).get();

    r_nullable<r_camera> camera;
    if(!result.cameras.empty())
        camera = result.cameras.front();

    return camera;
}

vector<r_camera> r_devices::get_all_cameras()
{
    r_devices_cmd cmd;
    cmd.type = GET_ALL_CAMERAS;

    return _db_work_q.post(cmd).get().cameras;
}

vector<r_camera> r_devices::get_assigned_cameras()
{
    r_devices_cmd cmd;
    cmd.type = GET_ASSIGNED_CAMERAS;

    return _db_work_q.post(cmd).get().cameras;
}

void r_devices::save_camera(const r_camera& camera)
{
    r_devices_cmd cmd;
    cmd.type = SAVE_CAMERA;
    cmd.cameras.push_back(camera);
    _db_work_q.post(cmd).wait();
}

void r_devices::remove_camera(const r_camera& camera)
{
    r_devices_cmd cmd;
    cmd.type = REMOVE_CAMERA;
    cmd.cameras.push_back(camera);
    _db_work_q.post(cmd).wait();
}

void r_devices::assign_camera(r_camera& camera)
{
    camera.state = "assigned";
    save_camera(camera);
}

pair<r_nullable<string>, r_nullable<string>> r_devices::get_credentials(const std::string& id)
{
    r_devices_cmd cmd;
    cmd.type = GET_CREDENTIALS_BY_ID;
    cmd.id = id;
    return _db_work_q.post(cmd).get().credentials;
}

vector<r_camera> r_devices::get_modified_cameras(const vector<r_camera>& cameras)
{
    r_devices_cmd cmd;
    cmd.type = GET_MODIFIED_CAMERAS;
    cmd.cameras = cameras;
    return _db_work_q.post(cmd).get().cameras;
}

vector<r_camera> r_devices::get_assigned_cameras_added(const vector<r_camera>& cameras)
{
    r_devices_cmd cmd;
    cmd.type = GET_ASSIGNED_CAMERAS_ADDED;
    cmd.cameras = cameras;

    return _db_work_q.post(cmd).get().cameras;
}

vector<r_camera> r_devices::get_assigned_cameras_removed(const vector<r_camera>& cameras)
{
    r_devices_cmd cmd;
    cmd.type = GET_ASSIGNED_CAMERAS_REMOVED;
    cmd.cameras = cameras;

    return _db_work_q.post(cmd).get().cameras;
}

void r_devices::_entry_point()
{
    while(_running)
    {
        auto maybe_cmd = _db_work_q.poll();
        if(!maybe_cmd.is_null())
        {
            auto cmd = move(maybe_cmd.take());

            try
            {
                auto conn = _open_or_create_db(_top_dir);

                if(cmd.first.type == INSERT_OR_UPDATE_DEVICES)
                    cmd.second.set_value(_insert_or_update_devices(conn, cmd.first.configs));
                else if(cmd.first.type == GET_CAMERA_BY_ID)
                    cmd.second.set_value(_get_camera_by_id(conn, cmd.first.id));
                else if(cmd.first.type == GET_ALL_CAMERAS)
                    cmd.second.set_value(_get_all_cameras(conn));
                else if(cmd.first.type == GET_ASSIGNED_CAMERAS)
                    cmd.second.set_value(_get_assigned_cameras(conn));
                else if(cmd.first.type == SAVE_CAMERA)
                {
                    if(cmd.first.cameras.empty())
                        R_THROW(("No cameras passed to SAVE_CAMERA."));
                    cmd.second.set_value(_save_camera(conn, cmd.first.cameras.front()));
                }
                else if(cmd.first.type == REMOVE_CAMERA)
                {
                    if(cmd.first.cameras.empty())
                        R_THROW(("No cameras passed to REMOVE_CAMERA."));
                    cmd.second.set_value(_remove_camera(conn, cmd.first.cameras.front()));
                }
                else if(cmd.first.type == GET_MODIFIED_CAMERAS)
                    cmd.second.set_value(_get_modified_cameras(conn, cmd.first.cameras));
                else if(cmd.first.type == GET_ASSIGNED_CAMERAS_ADDED)
                    cmd.second.set_value(_get_assigned_cameras_added(conn, cmd.first.cameras));
                else if(cmd.first.type == GET_ASSIGNED_CAMERAS_REMOVED)
                    cmd.second.set_value(_get_assigned_cameras_removed(conn, cmd.first.cameras));
                else if(cmd.first.type == GET_CREDENTIALS_BY_ID)
                    cmd.second.set_value(_get_credentials(conn, cmd.first.id));
                else R_THROW(("Unknown work q command."));
            }
            catch(...)
            {
                try
                {
                    cmd.second.set_exception(current_exception());
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
            "ipv4 TEXT, "
            "rtsp_url TEXT, "
            "rtsp_username TEXT, "
            "rtsp_password TEXT, "
            "video_codec TEXT, "
            "video_codec_parameters TEXT, "
            "video_timebase INTEGER, "
            "audio_codec TEXT, "
            "audio_codec_parameters TEXT, "
            "audio_timebase INTEGER, "
            "state TEXT, "
            "record_file_path TEXT, "
            "n_record_file_blocks INTEGER, "
            "record_file_block_size INTEGER, "
            "stream_config_hash TEXT"
        ");"
    );
    string record_file_path;
    int n_record_file_blocks {0};
    int record_file_block_size {0};
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

string r_devices::_create_insert_or_update_query(const r_db::r_sqlite_conn& conn, const r_stream_config& stream_config, const std::string& hash) const
{
    // Why not replace into? Because replace into replaces the whole row, and for the update case here we need
    // to make sure we only update the columns we have values for.
    auto result = conn.exec("SELECT * FROM cameras WHERE id='" + stream_config.id + "';");

    string query;

    if(result.empty())
    {
        query = r_string_utils::format(
            "INSERT INTO cameras (id, %s%s%s%s%s%s%s%s%s%sstate, %s%s%sstream_config_hash) "
            "VALUES('%s', %s%s%s%s%s%s%s%s%s%s'discovered', %s%s%s'%s');",

            (!stream_config.ipv4.is_null())?"ipv4, ":"",
            (!stream_config.rtsp_url.is_null())?"rtsp_url, ":"",
            (!stream_config.rtsp_username.is_null())?"rtsp_username, ":"",
            (!stream_config.rtsp_password.is_null())?"rtsp_password, ":"",
            (!stream_config.video_codec.is_null())?"video_codec, ":"",
            (!stream_config.video_codec_parameters.is_null())?"video_codec_parameters, ":"",
            (!stream_config.video_timebase.is_null())?"video_timebase, ":"",
            (!stream_config.audio_codec.is_null())?"audio_codec, ":"",
            (!stream_config.audio_codec_parameters.is_null())?"audio_codec_parameters, ":"",
            (!stream_config.audio_timebase.is_null())?"audio_timebase, ":"",
            (!stream_config.record_file_path.is_null())?"record_file_path, ":"",
            (!stream_config.n_record_file_blocks.is_null())?"n_record_file_blocks, ":"",
            (!stream_config.record_file_block_size.is_null())?"record_file_block_size, ":"",

            stream_config.id.c_str(),
            (!stream_config.ipv4.is_null())?r_string_utils::format("'%s', ", stream_config.ipv4.value().c_str()).c_str():"",
            (!stream_config.rtsp_url.is_null())?r_string_utils::format("'%s', ", stream_config.rtsp_url.value().c_str()).c_str():"",
            (!stream_config.rtsp_username.is_null())?r_string_utils::format("'%s', ", stream_config.rtsp_username.value().c_str()).c_str():"",
            (!stream_config.rtsp_password.is_null())?r_string_utils::format("'%s', ", stream_config.rtsp_password.value().c_str()).c_str():"",
            (!stream_config.video_codec.is_null())?r_string_utils::format("'%s', ", stream_config.video_codec.value().c_str()).c_str():"",
            (!stream_config.video_codec_parameters.is_null())?r_string_utils::format("'%s', ", stream_config.video_codec_parameters.value().c_str()).c_str():"",
            (!stream_config.video_timebase.is_null())?r_string_utils::format("'%d', ", stream_config.video_timebase.value()).c_str():"",
            (!stream_config.audio_codec.is_null())?r_string_utils::format("'%s', ", stream_config.audio_codec.value().c_str()).c_str():"",
            (!stream_config.audio_codec_parameters.is_null())?r_string_utils::format("'%s', ", stream_config.audio_codec_parameters.value().c_str()).c_str():"",
            (!stream_config.audio_timebase.is_null())?r_string_utils::format("%d, ", stream_config.audio_timebase.value()).c_str():"",
            (!stream_config.record_file_path.is_null())?r_string_utils::format("'%s', ", stream_config.record_file_path.value().c_str()).c_str():"",
            (!stream_config.n_record_file_blocks.is_null())?r_string_utils::format("%d, ", stream_config.n_record_file_blocks.value()).c_str():"",
            (!stream_config.record_file_block_size.is_null())?r_string_utils::format("%d, ", stream_config.record_file_block_size.value()).c_str():"",
            hash.c_str()
        );
    }
    else
    {
        query = r_string_utils::format(
            "UPDATE cameras SET "
                "%s"
                "%s"
                "%s"
                "%s"
                "%s"
                "%s"
                "%s"
                "%s"
                "%s"
                "%s"
                "%s"
                "%s"
                "%s"
                "stream_config_hash='%s' "
            "WHERE id='%s';",
            (!stream_config.ipv4.is_null())?r_string_utils::format("ipv4='%s', ", stream_config.ipv4.value().c_str()).c_str():"",
            (!stream_config.rtsp_url.is_null())?r_string_utils::format("rtsp_url='%s', ", stream_config.rtsp_url.value().c_str()).c_str():"",
            (!stream_config.rtsp_username.is_null())?r_string_utils::format("rtsp_username='%s', ", stream_config.rtsp_username.value().c_str()).c_str():"",
            (!stream_config.rtsp_password.is_null())?r_string_utils::format("rtsp_password='%s', ", stream_config.rtsp_password.value().c_str()).c_str():"",
            (!stream_config.video_codec.is_null())?r_string_utils::format("video_codec='%s', ", stream_config.video_codec.value().c_str()).c_str():"",
            (!stream_config.video_codec_parameters.is_null())?r_string_utils::format("video_codec_parameters='%s', ", stream_config.video_codec_parameters.value().c_str()).c_str():"",
            (!stream_config.video_timebase.is_null())?r_string_utils::format("video_timebase='%d', ", stream_config.video_timebase.value()).c_str():"",
            (!stream_config.audio_codec.is_null())?r_string_utils::format("audio_codec='%s', ", stream_config.audio_codec.value().c_str()).c_str():"",
            (!stream_config.audio_codec_parameters.is_null())?r_string_utils::format("audio_codec_parameters='%s', ", stream_config.audio_codec_parameters.value().c_str()).c_str():"",
            (!stream_config.audio_timebase.is_null())?r_string_utils::format("audio_timebase=%d, ", stream_config.audio_timebase.value()).c_str():"",
            (!stream_config.record_file_path.is_null())?r_string_utils::format("record_file_path='%s', ", stream_config.record_file_path.value().c_str()).c_str():"",
            (!stream_config.n_record_file_blocks.is_null())?r_string_utils::format("n_record_file_blocks=%d, ", stream_config.n_record_file_blocks.value()).c_str():"",
            (!stream_config.record_file_block_size.is_null())?r_string_utils::format("record_file_block_size=%d, ", stream_config.record_file_block_size.value()).c_str():"",
            hash.c_str(),
            stream_config.id.c_str()
        );
    }

    return query;
}

r_camera r_devices::_create_camera(const map<string, r_nullable<string>>& row) const
{
    r_camera camera;
    camera.id = row.at("id").value();
    if(!row.at("ipv4").is_null())
        camera.ipv4 = row.at("ipv4").value();
    if(!row.at("rtsp_url").is_null())
        camera.rtsp_url = row.at("rtsp_url").value();
    if(!row.at("rtsp_username").is_null())
        camera.rtsp_username = row.at("rtsp_username").value();
    if(!row.at("rtsp_password").is_null())
        camera.rtsp_password = row.at("rtsp_password").value();
    if(!row.at("video_codec").is_null())
        camera.video_codec = row.at("video_codec").value();
    if(!row.at("video_codec_parameters").is_null())
        camera.video_codec_parameters = row.at("video_codec_parameters").value();
    if(!row.at("video_timebase").is_null())    
        camera.video_timebase = s_to_int(row.at("video_timebase").value());
    if(!row.at("audio_codec").is_null())
        camera.audio_codec = row.at("audio_codec").value();
    if(!row.at("audio_codec_parameters").is_null())
        camera.audio_codec_parameters = row.at("audio_codec_parameters").value();
    if(!row.at("audio_timebase").is_null())
        camera.audio_timebase = s_to_int(row.at("audio_timebase").value());
    camera.state = row.at("state").value();
    if(!row.at("record_file_path").is_null())
        camera.record_file_path = row.at("record_file_path").value();
    if(!row.at("n_record_file_blocks").is_null())
        camera.n_record_file_blocks = s_to_int(row.at("n_record_file_blocks").value());
    if(!row.at("record_file_block_size").is_null())
        camera.record_file_block_size = s_to_int(row.at("record_file_block_size").value());
    camera.stream_config_hash = row.at("stream_config_hash").value();
    return camera;
}

r_devices_cmd_result r_devices::_insert_or_update_devices(const r_db::r_sqlite_conn& conn, const vector<pair<r_stream_config, string>>& stream_configs) const
{
    r_sqlite_transaction(conn, [&](const r_sqlite_conn& conn){
        for(auto& sc : stream_configs)
        {
            auto q = _create_insert_or_update_query(conn, sc.first, sc.second);
            conn.exec(q);
        }
    });

    return r_devices_cmd_result();
}

r_devices_cmd_result r_devices::_get_camera_by_id(const r_sqlite_conn& conn, const string& id) const
{
    r_devices_cmd_result result;
    r_sqlite_transaction(conn, [&](const r_sqlite_conn& conn){
        auto qr = conn.exec("SELECT * FROM cameras WHERE id='" + id + "';");
        if(!qr.empty())
            result.cameras.push_back(_create_camera(qr.front()));
    });
    return result;
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
    r_stream_config sc;
    sc.id = camera.id;
    sc.ipv4 = camera.ipv4;
    sc.rtsp_url = camera.rtsp_url;
    sc.video_codec = camera.video_codec;
    sc.video_codec_parameters = camera.video_codec_parameters;
    sc.video_timebase = camera.video_timebase;
    sc.audio_codec = camera.audio_codec;
    sc.audio_codec_parameters = camera.audio_codec_parameters;
    sc.audio_timebase = camera.audio_timebase;
    sc.record_file_path = camera.record_file_path;
    sc.n_record_file_blocks = camera.n_record_file_blocks;
    sc.record_file_block_size = camera.record_file_block_size;

    auto hash = hash_stream_config(sc);

    r_devices_cmd_result result;

    auto query = r_string_utils::format(
            "REPLACE INTO cameras("
                "id, %s%s%s%s%s%s%s%s%s%sstate, %s%s%sstream_config_hash) "
            "VALUES("
                "'%s', "
                "%s"
                "%s"
                "%s"
                "%s"
                "%s"
                "%s"
                "%s"
                "%s"
                "%s"
                "%s"
                "'%s', "
                "%s"
                "%s"
                "%s"
                "'%s'"
            ");",
            (!camera.ipv4.is_null())?"ipv4, ":"",
            (!camera.rtsp_url.is_null())?"rtsp_url, ":"",
            (!camera.rtsp_username.is_null())?"rtsp_username, ":"",
            (!camera.rtsp_password.is_null())?"rtsp_password, ":"",
            (!camera.video_codec.is_null())?"video_codec, ":"",
            (!camera.video_codec_parameters.is_null())?"video_codec_parameters, ":"",
            (!camera.video_timebase.is_null())?"video_timebase, ":"",
            (!camera.audio_codec.is_null())?"audio_codec, ":"",
            (!camera.audio_codec_parameters.is_null())?"audio_codec_parameters, ":"",
            (!camera.audio_timebase.is_null())?"audio_timebase, ":"",
            (!camera.record_file_path.is_null())?"record_file_path, ":"",
            (!camera.n_record_file_blocks.is_null())?"n_record_file_blocks, ":"",
            (!camera.record_file_block_size.is_null())?"record_file_block_size, ":"",

            camera.id.c_str(),
            (!camera.ipv4.is_null())?r_string_utils::format("'%s', ", camera.ipv4.value().c_str()).c_str():"",
            (!camera.rtsp_url.is_null())?r_string_utils::format("'%s', ", camera.rtsp_url.value().c_str()).c_str():"",
            (!camera.rtsp_username.is_null())?r_string_utils::format("'%s', ", camera.rtsp_username.value().c_str()).c_str():"",
            (!camera.rtsp_password.is_null())?r_string_utils::format("'%s', ", camera.rtsp_password.value().c_str()).c_str():"",
            (!camera.video_codec.is_null())?r_string_utils::format("'%s', ", camera.video_codec.value().c_str()).c_str():"",
            (!camera.video_codec_parameters.is_null())?r_string_utils::format("'%s', ", camera.video_codec_parameters.value().c_str()).c_str():"",
            (!camera.video_timebase.is_null())?r_string_utils::format("'%d', ", camera.video_timebase.value()).c_str():"",
            (!camera.audio_codec.is_null())?r_string_utils::format("'%s', ", camera.audio_codec.value().c_str()).c_str():"",
            (!camera.audio_codec_parameters.is_null())?r_string_utils::format("'%s', ", camera.audio_codec_parameters.value().c_str()).c_str():"",
            (!camera.audio_timebase.is_null())?r_string_utils::format("%d, ", camera.audio_timebase.value()).c_str():"",
            camera.state.c_str(),
            (!camera.record_file_path.is_null())?r_string_utils::format("'%s', ", camera.record_file_path.value().c_str()).c_str():"",
            (!camera.n_record_file_blocks.is_null())?r_string_utils::format("%d, ", camera.n_record_file_blocks.value()).c_str():"",
            (!camera.record_file_block_size.is_null())?r_string_utils::format("%d, ", camera.record_file_block_size.value()).c_str():"",
            hash.c_str()
        );

    r_sqlite_transaction(conn, [&](const r_sqlite_conn& conn){
        conn.exec(query);
    });

    return result;
}

r_devices_cmd_result r_devices::_remove_camera(const r_sqlite_conn& conn, const r_camera& camera) const
{
    r_sqlite_transaction(conn, [&](const r_sqlite_conn& conn){
        conn.exec("DELETE FROM cameras WHERE id='" + camera.id + "';");
    });

    return r_devices_cmd_result();
}

r_devices_cmd_result r_devices::_get_modified_cameras(const r_sqlite_conn& conn, const vector<r_camera>& cameras) const
{
    vector<r_camera> out_cameras;
    r_sqlite_transaction(conn, [&](const r_sqlite_conn& conn){
        for(auto& c : cameras)
        {
            auto query = r_string_utils::format(
                "SELECT * FROM cameras WHERE id=\"%s\" AND stream_config_hash!=\"%s\";",
                c.id.c_str(),
                c.stream_config_hash.c_str()
            );

            auto modified = conn.exec(query);
            if(!modified.empty())
                out_cameras.push_back(_create_camera(modified.front()));
        }
    });

    r_devices_cmd_result result;
    result.cameras = out_cameras;

    return result;
}

r_devices_cmd_result r_devices::_get_assigned_cameras_added(const r_sqlite_conn& conn, const vector<r_camera>& cameras) const
{
    vector<string> input_ids;
    transform(begin(cameras), end(cameras), back_inserter(input_ids),[](const r_camera& c){return c.id;});

    auto qr = conn.exec("SELECT id FROM cameras WHERE state='assigned';");

    vector<string> db_ids;
    transform(begin(qr), end(qr), back_inserter(db_ids), [](const map<string, r_nullable<string>>& r){return r.at("id").value();});

    // set_diff(a, b) - Returns the items in a that are not in b
    auto added_ids = set_diff(db_ids, input_ids);

    r_devices_cmd_result result;

    for(auto added_id : added_ids)
    {
        qr = conn.exec("SELECT * FROM cameras WHERE id='" + added_id + "';");
        if(!qr.empty())
            result.cameras.push_back(_create_camera(qr.front()));
    }

    return result;
}

r_devices_cmd_result r_devices::_get_assigned_cameras_removed(const r_sqlite_conn& conn, const vector<r_camera>& cameras) const
{
    map<string, r_camera> cmap;
    for(auto& c : cameras)
        cmap[c.id] = c;

    vector<string> input_ids;
    transform(begin(cameras), end(cameras), back_inserter(input_ids),[](const r_camera& c){return c.id;});

    auto qr = conn.exec("SELECT id FROM cameras WHERE state='assigned';");

    vector<string> db_ids;
    transform(begin(qr), end(qr), back_inserter(db_ids), [](const map<string, r_nullable<string>>& r){return r.at("id").value();});

    // set_diff(a, b) - Returns the items in a that are not in b
    auto removed_ids = set_diff(input_ids, db_ids);

    r_devices_cmd_result result;
    transform(begin(removed_ids), end(removed_ids), back_inserter(result.cameras),[&cmap](const string& id){return cmap[id];});

    return result;
}

r_devices_cmd_result r_devices::_get_credentials(const r_sqlite_conn& conn, const string& id)
{
    r_devices_cmd_result result;
    r_sqlite_transaction(conn, [&](const r_sqlite_conn& conn){
        auto qr = conn.exec("SELECT rtsp_username, rtsp_password FROM cameras WHERE id='" + id + "';");
        if(!qr.empty())
        {
            auto row = qr.front();

            auto found = row.find("rtsp_username");
            if(found != row.end() && !found->second.is_null())
                result.credentials.first = found->second.value();

            found = row.find("rtsp_password");
            if(found != row.end() && !found->second.is_null())
                result.credentials.second = found->second.value();
        }
    });
    return result;
}
