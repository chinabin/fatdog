#ifndef __FATDOG_NONCOPYABLE_H__
#define __FATDOG_NONCOPYABLE_H__

namespace fatdog {

class Noncopyable {
public:
    Noncopyable() = default;
    ~Noncopyable() = default;
    Noncopyable(const Noncopyable&) = delete;
    Noncopyable& operator=(const Noncopyable&) = delete;
};

}

#endif
