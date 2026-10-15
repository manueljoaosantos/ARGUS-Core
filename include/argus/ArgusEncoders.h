#pragma once
namespace ArgusEncoders {
    bool begin();
    void reset();
    long rightCount();
    long leftCount();
    void readCounts(long &right, long &left);
}