#ifndef INTEGER_SET_H
#define INTEGER_SET_H

#include <algorithm>
#include <cstddef>
#include <initializer_list>
#include <vector>

class integer_set
{
    std::vector<int> data;
public:
    using value_type = int;
    using size_type = std::size_t;
    using iterator = std::vector<int>::iterator;
    using const_iterator = std::vector<int>::const_iterator;

    integer_set() = default;
    constexpr integer_set(std::initializer_list<int> values);

    bool contains(int value) const;
    bool empty() const noexcept;
    size_type size() const noexcept;

    iterator begin();
    const_iterator begin() const;
    const_iterator cbegin() const;
    iterator end();
    const_iterator end() const;
    const_iterator cend() const;

    const_iterator find(int value) const;

    iterator insert(int value);
    iterator insert(const_iterator hint, int value);
    bool erase(int value);
    template <typename Predicate>
    void erase_if(Predicate pred);

    void minus(const integer_set &other);
	
	template <typename UnaryFunc>
	void for_each(UnaryFunc&& func) const;
};

template <typename Predicate>
void integer_set::erase_if(Predicate pred)
{
    data.erase(std::remove_if(data.begin(), data.end(), pred), data.end());
}

inline constexpr integer_set::integer_set(std::initializer_list<int> values)
    : data(values)
{
    std::sort(data.begin(), data.end());
    data.erase(std::unique(data.begin(), data.end()), data.end());
}

template <typename UnaryFunc>
void integer_set::for_each(UnaryFunc&& func) const
{
    for (const auto& elem : data) {
        std::forward<UnaryFunc>(func)(elem);
    }
}



#if 0
using index_set = integer_set;
#else

#include <bitset>
#include <initializer_list>
#include <cstddef>
#include <utility>
#include <iostream> 

//static inline  std::atomic<size_t> cnt_contains_{0};
//static inline  std::atomic<size_t> cnt_insert_{0};
//static inline  std::atomic<size_t> cnt_erase_{0};
//static inline  std::atomic<size_t> cnt_erase_if_{0};
//static inline  std::atomic<size_t> cnt_operator_index_{0};
//static inline  std::atomic<size_t> cnt_foreach_{0};
//static inline  std::atomic<size_t> cnt_minus_{0};

class index_set
{
    static constexpr int MAX_VALUE = 1023;
    static constexpr std::size_t BITSET_SIZE = static_cast<std::size_t>(MAX_VALUE) + 1;

    std::bitset<BITSET_SIZE> bits_;
    int current_min_;
    int current_max_;

    void update_current_min_after_erase()
    {
        for (int i = current_min_; i <= current_max_; ++i) {
            if (bits_.test(static_cast<std::size_t>(i))) {
                current_min_ = i;
                return;
            }
        }
        current_min_ = MAX_VALUE + 1;
    }

    void update_current_max_after_erase()
    {
        for (int i = current_max_; i >= current_min_; --i) {
            if (bits_.test(static_cast<std::size_t>(i))) {
                current_max_ = i;
                return;
            }
        }
        current_max_ = -1;
    }

public:
    using value_type = int;
    using size_type = std::size_t;

    index_set() noexcept : current_min_(MAX_VALUE + 1), current_max_(-1) {}

    constexpr index_set(std::initializer_list<int> values) noexcept : current_min_(MAX_VALUE + 1), current_max_(-1)
    {
        for (int val : values) {
            if (val >= 0 && val <= MAX_VALUE) {
                bits_.set(static_cast<std::size_t>(val));
                if (val < current_min_) {
                    current_min_ = val;
                }
                if (val > current_max_) {
                    current_max_ = val;
                }
            }
        }
		//std::cout << "\n==========  TOTAL STATS ==========\n"
		//  << " contains:     " << cnt_contains_ << "\n"
		//  << " insert:       " << cnt_insert_ << "\n"
		//  << " erase:        " << cnt_erase_ << "\n"
		//  << " operator[]:   " << cnt_operator_index_ << "\n"
		//  << " for_each:     " << cnt_foreach_ << "\n"
		//  << " minus:        " << cnt_minus_ << "\n"
		//  << " cnt_erase_if_:" << cnt_erase_if_ << "\n"
		//  << "==============================================\n";
    }

