// dbg.hpp
#ifndef DBG_HPP
#define DBG_HPP

#include <iostream>
#include <syncstream>

#define DBG(x)                                                                     \
    do {                                                                           \
        std::osyncstream(std::cerr)                                                \
            << __FILE__ << ':' << __LINE__ << " | " << #x << " = " << (x) << '\n'; \
    } while (0)

#endif // DBG_HPP
