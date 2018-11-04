
#include "r_vss_client/r_query_generator.h"
#include "r_utils/r_string_utils.h"

using namespace r_vss_client;
using namespace r_utils;
using namespace std;
using namespace std::chrono;

r_query_generator::r_query_generator(const string& dataSourceID,
                                     const string& type,
                                     uint64_t requestSize,
                                     const system_clock::time_point& start,
                                     const system_clock::time_point& end) :
    _dataSourceID(dataSourceID),
    _start(start),
    _end(end),
    _next(),
    _type(type),
    _chunkMillis(requestSize),
    _first(true)
{
    auto then = _start + milliseconds(_chunkMillis);
    _next = (then < _end)?then:_end;
}

r_query_generator::~r_query_generator() noexcept
{
}

r_nullable<string> r_query_generator::next()
{
    r_nullable<string> result;

    if(_start == _end)
        return result;

    string url = r_string_utils::format("/query?data_source_id=%s&type=%s&%s&start_time=%s&end_time=%s",
                                  _dataSourceID.c_str(),
                                  _type.c_str(),
                                  (_first)?"previous_playable=true":"previous_playable=false",
                                  r_time::tp_to_iso_8601(_start, false).c_str(),
                                  r_time::tp_to_iso_8601(_next, false).c_str());

    _first = false;

    _start = _next;

    auto then = _start + milliseconds(_chunkMillis);
    _next = (then < _end)?then:_end;

    result.set_value(url);

    return result;
}
