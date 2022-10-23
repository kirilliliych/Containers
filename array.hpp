#ifndef ARRAY_HPP
#define ARRAY_HPP


#include <cassert>
#include <cstring>
#include <iostream>


#define BANNED_COPYING_CONSTRUCTOR
//#define ELEM_TYPE_COMPARISON_OPERATORS

enum class ArrayExceptions
{
    SWAP_ARRAYS_WITH_MISMATCHING_CAPACITIES,
    INVALID_INDEX
};


const size_t MAX_CAPACITY = 1024;


template <class Type, size_t Capacity>
class Array
{
public:

    Array()
      : capacity_(Capacity)
    {
        for (size_t index = 0; index < Capacity; ++index)
        {
            data_[index * sizeof(Type)] = 0;
        }
    }

#ifdef BANNED_COPYING_CONSTRUCTOR
    Array(const Array &that) = delete;
#endif

    ~Array(){};


    Array &operator =(const Array &array)
    {
        for (size_t index = 0; index < array.capacity_; ++index)
        {
            size_t converted_index = index * sizeof(Type);
            data_[converted_index] = array.data_[converted_index];
        }

        capacity_ = array.capacity_;

        return *this;
    }


    size_t size() const
    {
        return capacity_;
    }

    size_t max_size() const
    {
        return MAX_CAPACITY;
    }

    Type &operator [](size_t index)
    {
        return data_[index * sizeof(Type)];
    }

    Type &at(const size_t index) const
    {
        if (index < capacity_)
        {
            return data_[index * sizeof(Type)];
        }

        throw ArrayExceptions::INVALID_INDEX;
    }

    Type *data() const
    {
        return data_;
    }

    void fill(const Type &value)
    {
        for (size_t index = 0; index < capacity_; ++index)
        {
            data_[index * sizeof(Type)] = value;
        }
    }

    void swap(Array &other)
    {
        if (capacity_ == other.capacity_)
        {
#ifdef BANNED_COPYING_CONSTRUCTOR
            for (size_t index = 0; index < capacity_; ++index)
            {
                size_t converted_index = index * sizeof(Type);

                Type temp_elem = data_[converted_index];
                data_[converted_index] = other.data_[converted_index];
                other.data_[converted_index] = temp_elem;
            }

            size_t temp_capacity = capacity_;
            capacity_ = other.capacity_;
            other.capacity_ = temp_capacity;

#else
            Array<Type, Capacity> temp = *this;
            *this = other;
            other = temp; 
#endif
        }
        else
        {
            throw ArrayExceptions::SWAP_ARRAYS_WITH_MISMATCHING_CAPACITIES;   
        }
    }

private:

    size_t capacity_ = Capacity;
    Type data_[Capacity] = {};
};


#ifdef ELEM_TYPE_COMPARISON_OPERATORS
template <class Type, size_t Capacity>
int arr_cmp(const Array<Type, Capacity> &arr1, const Array<Type, Capacity> &arr2)
{
    size_t min = std::min(arr1.max_size(), arr2.max_size());
    for (size_t index = 0; index < min; ++index)
    {
        if (arr1.at(index) > arr2.at(index))
        {
            return 1;
        }

        if (arr1.at(index) < arr2.at(index))
        {
            return -1;
        }
    }

    if (arr1.max_size() > arr2.max_size())
    {
        return 1;
    }

    if (arr1.max_size() < arr2.max_size())
    {
        return -1;
    }

    return 0;
}
#endif

template <class Type, size_t Capacity>
bool operator ==(const Array<Type, Capacity> &arr1, const Array<Type, Capacity> &arr2)
{
#ifdef ELEM_TYPE_COMPARISON_OPERATORS
    return arr_cmp(arr1, arr2) == 0;
#else
    int result = memcmp(arr1.data(), arr2.data(), std::min(arr1.max_size(), arr2.max_size()) * sizeof(Type));
    if ((!result) && (arr1.max_size() == arr2.max_size()))
    {
        return true;
    }

    return false;
#endif
}

template <class Type, size_t Capacity>
bool operator !=(const Array<Type, Capacity> &arr1, const Array<Type, Capacity> &arr2)
{
    return !(arr1 == arr2);
}

template <class Type, size_t Capacity>
bool operator  <(const Array<Type, Capacity> &arr1, const Array<Type, Capacity> &arr2)
{
#ifdef ELEM_TYPE_COMPARISON_OPERATORS
    return arr_cmp(arr1, arr2) == -1;
#else
    int result = memcmp(arr1.data(), arr2.data(), std::min(arr1.max_size(), arr2.max_size()) * sizeof(Type));
    if ((result < 0) || ((result == 0) && (arr1.max_size() < arr2.max_size())))
    {
        return true;
    }

    return false;
#endif
}

template <class Type, size_t Capacity>
bool operator <=(const Array<Type, Capacity> &arr1, const Array<Type, Capacity> &arr2)
{
    return ((arr1 == arr2) || (arr1 < arr2));
}

template <class Type, size_t Capacity>
bool operator >(const Array<Type, Capacity> &arr1, const Array<Type, Capacity> &arr2)
{
#ifdef ELEM_TYPE_COMPARISON_OPERATORS
    return arr_cmp(arr1, arr2) == 1;
#else
    int result = memcmp(arr1.data(), arr2.data(), std::min(arr1.max_size(), arr2.max_size()) * sizeof(Type));
    if ((result > 0) || ((result == 0) && (arr1.max_size() > arr2.max_size())))
    {
        return true;
    }

    return false;
#endif
}

template <class Type, size_t Capacity>
bool operator >=(const Array<Type, Capacity> &arr1, const Array<Type, Capacity> &arr2)
{
    return ((arr1 == arr2) || (arr1 > arr2));
}


#endif