    bool contains(int value) const noexcept
    {
		//cnt_contains_++;
        return (value >= 0 && value <= MAX_VALUE) && bits_.test(static_cast<std::size_t>(value));
    }

    bool empty() const noexcept { return current_min_ > current_max_; }
    size_type size() const noexcept { return bits_.count(); }
    bool is_contiguous() const noexcept
    {
        if (empty()) return true;
        return static_cast<size_type>(current_max_ - current_min_ + 1) == size();
    }

    bool insert(int value) noexcept
    {
		//cnt_insert_++;
        if (value < 0 || value > MAX_VALUE) return false;
        const std::size_t pos = static_cast<std::size_t>(value);
        bits_.set(pos);
        if (value < current_min_) {
            current_min_ = value;
        }
        if (value > current_max_) {
            current_max_ = value;
        }
        return true;
    }

    bool erase(int value) noexcept
    {
		//cnt_erase_++;
        if (value < 0 || value > MAX_VALUE) return false;
        const std::size_t pos = static_cast<std::size_t>(value);
        const bool existed = bits_.test(pos);
        bits_.reset(pos);
        if (existed) {
            if (value == current_min_) {
                update_current_min_after_erase();
            }
            if (value == current_max_) {
                update_current_max_after_erase();
            }
        }
        return existed;
    }

    template <typename Predicate>
    void erase_if(Predicate pred)
    {
		//cnt_erase_if_++;
        if (empty()) return;

        std::bitset<BITSET_SIZE> to_erase;

        for (int i = current_min_; i <= current_max_; ++i) {
            if (bits_.test(static_cast<std::size_t>(i)) && pred(i)) {
                to_erase.set(static_cast<std::size_t>(i));
            }
        }

        if (to_erase.none()) return;

        int old_min = current_min_;
        int old_max = current_max_;

        bits_ &= ~to_erase;

        if (bits_.none()) {
            current_min_ = MAX_VALUE + 1;
            current_max_ = -1;
        } else {
            if (!bits_.test(static_cast<std::size_t>(old_min))) {
                update_current_min_after_erase();
            }
            if (!bits_.test(static_cast<std::size_t>(old_max))) {
                update_current_max_after_erase();
            }
        }
    }

    void minus(const index_set& other) noexcept
    {
		//cnt_minus_++;
        bits_ &= ~other.bits_;
        if (!empty()) {
            if (other.contains(current_min_)) {
                update_current_min_after_erase();
            }
            if (other.contains(current_max_)) {
                update_current_max_after_erase();
            }
        }
    }

    template <typename UnaryFunc>
    void for_each(UnaryFunc&& func) const
    {
		//cnt_foreach_++;
        if (empty()) return;
        for (int i = current_min_; i <= current_max_; ++i) {
            if (bits_.test(static_cast<std::size_t>(i))) {
                std::forward<UnaryFunc>(func)(i);
            }
        }
    }

	int operator[](size_type n) const noexcept
	{
		//cnt_operator_index_++;
		if (is_contiguous()) {
			return current_min_ + static_cast<int>(n);
		}

		constexpr size_t BLOCK_BITS = 64;
		const size_t start_block = static_cast<size_t>(current_min_) / BLOCK_BITS;
		const size_t end_block = static_cast<size_t>(current_max_) / BLOCK_BITS;
		size_type remaining = n;

		for (size_t block_idx = start_block; block_idx <= end_block; ++block_idx) {
			uint64_t block_val = 0;
			for (size_t i = 0; i < BLOCK_BITS; ++i) {
				if (bits_.test(block_idx * BLOCK_BITS + i)) {
					block_val |= (1ULL << i);
				}
			}

			const size_t block_pop = std::popcount(block_val);

			if (remaining < block_pop) {
				uint64_t val = block_val;
				size_t bit_pos = 0;
				for (size_t shift = 32; shift > 0; shift >>= 1) {
					const size_t half_pop = std::popcount(val & ((1ULL << shift) - 1));
					if (remaining >= half_pop) {
						remaining -= half_pop;
						val >>= shift;
						bit_pos += shift;
					}
				}
				return static_cast<int>(block_idx * BLOCK_BITS + bit_pos);
			}
			remaining -= block_pop;
		}
		return -1;
	}
};
#endif

#endif // INTEGER_SET_H
