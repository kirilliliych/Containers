#ifndef VECTOR_HPP
#define VECTOR_HPP


#include <cassert>
#include <iostream>
#include <new>
#include "location.hpp"


//---------------------------Defines section---------------------------------------                                                
//---------------------Exception warranties section--------------------------------
#define STRICT_EXCEPTION_WARRANTY


#ifdef STRICT_EXCEPTION_WARRANTY
#define IF_STRICT_EXCEPTION_WARRANTY(code) code
#else
#define IF_STRICT_EXCEPTION_WARRANTY(code) 
#endif

#ifdef BASE_EXCEPTION_WARRANTY
#define IF_BASE_EXCEPTION_WARRANTY(code) code
#else
#define IF_BASE_EXCEPTION_WARRANTY(code) 
#endif
//---------------------------------------------------------------------------------
#define TRY_CATCH_BLOCK_STRICT_EXCEPTION_WARRANTY(try_section, catch_section)  IF_STRICT_EXCEPTION_WARRANTY(Vector saved_copy = *this;)  \
                                                                               try                                                       \
                                                                               {                                                         \
                                                                                    try_section                                          \
                                                                               }                                                         \
                                                                               catch (...)                                               \
                                                                               {                                                         \
                                                                                    catch_section                                        \
                                                                                                                                         \
                                                                                    throw;                                               \
                                                                               }                                                         \
                                                                               IF_STRICT_EXCEPTION_WARRANTY(*this = saved_copy;)         \
                                                                               IF_BASE_EXCEPTION_WARRANTY(~Vector();)
//---------------------------Const section-----------------------------------------
const char *UNINIT_PTR  = reinterpret_cast<const char *> (0xDEADBEEF);
const char *DESTR_PTR   = reinterpret_cast<const char *> (0xBAADF00D);
const char *INVALID_PTR = reinterpret_cast<const char *> (0xDEADDEAD);
const size_t POISONED_SIZE_T = 0xAB0BAC0C;

const size_t DEFAULT_CAPACITY_MULTIPLIER = 2;
const size_t VECTOR_MAX_CAPACITY         = 1024;


//---------------------------Class Vector------------------------------------------
template <class Type>
class Vector
{
public:
//--------------------Constructors, destructors and =------------------------------
    Vector()
      : capacity_(0),
        size_    (0),
        data_    (const_cast<char *> (UNINIT_PTR))
    {}

    Vector(const size_t reserved_size, const Type &value = Type())
      : Vector()
    {
        if (reserved_size != 0)
        {
            capacity_ = calculate_enough_capacity(reserved_size);
            try
            {
                data_ = new char[capacity_ * sizeof(Type)];
            }
            catch (...)
            {
                std::cerr << "ERROR: vector data was not allocated" << std::endl;
                destroy_fields();

                throw;
            }
            
            init_elements(0, reserved_size, value);

            size_ = reserved_size;
        }
    }

    ~Vector()
    {
        destroy_existing_elems(0, size_);
        
        if ((data_is_valid()) && (data_ != const_cast<char *> (UNINIT_PTR)))
        {
            delete [] data_;
        }

        destroy_fields();
    }

    Vector (const Vector &other)
      : capacity_(other.capacity_),
        size_    (other.size_)
    {
        try
        {
            data_ = new char[capacity_ * sizeof(Type)];
        }
        catch (...)
        {
            std::cerr << "ERROR: vector data was not allocated" << std::endl;
            destroy_fields();

            throw;
        }

        copy_data_to_uninit_place(data_, other.data_, other.size_);
    }

    Vector &operator =(Vector other)
    {
        this->swap(other);

        return *this;
    }

//----------------------------------Dump-------------------------------------------

