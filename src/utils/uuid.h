#ifndef ESR_UTILS_UUID_H
#define ESR_UTILS_UUID_H

#define NOMINMAX
#define UUID_SYSTEM_GENERATOR
#include <uuid.h>

#include <cassert>

namespace esr {

inline std::string string_to_hex(const std::string& input) {
    static const char hex_digits[] = "0123456789ABCDEF";

    std::string output;
    output.reserve(input.length() * 2);
    for (unsigned char c : input) {
        output.push_back(hex_digits[c >> 4]);
        output.push_back(hex_digits[c & 15]);
    }
    return output;
}

class Uuid {
public:
    /// Default is nil.
    Uuid() = default;

    Uuid(const std::string& str) {
        uuids::uuid_name_generator gen(uuids::uuid::from_string("47183823-2574-4bfd-b411-99ed177d3e43").value());
        const uuids::uuid id = gen(str);
        value = id;
    }

    static Uuid from_random() {
        Uuid uuid;

        auto id = uuids::uuid_system_generator{}();
        assert(!id.is_nil());
        assert(id.version() == uuids::uuid_version::random_number_based);
        assert(id.variant() == uuids::uuid_variant::rfc);

        uuid.value = id;

        return uuid;
    }

    bool operator<(const Uuid& rhs) const {
        return value < rhs.value;
    }

    bool operator==(const Uuid& rhs) const {
        return value == rhs.value;
    }

    friend std::ostream& operator<<(std::ostream& os, const Uuid& id) {
        os << id.value;
        return os;
    }

private:
    uuids::uuid value;
};

} // namespace esr

#endif // ESR_UTILS_UUID_H
