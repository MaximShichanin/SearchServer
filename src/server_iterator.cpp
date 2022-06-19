#include "server_iterator.h"

ServerIterator::ServerIterator(const ServerIterator& other) : iter(other.iter) {
}
ServerIterator::ServerIterator(map_iterator other) : iter(other) {
}
ServerIterator::ServerIterator(const_map_iterator other) : const_iter(other) {
}
int ServerIterator::operator*() {
    return iter->first;
}
const int ServerIterator::operator*() const {
    return const_iter->first;
}
[[nodiscard]] bool ServerIterator::operator==(const ServerIterator& other) const {
    return iter == other.iter;
}
[[nodiscard]] bool ServerIterator::operator!=(const ServerIterator& other) const {
    return iter != other.iter;
}
ServerIterator& ServerIterator::operator++() {
    ++iter;
    return *this;
}