    void dump(void (*dump_elem)(const Type &value), size_t from = 0, size_t to = VECTOR_MAX_CAPACITY + 1)
    {   
        if (to == VECTOR_MAX_CAPACITY + 1)
        {
            to = capacity_;
        }

        std::cout << "Vector[" << this << "]" << std::endl;
        std::cout << "capacity_: " << capacity_ << std::endl;
        std::cout << "size_: " << size_ << std::endl << std::endl;

        for (; from < to; ++from)
        {
            std::cout << "vector[" << from << "] = ";
            dump_elem(reinterpret_cast<const Type &> (data_[from * sizeof(Type)]));
            std::cout << "\t\t\t&vector[" << from << "] = " << reinterpret_cast<void *> (&data_[from * sizeof(Type)]) << std::endl;
        }

        std::cout << std::endl;
    }
//------------------------------Verificator----------------------------------------
    void verificator()
    {
        assert(size_ <= capacity_);
        assert(capacity_ <= VECTOR_MAX_CAPACITY);
        assert(data != nullptr);
        assert(data != INVALID_PTR);
    }

//---------------------------Size and capacity-------------------------------------

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
        return VECTOR_MAX_CAPACITY;
    }

    size_t capacity() const
    {
        return capacity_;
    }

    void reserve(size_t reserved_size)
    {
        if (reserved_size <= capacity_)
        {
            return;
        }

        char *new_data = const_cast<char *> (UNINIT_PTR);
        try
        {
            new_data = vector_realloc(reserved_size);
        }
        catch (...)
        {
            std::cerr << "ERROR: reserving memory failed" << std::endl; 

            throw;
        }

        if ((data_is_valid()) && (data_ != const_cast<char *> (UNINIT_PTR)))
        {
            delete [] data_;
        }

        data_     = new_data;
        capacity_ = reserved_size;
    }

    void shrink_to_fit()
    {
        if (capacity_ == size_)
        {
            return;
        }

        char *new_data = const_cast<char *> (UNINIT_PTR);
        try
        {
            new_data = vector_realloc(size_);
        }
        catch (...)
        {
            std::cerr << "ERROR: shrink_to_fit failed" << std::endl;

            throw;
        }
        copy_data_to_uninit_place(new_data, data_, size_);
        destroy_existing_elems(0, size_);

        if ((data_is_valid()) && (data_ != const_cast<char *> (UNINIT_PTR)))
        {
            delete [] data_;
        }

        data_     = new_data;
        capacity_ = size_;
    }

//-----------------------------Operating elements----------------------------------

    const Type &operator [](const size_t index) const
    {
        return const_cast<const Type &>(const_cast<Vector<Type> *>(this)->operator[](index));
    }

    Type &operator [](const size_t index)
    {
        assert(index < capacity_);

        return reinterpret_cast<Type &> (data_[index * sizeof(Type)]);
    }

    const Type &at(const size_t index) const
    {
       return const_cast<const Type &>(const_cast<Vector<Type> *>(this)->at(index));
    }

    Type &at(const size_t index)
    {
        if (index < capacity_)
        {
            return this->operator[](index);
        }

        std::cerr << "ERROR: attempt to get value out of bounds" << std::endl;

        throw std::out_of_range("ERROR: attempt to get value out of bounds");
    }

    const Type &front() const
    {
        return const_cast<const Type &>(const_cast<Vector<Type> *>(this)->front());
    }

    Type &front()
    {
        return reinterpret_cast<Type &> (data_[0]);
    }

    const Type &back() const
    {
        return const_cast<const Type &>(const_cast<Vector<Type> *>(this)->back());
    }

    Type &back()
    {
        return reinterpret_cast<Type &> (data_[(size_ - 1) * sizeof(Type)]);
    }

    const Type *data() const
    {
        return const_cast<const Type &>(const_cast<Vector<Type> *>(this)->data());
    }

    Type *data()
    {
        return reinterpret_cast<Type *> (data_);
    }

