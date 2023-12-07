#include "commons.hpp"

bool writeComms(int sd, char* buff, size_t len) {
    int retry = 0;
    ssize_t sent = 0, total = 0;
    while (retry < 10) {
        /// possible errors include:
        /// EAGAIN or EWOULDBLOCK or EINTR -> recoverable
        /// anything else -> unrecoverable
        sent = write(sd, buff + total, len - total);
        if (sent == -1) {
            if (errno == EINTR || errno == EWOULDBLOCK || errno == EINTR) {
                std::this_thread::sleep_for(std::chrono::seconds(1));
                retry++;
                continue;
            }
            else {
                return false;
            }
        }
        else if (total == len || sent == 0) {
            // finished reading properly
            return true;
        }
        total += sent;
        // this should be impossible
        if (total > len) {
            // disaster, program is oficially UB
            // no real point trying to catch this, a buffer has
            // already overflown
            throw std::logic_error("Wrote more than the total length");
        }
    }
    return false;
}

bool readComms(int sd, char* buff, size_t len) {
    int retry = 0;
    ssize_t received = 0, total = 0;
    while (retry < 10) {
        /// possible errors include:
        /// EAGAIN or EWOULDBLOCK or EINTR -> recoverable
        /// anything else -> unrecoverable
        received = read(sd, buff + total, len - total);
        if (received == -1) {
            if (errno == EINTR || errno == EWOULDBLOCK || errno == EINTR) {
                std::this_thread::sleep_for(std::chrono::seconds(1));
                retry++;
                continue;
            }
            else {
                return false;
            }
        }
        else if (total == len || received == 0) {
            // finished writing properly
            return true;
        }
        total += received;
        // this should be impossible
        if (total > len) {
            // disaster, program is oficially UB
            // no real point trying to catch this, a buffer has
            // already overflown
            throw std::logic_error("Read more than the total length");
        }
    }
    return false;
}

bool Communication::read(int sd, void* buff, size_t len) {
    return readComms(sd, static_cast<char*>(buff), len);
}

bool Communication::write(int sd, void* buff, size_t len) {
    return writeComms(sd, static_cast<char*>(buff), len);
}