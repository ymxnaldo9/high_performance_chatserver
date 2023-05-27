#ifndef BUFFER_H
#define BUFFER_H

#include <string>
#include <vector>
#include <algorithm>
using namespace std;

class Buffer {
public:
    static const size_t kCheapPrepend = 8;
    static const size_t kInitialSize = 1024;
    explicit Buffer(size_t initialSize = kInitialSize)
        :buffer(kCheapPrepend + kInitialSize), ReaderIndex(kCheapPrepend), WriterIndex(kCheapPrepend) {}
    size_t readableBytes() {
        return WriterIndex - ReaderIndex;
    }
    size_t writableBytes() {
        return buffer.size() - WriterIndex;
    }
    size_t prependBytes() {
        return ReaderIndex;
    }
    const char* peek() const {
        return begin() + ReaderIndex;
    }
    void retrieve(size_t len) {  //string <- buffer
        if (len < readableBytes()) {
            ReaderIndex += len;
        }
        else {
            retrieveAll();
        }
    }
    void retrieveAll() {
        ReaderIndex = kCheapPrepend;
        WriterIndex = kCheapPrepend;
    }
    string retrieveAllAsString() {
        return retrieveAsString(readableBytes());
    }
    string retrieveAsString(size_t len) {
        string result(peek(), len);
        retrieve(len);
        return result;
    } 
    void ensureWritableBytes(size_t len) {
        if (writableBytes() < len) {
            makeSpace(len);
        }
    }
    void append(const char* data, size_t len) {
        ensureWritableBytes(len);
        copy(data, data + len, beginWrite());
        WriterIndex += len;
    }
    char* beginWrite() {
        return begin() + WriterIndex;
    }
    const char* beginWrite() const {
        return begin() + WriterIndex;
    }
    ssize_t readFd(int fd, int* saveErrno);
    ssize_t writeFd(int fd, int* saveErrno);
private:
    char* begin() {
        return &(*buffer.begin());
    }
    const char* begin() const {
        return &(*buffer.begin());
    }
    void makeSpace(size_t len) {
        if (writableBytes() + prependBytes() < len + kCheapPrepend) {
            buffer.resize(len + WriterIndex);
        }
        else {
            size_t readable = readableBytes();
            copy(begin() + ReaderIndex, begin() + WriterIndex, begin() + kCheapPrepend);
            ReaderIndex = kCheapPrepend;
            WriterIndex = ReaderIndex + readable;
        }
    }
    vector<char> buffer;
    size_t ReaderIndex;
    size_t WriterIndex;
};

#endif