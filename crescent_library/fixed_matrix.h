#ifndef FIXED_MATRIX_H
#define FIXED_MATRIX_H
#include "sfinae_operators.h"
#include <algorithm>
#include <array>
#include <iterator>
#include <ostream>
#include <stdexcept>

namespace crsc {
	/**
	 * \class fixed_matrix
	 *
	 * \brief A container encapsulating a `std::array` using a row-major configuration to store a fixed size
	 *        matrix style object. The number of elements in every row are equal and the number of elements
	 *        in every column are equal, such that no holes occur in the structure.
	 *
	 * This container is a fixed-size version of the `crsc::dynamic_matrix` structure which uses the same
	 * semantics as a class holding a C-style array using a row-major configuration for the matrix. It 
	 * combines the performance and accessibility of a C-style array with the benefits of a standard container,
	 * such a knowing its own size, supporting assignment and random access iteration.
	 *
	 * Iteration support is via a `std::random_access_iterator` from the `std::array` structure, the order of
	 * iteration uses an "in-order traversal" such that elements of the `fixed_matrix` are iterated through 
	 * by rows from left to right, top to bottom.
	 *
	 * \tparam _Ty The type of the elements.
	 * \tparam _Rows Number of matrix rows.
	 * \tparam _Cols Number of matrix columns.
	 * \remark The dimensions of the matrix must be known at compile-time and cannot be altered at any time
	 *         during execution. If you require a matrix-object with run-time dimension manipulation then
	 *         use crsc::dynamic_matrix instead.
	 * \invariant Every row shall have an equal number of elements and every column shall have an equal number of elements
	 *            such that no holes occur in the structure.
	 * \author Samuel Rowlinson
	 * \date July, 2016
	 */
	template<typename _Ty,
		std::size_t _Rows,
		std::size_t _Cols
	> class fixed_matrix {
	public:
		// PUBLIC API TYPE DEFINITIONS
		typedef _Ty value_type;
		typedef _Ty& reference;
		typedef const _Ty& const_reference;
		typedef _Ty* pointer;
		typedef const _Ty* const_pointer;
		typedef std::size_t size_type;
		typedef std::ptrdiff_t difference_type;
		typedef typename std::array<_Ty, _Rows*_Cols>::const_iterator const_iterator;
		typedef typename std::array<_Ty, _Rows*_Cols>::iterator iterator;
		typedef typename std::array<_Ty, _Rows*_Cols>::const_reverse_iterator const_reverse_iterator;
		typedef typename std::array<_Ty, _Rows*_Cols>::reverse_iterator reverse_iterator;
	private:
		/**
		 * \class proxy_row_array
		 *
		 * \brief Proxy class used for enabling operator[][] overload on fixed_matrix objects.
		 */
		class proxy_row_array {
		public:
			proxy_row_array(std::array<value_type, _Rows*_Cols>& _row_array, size_type _row_index, size_type _cols)
				: row_array(_row_array), row_index(_row_index), cols(_cols) {}
			constexpr const_reference operator[](size_type _col_index) const {
				return row_array[row_index*cols + _col_index];
			}
			reference operator[](size_type _col_index) {
				return row_array[row_index*cols + _col_index];
			}
		private:
			std::array<value_type, _Rows*_Cols>& row_array;
			size_type row_index;
			size_type cols;
		};
	public:
		// CONSTRUCTION/ASSIGNMENT
		/**
		 * \brief Default constructor, initialises container with template-specified rows 
		 *        and columns each taking the default-constructed value of `_Ty`.
		 */
		fixed_matrix() : mtx() {}
		/**
		 * \brief Fill constructor, initialises container with template-specified rows
		 *        and columns each taking the value `_val`.
		 *
		 * \param _val Value to fill matrix with.
		 */
		explicit fixed_matrix(const value_type& _val) : mtx() { fill(_val); }
		/**
		 * \brief Converting constructor, intialises container from 2D C-style array.
		 *
		 * \warning If number of rows, columns of _c_arr_2d are not consistent
		 *          with _Rows, _Cols then undefined behaviour is invoked.
		 * \param _c_arr_2d Two-dimensional C-style array.
		 */
		explicit fixed_matrix(value_type** _c_arr_2d) : mtx() {
			for (size_type i = 0; i < _Rows; ++i) {
				for (size_type j = 0; j < _Cols; ++j)
					mtx[i*_Cols + j] = _c_arr_2d[i][j];
			}
		}
		/**
		 * \brief Copy constructor, constructs the container with the copy of the 
		 *        contents of `_other`.
		 *
		 * \param _other Another `fixed_matrix` container to be used as initialisation source.
		 */
		fixed_matrix(const fixed_matrix& _other) : mtx(_other.mtx) {}
		/**
		 * \brief Move constructor, constructs the container with the contents of
		 *        `_other` using move-semantics.
		 *
		 * \param _other rvalue reference to a `fixed_matrix` container to move to this.
		 */
		fixed_matrix(fixed_matrix&& _other) : mtx(std::move(_other.mtx)) {}
		/**
		 * \brief Constructs the container with the contents of the nested initializer list `_init_list`.
		 *
		 * \warning Undefined behaviour if sizes of each `std::initializer_list` within `_init_list` are not equivalent.
		 * \param _init_list Initializer list of initializer lists representing a matrix.
		 * \complexity Linear in size of `_Rows*_Cols`.
		 * \throw Throws `std::invalid_argument` exception if `_init_list.size() != _Rows` or if
		 *        `_init_list.begin()->size() != _Cols`.
		 */
		fixed_matrix(std::initializer_list<std::initializer_list<value_type>> _init_list) {
			if (_init_list.size() != _Rows || _init_list.begin()->size() != _Cols)
				throw std::invalid_argument("_init_list dimensions not consistent with fixed_matrix dimensions.");
			auto it = mtx.begin();
			for (const auto& el : _init_list) {
				std::copy(el.begin(), el.end(), it);
				std::advance(it, _Cols);
			}
		}
		/**
		 * \brief Destructs the container. The destructors of the elements are called and used
		 *        storage is deallocated.
		 */
		~fixed_matrix() {}
		/**
		 * \brief Copy-assignment operator. Replaces the contents of the container with
		 *        a copy of the contents of `_other`.
		 *
		 * \param _other Another `fixed_matrix` container to be used as data source.
		 * \return `*this`.
		 */
		fixed_matrix& operator=(const fixed_matrix& _other) {
			if (this != &_other)
				fixed_matrix(_other).swap(*this);
			return *this;
		}
		/**
		 * \brief Move-assignment operator. Replaces the contents of the container with
		 *        the contents of `_other` using move-semantics.
		 *
		 * \param _other rvalue reference to a `fixed_matrix` container to move to this.
		 * \return `*this`.
		 */
		fixed_matrix& operator=(fixed_matrix&& _other) {
			if (this != &_other)
				swap(*this, _other);
			return *this;
		}
		// CAPACITY
		/**
		 * \brief Checks if the container has no elements.
		 *
		 * \return `true` if the container is empty, `false` otherwise.
		 * \complexity Constant.
		 * \exceptionsafety No-throw guarantee, `noexcept` specification.
		 */
		constexpr bool empty() const noexcept {
			return mtx.empty();
		}
		/**
		 * \brief Returns the number of rows in the container.
		 *
		 * \return Number of rows.
		 * \complexity Constant.
	 	 * \exceptionsafety No-throw guarantee, `noexcept` specification.
		 */
		constexpr size_type rows() const noexcept {
			return _Rows;
		}
		/**
		 * \brief Returns the number of columns in the container.
		 *
		 * \return Number of columns.
		 * \complexity Constant.
		 * \exceptionsafety No-throw guarantee, `noexcept` specification.
		 */
		constexpr size_type columns() const noexcept {
			return _Cols;
		}
		/**
		 * \brief Returns the size of the container in terms of number of elements it holds.
		 *
		 * \return Number of elements in the container.
		 * \complexity Constant.
		 * \exceptionsafety No-throw guarantee, `noexcept` specification.
		 */
		constexpr size_type size() const noexcept {
			return mtx.size();
		}
		/**
		 * \brief Returns the maximum number of elements the container can hold, this is
		 *        equivalent to `size()` as the container is of a fixed size.
		 *
		 * \return Maximum number of elements.
		 * \complexity Constant.
		 * \exceptionsafety No-throw guarantee, `noexcept` specification.
		 */
		constexpr size_type max_size() const noexcept {
			return mtx.max_size();
		}
		// ELEMENT ACCESS
		/**
		 * \brief Gets const_reference to element at specified row-column indices.
		 *
		 * \param _row_index Row position.
		 * \param _col_index Column position.
		 * \return Constant reference to element at given position.
		 * \throw Throws std::out_of_range exception if either of `_row_index`, `_col_index`
		 *        are greater than or equal to rows(), columns() respectively.
		 * \complexity Constant.
		 * \exceptionsafety Strong guarantee - if an exception is thrown there are no
		 *                  changes in the container.
		 */
		constexpr const_reference at(size_type _row_index, size_type _col_index) const {
			if (_row_index >= _Rows || _col_index >= _Cols)
				throw std::out_of_range("fixed_matrix index out of bounds.");
			return mtx.at(_row_index*_Cols + _col_index);
		}
		/**
		 * \brief Gets reference to element at specified row-column indices.
		 *
		 * \param _row_index Row position.
		 * \param _col_index Column position.
		 * \return Constant reference to element at given position.
		 * \throw Throws std::out_of_range exception if either of `_row_index`, `_col_index`
		 *        are greater than or equal to rows(), columns() respectively.
		 * \complexity Constant.
		 * \exceptionsafety Strong guarantee - if an exception is thrown there are no
		 *                  changes in the container.
		 */
		reference at(size_type _row_index, size_type _col_index) {
			if (_row_index >= _Rows || _col_index >= _Cols)
				throw std::out_of_range("fixed_matrix index out of bounds.");
			return mtx.at(_row_index*_Cols + _col_index);
		}
		/**
		 * \brief Gets a proxy object representing the row vector of the matrix at a given `_row_index`
		 *        such that calling the double subscript operator gets a `const_reference` to the 
		 *        element at the specified position.
		 *
		 * \param _row_index Row position.
		 * \return Object of type `proxy_row_array` corresponding to given `_row_index`.
		 * \complexity Constant.
		 * \exceptionsafety No-throw guarantee if `_row_index < rows()`, otherwise undefined behaviour.
		 */
		constexpr proxy_row_array operator[](size_type _row_index) const {
			return proxy_row_array(mtx, _row_index, _Cols);
		}
		/**
		 * \brief Gets a proxy object representing the row vector of the matrix at a given `_row_index`
		 *        such that calling the double subscript operator gets a `reference` to the element
		 *        at the specified position.
		 *
		 * \param _row_index Row position.
		 * \return Object of type `proxy_row_array` corresponding to given `_row_index`.
		 * \complexity Constant.
		 * \exceptionsafety No-throw guarantee if `_row_index < rows()`, otherwise undefined behaviour.
		 */
		proxy_row_array operator[](size_type _row_index) {
			return proxy_row_array(mtx, _row_index, _Cols);
		}
		/**
		 * \brief Gets `const_reference` to element at specified row-column indices.
		 *
		 * \param _row_index Row position.
		 * \param _col_index Column position.
		 * \return Constant reference to element at given position.
		 * \complexity Constant.
		 * \exceptionsafety No-throw guarantee if `_row_index < rows() && _col_index < columns()`,
		 *                  otherwise undefined behaviour.
		 */
		constexpr const_reference operator()(size_type _row_index, size_type _col_index) const {
			return mtx[_row_index*_Cols + _col_index];
		}
		/**
		 * \brief Gets `reference` to element at specified row-column indices.
		 *
		 * \param _row_index Row position.
		 * \param _col_index Column position.
		 * \return Reference to element at given position.
		 * \complexity Constant.
		 * \exceptionsafety No-throw guarantee if `_row_index < rows() && _col_index < columns()`,
		 *                  otherwise undefined behaviour.
		 */
		reference operator()(size_type _row_index, size_type _col_index) {
			return mtx[_row_index*_Cols + _col_index];
		}
		/**
		 * \brief Returns a `const_reference` to the first element in the container.
		 *
		 * \return Constant reference to the first element.
		 * \complexity Constant.
		 * \exceptionsafety No-throw guarantee if `!empty()`, otherwise undefined behaviour.
		 */
		constexpr const_reference front() const {
			return mtx.front();
		}
		/**
		 * \brief Returns a `reference` to the first element in the container.
		 *
		 * \return Reference to the first element.
		 * \complexity Constant.
		 * \exceptionsafety No-throw guarantee if `!empty()`, otherwise undefined behaviour.
		 */
		reference front() {
			return mtx.front();
		}
		/**
		 * \brief Returns a `const_reference` to the last element in the container.
		 *
		 * \return Constant reference to the last element.
		 * \complexity Constant.
		 * \exceptionsafety No-throw guarantee if `!empty()`, otherwise undefined behaviour.
		 */
		constexpr const_reference back() const {
			return mtx.back();
		}
		/**
		 * \brief Returns a `reference` to the last element in the container.
		 *
		 * \return Reference to the last element.
		 * \complexity Constant.
		 * \exceptionsafety No-throw guarantee if `!empty()`, otherwise undefined behaviour.
		 */
		reference back() {
			return mtx.back();
		}
		/**
		 * \brief Returns `const_pointer` to the underlying array `const _Ty*` serving as element storage.
		 *
		 * This pointer is such that the range `[data(), data() + size()]` is always a valid range, even if
		 * the container is empty - `data()` is non-dereferencable in this case.
		 *
		 * \return Constant pointer to underlying element storage, for non-empty container => `&front()`.
		 * \complexity Constant.
		 * \exceptionsafety No-throw guarantee, `noexcept` specification.
		 */
		const_pointer data() const noexcept {
			return mtx.data();
		}
		/**
		 * \brief Returns pointer to the underling array `_Ty*` serving as element storage.
		 *
		 * This pointer is such that the range `[data(), data() + size()]` is always a valid range, even if
		 * the container is empty - `data()` is non-dereferencable in this case.
		 *
		 * \return Pointer to underlying element storage, for non-empty container => `&front()`.
		 * \complexity Constant.
		 * \exceptionsafety No-throw guarantee, `noexcept` specification.
		 */
		pointer data() noexcept {
			return mtx.data();
		}
		/**
		 * \brief Sends the container data to a `std::ostream` instance in a mathematical-matrix style format.
		 *
		 * \param _os Instance of `std::ostream` to write to.
		 * \param _delim Delimiter separating elements on each line of the `fixed_matrix`.
		 * \return Modified reference to `_os`.
		 * \complexity Linear in `rows()*columns()`.
		 * \exceptionsafety No-throw guarantee, `noexcept` specification.
		 */
		template<class _Uty = _Ty,
			class = std::enable_if_t<has_insertion_operator<_Uty>::value>
		> std::ostream& write(std::ostream& _os, char _delim = ' ') const noexcept {
			size_type count = 0;
			for (const auto& el : mtx) {
				_os << el << _delim;
				++count;
				if (!(count % _Cols))
					_os << '\n';
			}
			return _os;
		}
		// ITERATORS
		/**
		 * \brief Returns a const_iterator the first element of the container.
		 *
		 * \remark If the container is empty, the return value will be equal to `cend()`.
		 * \return Constant iterator to the first element.
		 * \complexity Constant.
		 * \exceptionsafety No-throw guarantee, `noexcept` specification.
		 */
		const_iterator cbegin() const {
			return mtx.cbegin();
		}
		/**
		 * \brief Returns an iterator to the first element of the container.
		 *
		 * \remark If the container is empty, the return value will be equal to `end()`.
		 * \return Iterator to the first element.
		 * \complexity Constant.
		 * \exceptionsafety No-throw guarantee, `noexcept` specification.
		 */
		iterator begin() {
			return mtx.begin();
		}
		/**
		 * \brief Returns a const_iterator to the past-the-end element of the container.
		 *
		 * \return Constant iterator to the past-the-end element.
		 * \complexity Constant.
		 * \exceptionsafety No-throw guarantee, `noexcept` specification.
		 */
		const_iterator cend() const {
			return mtx.cend();
		}
		/**
		 * \brief Returns an iterator to the past-the-end element of the container.
		 *
		 * \return Iterator to the past-the-end element.
		 * \complexity Constant.
		 * \exceptionsafety No-throw guarantee, `noexcept` specification.
		 */
		iterator end() {
			return mtx.end();
		}
		/**
		 * \brief Returns a const_reverse_iterator to the first element of the reversed container. It
		 *        corresponds to the last element of the non-reversed container.
		 *
		 * \return Constant reverse iterator to the first element.
		 * \complexity Constant.
		 * \exceptionsafety No-throw guarantee, `noexcept` specification.
		 */
		const_reverse_iterator crbegin() const {
			return mtx.crbegin();
		}
		/**
		 * \brief Returns a reverse_iterator to the first element of the reversed container. It
		 *        corresponds to the last element of the non-reversed container.
		 *
		 * \return Reverse iterator to the first element.
		 * \complexity Constant.
		 * \exceptionsafety No-throw guarantee, `noexcept` specification.
		 */
		reverse_iterator rbegin() {
			return mtx.rbegin();
		}
		/**
		 * \brief Returns a const_reverse_iterator to the past-the-end element of the reversed container. It
		 *        corresponds to the element preceding the first element of the non-reversed container.
		 *
		 * \return Constance reverse iterator to the past-the-end element.
		 * \complexity Constant.
		 * \exceptionsafety No-throw guarantee, `noexcept` specification.
		 */
		const_reverse_iterator crend() const {
			return mtx.crend();
		}
		/**
		 * \brief Returns a reverse_iterator to the past-the-end element of the reversed container. It
		 *        corresponds to the element preceding the first element of the non-reversed container.
		 *
		 * \return Reverse iterator to the past-the-end element.
		 * \complexity Constant.
		 * \exceptionsafety No-throw guarantee, `noexcept` specification.
		 */
		reverse_iterator rend() {
			return mtx.rend();
		}
		// OPERATIONS
		/**
		 * \brief Assigns the given value `_val` to all elements in the container.
		 *
		 * \param _val Value to assign to all elements.
		 * \complexity Exactly `rows()*columns()` assignments.
		 * \exceptionsafety No-throw guarantee, `noexcept` specification.
		 */
		void fill(const value_type& _val) {
			mtx.fill(_val);
		}
		/**
		 * \brief Exchanges the contents of the container with those of `_other`. Does not
		 *        cause iterators and references to associate with the other container.
		 *
		 * \param _other `fixed_matrix` container to swap with.
		 * \complexity Constant.
		 * \exceptionsafety No-throw guarantee.
		 */
		void swap(fixed_matrix& _other) {
			mtx.swap(_other.mtx);
		}
		/**
		 * \brief Exchanges the contents of two `crsc::fixed_matrix` containers. Does not
		 *        cause iterators and references to associate with the other containers.
		 *
		 * \param lhs First instance of `fixed_matrix`.
		 * \param rhs Second instance of `fixed_matrix`.
		 * \complexity Constant.
		 * \exceptionsafety No-throw guarantee.
		 */
		static void swap(fixed_matrix& lhs, fixed_matrix& rhs) {
			lhs.swap(rhs);
		}
		/**
		 * \brief Gets the submatrix of the container obtained by removing the specified row and column
		 *        and returning the resulting matrix.
		 *
		 * \param _row_index Index of row to remove.
		 * \param _col_index Index of column to remove.
		 * \return Submatrix of the container with specified row, column removed.
		 * \complexity Linear in `(rows()-1)*(columns()-1)` plus complexity of containers'
		 *             copy constructor (subject to RVO).
		 * \exceptionsafety No-throw guarantee, `noexcept` specification.
		 */
		fixed_matrix<value_type, _Rows - 1, _Cols - 1> submatrix(size_type _row_index, size_type _col_index) const noexcept {
			fixed_matrix<value_type, _Rows - 1, _Cols - 1> sub;
			size_type row_erased = 0;
			size_type col_erased = 0;
			for (size_type i = 0; i < _Rows - 1; ++i) {
				col_erased = 0;
				for (size_type j = 0; j < _Cols - 1; ++j) {
					if (i == _row_index)
						row_erased = 1;
					if (j == _col_index)
						col_erased = 1;
					sub.at(i, j) = at(i + row_erased, j + col_erased);
				}
			}
			return sub;
		}
		/**
		 * \brief Computes the trace of a square `fixed_matrix` container.
		 *
		 * \return The trace of the matrix-container.
		 * \throw Throws `std::logic_error` if `rows() != columns()`.
		 * \complexity Linear in `rows()`.
		 * \exceptionsafety Strong-guarantee - if an exception is thrown there are no changes
		 *                  in the container.
		 */
		value_type trace() const {
			if (_Rows != _Cols)
				throw std::logic_error("cannot compute trace() of non-square matrix.");
			value_type result = value_type();
			for (size_type i = 0; i < _Rows; ++i)
				result += mtx[i*(_Cols + 1)];
			return result;
		}
		// OPERATORS
		/**
		 * \brief Adds each element of `_other` container to this container.
		 *
		 * \param _other Container to add element-wise to this.
		 * \return `*this`.
		 * \complexity Linear in `rows()*columns()`.
		 * \exceptionsafety Strong guarantee - if an exception is thrown there are no changes
		 *                  in the container.
		 */
		fixed_matrix& operator+=(const fixed_matrix& _other) {
			for (auto it = mtx.begin(), it_other = _other.mtx.begin(); it < mtx.end(); ++it, ++it_other)
				*it += *it_other;
			return *this;
		}
		/**
		 * \brief Subtracts each element of `_other` container from this container.
		 *
		 * \param _other Container to subtract element-wise from this.
		 * \return `*this`.
		 * \complexity Linear in `rows()*columns()`.
		 * \exceptionsafety Strong-guarantee - if an exception is thrown there are no changes
		 *                  in the container.
		 */
		fixed_matrix& operator-=(const fixed_matrix& _other) {
			for (auto it = mtx.begin(), it_other = _other.mtx.begin(); it < mtx.end(); ++it, ++it_other)
				*it -= *it_other;
			return *this;
		}
		/**
		 * \brief Copies this container and adds each element of `_other` to the copy then returns it.
		 *
		 * \param _other Container to add to the copy.
		 * \return Container consisting of sum of `*this` and `_other`.
		 * \complexity Linear in `rows()*columns()` (assignments) plus linear in
	 	 *             `rows()*columns()` (additions).
		 * \exceptionsafety Strong-guarantee - if an exception is thrown there are no changes
		 *                  in the container.
		 */
		fixed_matrix operator+(const fixed_matrix& _other) const {
			fixed_matrix<value_type, _Rows, _Cols> tmp(*this);
			return tmp += _other;
		}
		/**
		 * \brief Copies this container and subtracts each element of `_other` from the copy then returns it.
		 *
		 * \param _other Container to subtract from the copy.
		 * \return Container consisting of difference of `*this` and `_other`.
		 * \complexity Linear in `rows()*columns()` (assignments) plus linear in
		 *             `rows()*columns()` (subtractions).
		 * \exceptionsafety Strong-guarantee - if an exception is thrown there are no changes
		 *                  in the container.
		 */
		fixed_matrix operator-(const fixed_matrix& _other) const {
			fixed_matrix<value_type, _Rows, _Cols> tmp(*this);
			return tmp -= _other;
		}
		/**
		 * \brief Performs matrix multiplication of `*this` multiplied by `_other` and returns the
		 *        result as a new container.
		 *
		 * \param _other Container to multiply with `*this`.
		 * \return Container consisting of product of `*this` and `_other`.
		 * \complexity Linear in `rows()*_other.columns()*columns()`.
	 	 * \exceptionsafety Strong-guarantee - if an exception is thrown there are no changes
		 *                  in the container.
		 */
		template<
			std::size_t _Second_Rows,
			std::size_t _Second_Cols
		> fixed_matrix<value_type, _Rows, _Second_Cols> operator*(const fixed_matrix<value_type, _Second_Rows, _Second_Cols>& _other) const {
			fixed_matrix<value_type, _Rows, _Second_Cols> product;
			for (size_type i = 0; i < _Rows; ++i) {
				for (size_type j = 0; j < _Second_Cols; ++j) {
					for (size_type k = 0; k < _Cols; ++k)
						product(i, j) += (*this)(i, j) * _other(i, j);
				}
			}
			return product;
		}
		/**
		 * \brief Checks for equality of this container and `_other`.
		 *
		 * \param _other Container to check for equality.
		 * \return `true` if `*this` equals, element-wise `_other`, otherwise `false`.
		 * \complexity Constant if `rows() != _other.rows() || columns != _other.columns()`,
		 *             otherwise linear in `rows()*columns()`.
		 * \exceptionsafety No-throw guarantee, `noexcept` specification.
		 */
		bool operator==(const fixed_matrix& _other) const noexcept {
			if (this != &_other)
				return std::equal(mtx.begin(), mtx.end(), _other.mtx.begin());
			return true;
		}
		/**
		 * \brief Checks for inequality of this container and `_other`.
		 *
		 * \param _other Container to check for inequality.
		 * \return `true` if `*this` unequals, element-wise `_other`, otherwise `false`.
		 * \complexity Constant if `rows() != _other.rows() || columns != _other.columns()`,
	 	 *             otherwise linear in `rows()*columns()`.
		 * \exceptionsafety No-throw guarantee, `noexcept` specification.
		 */
		bool operator!=(const fixed_matrix& _other) const noexcept {
			return !(*this == _other);
		}
	private:
		std::array<value_type, _Rows*_Cols> mtx;
	};
	template<typename _Ty,
		std::size_t _Rows,
		std::size_t _Cols,
		class = std::enable_if_t<has_insertion_operator<_Ty>::value>
	> std::ostream& operator<<(std::ostream& _os, const fixed_matrix<_Ty, _Rows, _Cols>& _fm) {
		return _fm.write(_os);
	}
	/**
	 * \brief Makes an identity `fixed_matrix` of template-specified size.
	 * 
	 * \tparam _Ty The type of stored elements, must satisfy `std::is_arithmetic<_Ty>::value`.
	 * \tparam _rows Number of rows.
	 * \tparam _cols Number of columns.
	 * \remark Only enabled if `_rows == _columns && std::is_arithmetic<_Ty>::value`.
	 * \return Identity `fixed_matrix` of given dimensions.
	 * \complexity Linear in `_rows*_cols` plus complexity of container's copy constructor (subject
	 *             to RVO).
	 */
	template<typename _Ty,
		std::size_t _rows,
		std::size_t _cols,
		class = std::enable_if_t<_rows == _cols
			&& std::is_arithmetic<_Ty>::value>
	> fixed_matrix<_Ty, _rows, _cols> make_identity_matrix() {
		fixed_matrix<_Ty, _rows, _cols> identity_matrix();
		for (std::size_t i = 0; i < _rows; ++i) {
			for (std::size_t j = 0; j < _cols; ++j)
				if (i == j) identity_matrix[i][j] = static_cast<_Ty>(1);
		}
		return identity_matrix;
	}
	/**
	 * \brief Makes a `fixed_matrix` object from a 2D C-style array.
	 *
	 * \warning This method does not delete `c_arr_2d` after use.
	 * \tparam _Ty The type of stored elements.
	 * \tparam _rows Number of rows.
	 * \tparam _cols Number of columns.
	 * \param c_arr_2d Two-dimensional C-style array used as data source.
	 * \return A `fixed_matrix` object constructed using the contents of `c_arr_2d`.
	 * \complexity Linear in `_rows*_cols` plus complexity of container's copy constructor (subject
	 *             to RVO).
	 */
	template<typename _Ty,
		std::size_t _rows,
		std::size_t _cols
	> fixed_matrix<_Ty, _rows, _cols> to_fixed_matrix(_Ty** c_arr_2d) {
		return fixed_matrix<_Ty, _rows, _cols>(c_arr_2d);
	}
}

#endif // !FIXED_MATRIX_H
