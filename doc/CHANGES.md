2017.05.16

* Start conversion to c++17
    - this means std::optional, std::filesystem (complete in llvm 3.9), std::variant etc instead of boost versions
    - see http://libcxx.llvm.org/ts1z_status.html

    - removed stdext::make_unique() which is in libc++ now
