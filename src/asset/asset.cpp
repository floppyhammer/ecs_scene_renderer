#include "asset.h"

namespace esr {

Result<std::vector<uint8_t>, GenericError> load_file_as_bytes(const std::string &file_path) {
    FILE *file = fopen(file_path.c_str(), "rb");
    if (file == nullptr) {
        return {GenericError::Error};
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    std::vector<uint8_t> bytes(length);
    fseek(file, 0, SEEK_SET);
    fread(bytes.data(), 1, length, file);
    fclose(file);

    return {bytes};
}

} // namespace esr
