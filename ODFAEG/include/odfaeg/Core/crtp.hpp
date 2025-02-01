#pragma once

namespace odfaeg {
    template <typename T, template <typename> typename crtp_T>
    struct crtp {
        constexpr auto underlying() -> T & { return static_cast<T &>(*this); }
        constexpr auto underlying() const -> const T & { return static_cast<const T &>(*this); }
    };

    struct NoCopy {
        NoCopy() = default;

        constexpr NoCopy(const NoCopy &) = delete;
        constexpr NoCopy &operator=(const NoCopy &) = delete;
    };

    struct NoMove {
        NoMove() = default;

        constexpr NoMove(NoMove &&) noexcept = delete;
        constexpr NoMove &operator=(NoMove &&) noexcept = delete;
    };
}  // namespace odfaeg
