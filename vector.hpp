#ifndef VECTOR_HPP
#define VECTOR_HPP


#include <cassert>
#include <iostream>
#include <new>


enum class VectorExceptions
{
    SWAP_VECTORS_WITH_MISMATCHING_CAPACITIES,
    INVALID_INDEX,
    DATA_WAS_NOT_INITIALIZED,
    TOO_MUCH_MEMORY_DEMANDED,
    FAILED_RESIZE,
    FAILED_RESERVE,
    NULL_POP
};


char *UNINIT_PTR = (char *) 0xDEADBEEF;
char *DESTR_PTR  = (char *) 0xBAADF00D;

const size_t POISONED_SIZE_T = 0xAB0BAC0C;

const size_t VECTOR_DEFAULT_CAPACITY     = 4;
const size_t DEFAULT_CAPACITY_MULTIPLIER = 2;

const size_t MAX_CAPACITY = 1024;


template <class Type>
class Vector
{
private:

    size_t capacity_  = VECTOR_DEFAULT_CAPACITY;
    size_t size_      = 0;
    
    char *data_ = UNINIT_PTR;


    char *vector_realloc(char *data, size_t new_capacity, size_t new_size, VectorExceptions code, const Type &value)
    {
        assert(data != nullptr);

        char *new_data = UNINIT_PTR;
        try
        {
            new_data = new char[new_capacity * sizeof(Type)];
        }
        catch (std::bad_alloc)
        {
            std::cout << "ERROR: not enough memory" << std::endl;

            throw code;
        }

        for (size_t index = 0; index < size_; ++index)
        {
            size_t converted_index = index * sizeof(Type);
            new (new_data + converted_index) Type{data_[converted_index]};

            data_[converted_index].~Type();
        }

        for (size_t index = size_; index < new_size; ++index)
        {
            new (new_data + index * sizeof(Type)) Type{value};
        }

        return new_data;
    }

public:
    
    Vector(const size_t reserved_size)
      : capacity_(VECTOR_DEFAULT_CAPACITY),
        size_    (reserved_size),
        data_    (UNINIT_PTR)
    {
        while (capacity_ < reserved_size)
        {
            capacity_ *= DEFAULT_CAPACITY_MULTIPLIER;
        }

        try
        {
            data_ = new char[capacity_ * sizeof(Type)];
        }
        catch (std::bad_alloc)
        {
            std::cout << "ERROR: vector data was not initialized" << std::endl;

            capacity_ = POISONED_SIZE_T;
            size_     = POISONED_SIZE_T;
            data_     = DESTR_PTR;

            throw VectorExceptions::DATA_WAS_NOT_INITIALIZED;
        }
        
        for (size_t index = 0; index < reserved_size; ++index)
        {
            new (data_ + index * sizeof(Type)) Type{};
        }
    }

    Vector()
    {
        Vector(0);
    }

    
    bool empty() const
    {
        return size_ == 0;
    }

    size_t size() const
    {
        return size_;
    }

    size_t max_size() const
    {
        return MAX_CAPACITY;
    }

    size_t capacity() const
    {
        return capacity_;
    }

    Type &operator [](const size_t index)
    {
        return *((Type *) (data_ + index * sizeof(Type)));
    }

    Type &at(const size_t index) const
    {
        if (index < capacity_)
        {
            return data_[index * sizeof(Type)];
        }

        throw VectorExceptions::INVALID_INDEX;
    }

    Type &front() const
    {
        return data_[0];
    }

    Type &back() const
    {
        return data_[(size_ - 1) * sizeof(Type)];
    }

    Type *data() const
    {
        return data_;
    }

    int reserve(size_t new_capacity)
    {
        if (new_capacity <= capacity_)
        {
            return 0;
        }

        char *new_data = vector_realloc(data_, new_capacity, size_, VectorExceptions::FAILED_RESERVE);

        delete [] data_;

        data_     = new_data;
        capacity_ = new_capacity;

        return 0;
    }

    int resize(size_t new_size, const Type &value)
    {
        if (new_size > MAX_CAPACITY)
        {
            std::cout << "ERROR: too much memory required" << std::endl;

            throw VectorExceptions::TOO_MUCH_MEMORY_DEMANDED;
        }

        if (new_size < size_)                                               // new size is smaller than previous
        {
            for (size_t index = new_size; index < size_; ++index)
            {
                data_[index * sizeof(Type)].~Type();
            }

            size_ = new_size;

            return 0;
        }

        if (new_size <= capacity_)                                          // new size is smaller or equal to previous but smaller or equal to capacity
        {
            for (size_t index = size_; index < new_size; ++index)
            {
                new (data_ + index * sizeof(Type)) Type{};
            }

            size_ = new_size;

            return 0;
        }

        size_t new_capacity = capacity_ * DEFAULT_CAPACITY_MULTIPLIER;      // new size is bigger than capacity
        while (new_capacity < new_size)
        {
            new_capacity *= DEFAULT_CAPACITY_MULTIPLIER;
        }

        char *new_data = vector_realloc(data_, new_capacity, new_size, VectorExceptions::FAILED_RESIZE, value);

        delete [] data_;

        data_     = new_data;
        capacity_ = new_capacity;
        size_     = new_size;

        return 0;
    }

    int shrink_to_fit()
    {
        if (capacity_ == size_)
        {
            return 0;
        }

        char *new_data = vector_realloc(data_, size_, size_, VectorExceptions::FAILED_RESIZE, Type{0.0});

        delete [] data_;

        data_     = new_data;
        capacity_ = size_;

        return 0;
    }

    void push_back(const Type &value)
    {
        if (capacity_ == size_)
        {
            resize(size_ * DEFAULT_CAPACITY_MULTIPLIER, Type{0.0});
        }

        data_[size_ * sizeof(Type)] = value;

        ++size_;
    }

    Type &pop_back()
    {
        if (size_ == 0)
        {
            throw VectorExceptions::NULL_POP;
        }

        --size_;

        size_t converted_index = size_ * sizeof(Type);
        Type result = data_[converted_index];
        data_[converted_index].~Type();

        return result;
    }
};

#endif