//---------------------------Modifiers---------------------------------------------

    void clear()
    {
        destroy_existing_elems(0, size_);

        size_ = 0;
    }

    Type &insert(size_t index, const Type &value)
    {
        if ((index > capacity_) || ((index == capacity_) && (size_ != capacity_)))
        {
            std::cerr << "ERROR: attempt to insert out of bounds" << std::endl;

            throw std::out_of_range("ERROR: attempt to insert out of bounds");
        }
        if (index > size_)
        {
            index = size_;
        }

        TRY_CATCH_BLOCK_STRICT_EXCEPTION_WARRANTY
        (
            reserve(size_ + 1);
            init_elements(size_, size_ + 1);
            copy_data(data_ + (index + 1) * sizeof(Type), data_ + index * sizeof(Type), size_ - index);

            ++size_;

            reinterpret_cast<Type &> (data_[index * sizeof(Type)]) = value;
        ,
    
            std::cerr << "ERROR: insertion failed" << std::endl;
        )

        return reinterpret_cast<Type &> (data_[index * sizeof(Type)]);
    }

    Type &erase(size_t index)
    {
        if ((index > capacity_) || ((index == capacity_) && (size_ != capacity_)))
        {
            std::cerr << "ERROR: attempt to erase out of bounds" << std::endl;

            throw std::out_of_range("ERROR: attempt to erase out of bounds");
        }

        if (index >= size_)
        {
            return reinterpret_cast<Type &> (data_[index * sizeof(Type)]);
        }

        TRY_CATCH_BLOCK_STRICT_EXCEPTION_WARRANTY
        (
            copy_data(data_ + index * sizeof(Type), data_ + (index + 1) * sizeof(Type), size_ - index);
            std::cout << "size_: " << size_ << std::endl;
            destroy_existing_elems(size_ - 1, size_);

            --size_;
        ,

            std::cerr << "ERROR: erase failed" << std::endl;  
        )

        return reinterpret_cast<Type &> (data_[(index + 1) * sizeof(Type)]);
    }

    void push_back(const Type &value)
    {
        TRY_CATCH_BLOCK_STRICT_EXCEPTION_WARRANTY
        (
            insert(size_, value);
        ,
            std::cerr << "ERROR: push_back failed" << std::endl;
        )
    }

    void pop_back()
    {
        if (size_ == 0)
        {
            return;
        }

        TRY_CATCH_BLOCK_STRICT_EXCEPTION_WARRANTY
        (
            erase(size_ - 1);
        ,
            std::cerr << "ERROR: pop_back failed" << std::endl;
        )
    }

    void resize(size_t new_size, const Type &value = Type())
    {
        if (new_size > VECTOR_MAX_CAPACITY)
        {
            throw std::bad_alloc("ERROR: resizing requires too much memory");
        }

        if (new_size <= size_)                                                      // new size is smaller or equal to previous
        {
            destroy_existing_elems(new_size, size_);

            size_ = new_size;

            return;
        }

        if (new_size <= capacity_)                                                  // new size is bigger than previous but smaller or equal to capacity
        {
            init_elements(size_, new_size, value);

            size_ = new_size;

            return;
        }

        size_t new_capacity = calculate_enough_capacity(capacity_, new_size);       // new size is bigger than capacity
        char *new_data = const_cast<char *> (UNINIT_PTR);
        try
        {
            new_data = vector_realloc(new_capacity);
        }
        catch (...)
        {
            std::cerr << "ERROR: resizing failed" << std::endl; 

            throw;
        }
        init_elements(size_, new_size, value);

        if ((data_is_valid()) && (data_ != UNINIT_PTR))
        {
            delete [] data_;
        }

        data_     = new_data;
        capacity_ = new_capacity;
        size_     = new_size;
    }

    void swap(Vector &other)
    {
        std::swap(capacity_, other.capacity_);
        std::swap(size_, other.size_);
        std::swap(data_, other.data_);
    }

