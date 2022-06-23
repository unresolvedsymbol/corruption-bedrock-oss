#pragma once

#include <functional>
#include <string>

namespace SDK {
    struct UWPString {
        union {
            char* pdata;
            char data[8];
        };
        size_t size;
        //size_t alloc;
    public:

        UWPString() = default;
        UWPString(const UWPString&) = delete;
        explicit UWPString(std::string_view str);
        ~UWPString();

        UWPString& operator=(std::string_view str);

        operator std::string_view() const;
        bool operator==(std::string_view) const;

        inline const char* c_str() const {
            // leijurv resigned
            return this->size >= 16 ? this->pdata : this->data;
        }

        inline std::string str() const {
            return std::string(this->c_str(), this->size);
        }
    };

    template<typename T>
    struct UWPList {
        T* m_begin;
        T* m_end;
        T* alloc;

        T& operator[](size_t i) noexcept {
            return this->m_begin[i];
        }

        [[nodiscard]] size_t size() const  {
            return m_end - m_begin;
        }

        T* begin() {
            return m_begin;
        }

        T* end() {
            return m_end;
        }

        template<typename Fn>
        void forEach(Fn consumer) {
            for (T& iter : *this) {
                consumer(iter);
            }
        }
    };
}