


class buffer_pool
{
    uint8_t* data_;
    std::deque<asio::mutable_buffer> buffers_;
    std::mutex mutex_;

    // to improve locality get and return buffers at the same end

    asio::mutable_buffer get_write_buffer();
    asio::const_buffer get_read_buffer(int index);

    void return_buffer(asio::mutable_buffer& );
    void return_buffer(asio::const_buffer& );

    int handoff(asio::mutable_buffer buf); // read handler writes into provided mutable buf and
    // hands it off as an index for further use in get_read_buffer() by client.
};