//operators
private:
//--------------------------Utility functions--------------------------------------

    size_t calculate_enough_capacity(size_t required_size) 
    {
        size_t capacity = 1;

        for (int base = 32; base > 0; base /= 2)
        {
            if (required_size >= (capacity << base))
            {
                capacity <<= base;
            }
        }

        capacity <<= 1;

        return capacity > VECTOR_MAX_CAPACITY ? VECTOR_MAX_CAPACITY : capacity;
    }

    void init_elements(size_t from, size_t to, const Type &value = Type())
    {                                                   
        TRY_CATCH_BLOCK_STRICT_EXCEPTION_WARRANTY
        (
            for (size_t vector_elem_index = from; vector_elem_index < to; ++vector_elem_index)
            {
                new (data_ + vector_elem_index * sizeof(Type)) Type(value);
            }    
        ,                                                         
            std::cerr << "ERROR: sequental initialization failed" << std::endl;
        )                                                                          
    }

    void copy_data_to_uninit_place(char *dest, const char *src, size_t quantity)
    {
        if ((dest == nullptr) || (src == nullptr))
        {
            return;
        }

        for (size_t index = 0; index < quantity; ++index)
        {
            const Type &elem_to_copy = reinterpret_cast<const Type &> (src[index * sizeof(Type)]);
            new (dest + index * sizeof(Type)) Type(elem_to_copy);
        }
    }

    void copy_data(char *dest, const char *src, size_t quantity)
    {
        if ((dest == nullptr) || (src == nullptr))
        {
            return;
        }

        Type *casted_dest      = reinterpret_cast<Type *> (dest);
        const Type *casted_src = reinterpret_cast<const Type *> (src); 

        if (dest == src)
        {
            return;
        }
        
        if (dest < src)
        {
            for (size_t counter = 0; counter < quantity; ++counter)
            {
                *(casted_dest + counter) = *(casted_src + counter);
            }
        }
        else
        {
            for (size_t counter = quantity; counter > 0; --counter)
            {
                *(casted_dest + counter - 1) = *(casted_src + counter - 1);
            }
        }
    }

    char *vector_realloc(size_t new_capacity)
    {
        char *new_data = const_cast<char *> (UNINIT_PTR);
        try
        {
            new_data = new char[new_capacity * sizeof(Type)];
        }
        catch (...)
        {
            std::cout << "ERROR: allocating more memory failed" << std::endl;

            throw;
        }
        copy_data_to_uninit_place(new_data, data_, size_);
        destroy_existing_elems(0, size_);

        return new_data;
    }

    bool data_is_valid()
    {
        return (data_ != DESTR_PTR) && (data_ != INVALID_PTR);
    }

    void destroy_existing_elems(size_t from, size_t to)
    {
        for (size_t index = from; index < to; ++index)
        {
            (reinterpret_cast<Type *> (data_))[index * sizeof(Type)].~Type();
        }
    }

    void destroy_fields()
    {
        capacity_ = POISONED_SIZE_T;
        size_     = POISONED_SIZE_T;
        data_     = const_cast<char *> (DESTR_PTR);
    }

private:
//----------------------------Variables--------------------------------------------

    size_t capacity_  = 0;
    size_t size_      = 0;
    
    char *data_ = const_cast<char *> (UNINIT_PTR);
};


template<class Type>
int vector_cmp(const Vector<Type> &v1, const Vector<Type> &v2)
{
    if (v1.capacity() > v2.capacity())
    {
        return 1;
    }
    if (v1.capacity() < v2.capacity())
    {
        return -1;
    }

    if (v1.size() > v2.size())
    {
        return 1;
    }
    if (v1.size() < v2.size())
    {
        return -1;
    }

    for (long long cur_elem_index = 0; cur_elem_index < v1.size(); ++cur_elem_index)
    {
        const char *v1_data_ptr = v1.data() + cur_elem_index * sizeof(Type) - 1;
        const char *v2_data_ptr = v2.data() + cur_elem_index * sizeof(Type) - 1;

        for (int cur_byte = sizeof(Type); cur_byte > 0; --cur_byte)
        {
            if (*v1_data_ptr > *v2_data_ptr)
            {
                return 1;
            }

            if (*v1_data_ptr < *v2_data_ptr)
            {
                return -1;
            }

            --v1_data_ptr;
            --v2_data_ptr;
        }        
    }

    return 0;
}

template<class Type>
bool operator ==(const Vector<Type> &v1, const Vector<Type> &v2)
{
    return vector_cmp(v1, v2) == 0;
}

template<class Type>
bool operator !=(const Vector<Type> &v1, const Vector<Type> &v2)
{
    return vector_cmp(v1, v2) != 0;
}

template<class Type>
bool operator <(const Vector<Type> &v1, const Vector<Type> &v2)
{
    return vector_cmp(v1, v2) == -1;
}

template<class Type>
bool operator <=(const Vector<Type> &v1, const Vector<Type> &v2)
{
    return vector_cmp(v1, v2) <= 0;
}

template<class Type>
bool operator >(const Vector<Type> &v1, const Vector<Type> &v2)
{
    return vector_cmp(v1, v2) == 1;
}

template<class Type>
bool operator >=(const Vector<Type> &v1, const Vector<Type> &v2)
{
    return vector_cmp(v1, v2) >= 0;
}

#endif
