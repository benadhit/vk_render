#ifndef HAMON_STRONG_ID_H__
#define HAMON_STRONG_ID_H__

#include <type_traits> //for std::is_integral
#include <functional> //for std::hash
#include <cstddef> //for std::size_t

//Forward delcare the class (needed for operator declarations)
template<typename tag, typename T, T sentinel>
class StrongId;

//Forward declare the equality/inequality operators
// We need to do this before the class definition so the class can
// friend them
template<typename tag, typename T, T sentinel>
bool operator==(const StrongId<tag,T,sentinel>& lhs, const StrongId<tag,T,sentinel>& rhs);

template<typename tag, typename T, T sentinel>
bool operator!=(const StrongId<tag,T,sentinel>& lhs, const StrongId<tag,T,sentinel>& rhs);

template<typename tag, typename T, T sentinel>
bool operator<(const StrongId<tag,T,sentinel>& lhs, const StrongId<tag,T,sentinel>& rhs);


//Class template definition with default template parameters
template<typename tag, typename T=int, T sentinel=T(-1)>
class StrongId {
    static_assert(std::is_integral<T>::value, "T must be integral");

    public:
        //Gets the invalid Id
        static constexpr StrongId INVALID() { return StrongId(); }

        //Default to the sentinel value
        constexpr StrongId() : id_(sentinel) {}

        //Only allow explict constructions from a raw Id (no automatic conversions)
        explicit StrongId(T id) noexcept : id_(id) {}

        //Allow some explicit conversion to useful types

        //Allow explicit conversion to bool (e.g. if(id))
        explicit operator bool() const { return *this != INVALID(); }

        //Allow explicit conversion to size_t (e.g. my_vector[size_t(strong_id)])
        explicit operator std::size_t() const { return static_cast<std::size_t>(id_); }


        //To enable hasing Ids
        friend std::hash<StrongId<tag,T,sentinel>>;

        //To enable comparisions between Ids
        // Note that since these are templated functions we provide an empty set of template parameters
        // after the function name (i.e. <>)
        friend bool operator== <>(const StrongId<tag,T,sentinel>& lhs, const StrongId<tag,T,sentinel>& rhs);
        friend bool operator!= <>(const StrongId<tag,T,sentinel>& lhs, const StrongId<tag,T,sentinel>& rhs);
        friend bool operator< <>(const StrongId<tag,T,sentinel>& lhs, const StrongId<tag,T,sentinel>& rhs);
    private:
        T id_;
};

template<typename tag, typename T, T sentinel>
bool operator==(const StrongId<tag,T,sentinel>& lhs, const StrongId<tag,T,sentinel>& rhs) {
    return lhs.id_ == rhs.id_;
}

template<typename tag, typename T, T sentinel>
bool operator!=(const StrongId<tag,T,sentinel>& lhs, const StrongId<tag,T,sentinel>& rhs) {
    return !(lhs == rhs);
}

//Needed for std::map-like containers
template<typename tag, typename T, T sentinel>
bool operator<(const StrongId<tag,T,sentinel>& lhs, const StrongId<tag,T,sentinel>& rhs) {
    return lhs.id_ < rhs.id_;
}

//Specialize std::hash for StrongId's (needed for std::unordered_map-like containers)
namespace std {
    template<typename tag, typename T, T sentinel>
    struct hash<StrongId<tag,T,sentinel>> {
        std::size_t operator()(const StrongId<tag,T,sentinel> k) const noexcept {
            return std::hash<T>()(k.id_); //Hash with the underlying type
        }
    };
} //namespace std
#endif