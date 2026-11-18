#ifndef ESR_UTILS_RESULT_H
#define ESR_UTILS_RESULT_H

#include <cassert>
#include <optional>
#include <cstdint>

namespace esr {
    enum class GenericError {
        Error,
    };

#define CHECK_RESULT_RETURN(res) \
    if (!res.is_ok()) {          \
        return {res.error()};    \
    }

#define DEBUG_PRINT(message) std::cout << message << "\n" << __FILE__ << ":" << __LINE__ << std::endl;

    template<typename C, typename E>
    class Result {
    public:
        Result(const C &content) {
            content_ = std::make_optional(content);
        }

        Result(const E &error) {
            error_ = std::make_optional(error);
        }

        bool is_ok() const {
            return content_.has_value();
        }

        const C &unwrap() const {
            assert(content_.has_value() && "Expected valid result!");
            return content_.value();
        }

        E error() const {
            assert(error_.has_value() && "Expected invalid result!");
            return error_.value();
        }

    private:
        std::optional<C> content_;
        std::optional<E> error_;
    };

    struct OrderedId {
        uint64_t v_;

        bool operator<(const OrderedId &b) const {
            return v_ < b.v_;
        }

        bool operator==(const OrderedId &b) const {
            return v_ == b.v_;
        }
    };
} // namespace esr

#endif // ESR_UTILS_RESULT_H
