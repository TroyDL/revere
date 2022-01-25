
#include "utils.h"

#include <QStandardPaths>
#include <QDir>
#include <deque>

using namespace std;

string top_dir()
{
    auto top_dir = QStandardPaths::locate(QStandardPaths::DocumentsLocation, "", QStandardPaths::LocateDirectory) + "revere" + QDir::separator() + "revere";
    if(!QDir(top_dir).exists())
        QDir().mkpath(top_dir);
    return top_dir.toStdString();
}

vector<pair<int64_t, int64_t>> find_contiguous_segments(const vector<int64_t>& times)
{
    // Since our input times are key frames and different cameras may have different GOP sizes we need to discover for
    // each camera what the normal space between key frames is. We then multiply this value by 1.5 to find our segment
    // threshold.

    int64_t threshold = 0;
    if(times.size() > 1)
    {
        vector<int64_t> deltas;
        adjacent_difference(begin(times), end(times), back_inserter(deltas));
        threshold = (deltas.size() > 2)?(int64_t)((accumulate(begin(deltas)+1, end(deltas), .0) / deltas.size()) * 1.5):0;
    }

    printf("threshold: %ld\n", threshold);

    deque<int64_t> timesd;
    for (auto t : times) {
        timesd.push_back(t);
    }

    vector<vector<int64_t>> segments;
    vector<int64_t> current;

    // Walk the times front to back appending new times to current as long as they are within the threshold.
    // If a new time is outside the threshold then we have a new segment so push current onto segments and
    // start a new current.

    while(!timesd.empty())
    {
        auto t = timesd.front();
        timesd.pop_front();

        if(current.empty())
            current.push_back(t);
        else
        {
            auto delta = t - current.back();
            if(delta > threshold && !current.empty())
            {
                segments.push_back(current);
                current = {t};
            }
            else current.push_back(t);
        }
    }

    // since we push current when we find a gap bigger than threshold we need to push the last current segment.
    if(!current.empty())
        segments.push_back(current);

    vector<pair<int64_t, int64_t>> output;
    for(auto& s : segments)
        output.push_back(make_pair(s.front(), s.back()));

    return output;
}
