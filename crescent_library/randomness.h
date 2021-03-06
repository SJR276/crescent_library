#include "algorithm_utilities.h"
#include <complex>
#include <numeric>
#include <random>

namespace crsc {
#ifndef DISCRETE_TRIANGULAR_DISTRIBUTION_H
#define DISCRETE_TRIANGULAR_DISTRIBUTION_H
	/**
	 * \class discrete_triangular_distribution
	 *
	 * \brief `crsc::discrete_triangular_distribution` produces random integers on the interval `[0, n)` where
	 *        `n` is a maximum specified during construction and the probability of each integer i is defined as
	 *        its index in the series over the total series size in the case of an ascending triangle or 1 minus
	 *        this quantity for a descending triangle.
	 *
	 * `crsc::discrete_triangular_distribution` satisfies all the requirements of `RandomNumberDistribution` (see
	 * C++ Concepts).
	 *
	 * \tparam IntType The result type generated by the generator. The effect is undefined if this is not one 
	 *         of `short`, `int`, `long`, `long long`, `unsigned short`, `unsigned int`, `unsigned long` or
	 *         `unsigned long long`.
	 */
	template<class IntType = int>
	class discrete_triangular_distribution {
	public:
		typedef IntType result_type;
		typedef typename std::discrete_distribution<IntType>::param_type param_type;
		// CONSTRUCTION/ASSIGNMENT
		/**
		 * \brief Constructs a new distribution object. Constructs the distribution with linearly increasing weights
		 *        up to a `max` if `ascending == true` or linearly descending weights from `max` if `ascending == false`.
		 *
		 * \param max Maximum value of distribution.
		 * \param ascending Determines whether the weightings are linearly ascending or descending.
		 */
		explicit discrete_triangular_distribution(IntType max, bool ascending = true) {
			std::vector<IntType> weights(max);
			if (ascending) std::iota(weights.begin(), weights.end(), 0);
			else crsc::iota_opp(weights.begin(), weights.end(), max);
			dd = std::discrete_distribution<IntType>(std::move(weights));
		}
		/**
		 * \brief Resets the internal state of the distribution object.
		 *
		 * After a call to this function, the next call to `operator()` on the distribution
		 * object will not be dependent on previous calls to `operator()`.
		 */
		void reset() { dd.reset(); }
		// GENERATION
		/**
		 * \brief Generates random numbers that are distributed according to the associated probability function. The
		 *        entropy is acquired by calling `g.operator()`. Uses the associated parameter set.
		 * \param g An uniform random bit generator object.
		 */
		template<class Generator>
		result_type operator()(Generator& g) { return dd(g); }
		/**
		 * \brief Generates random numbers that are distributed according to the associated probability function. The
		 *        entropy is acquired by calling `g.operator()`. Uses `params` as parameter set.
		 * \param g An uniform random bit generator object.
		 * \param params Distribution parameter set to use instead of the associated one.
		 */
		template<class Generator>
		result_type operator()(Generator& g, const param_type& params) { return dd(g, params); }
		// CHARACTERISTICS
		/**
		 * \brief Obtains a `std::vector<double>` containing the individual probabilities of each integer that is
		 *        generated by this distribution.
		 * \return An object of type `std::vector<double>`.
		 */
		std::vector<double> probabilities() const noexcept { return dd.probabilities(); }
		/**
		 * \brief Returns the associated distribution parameter set.
		 * \return The associated parameter set.
		 */
		param_type param() const noexcept { return dd.param(); }
		/**
		 * \brief Sets the associated distribution parameter set.
		 * \param params New contents of the associated parameter set.
		 */
		void param(const param_type& params) noexcept { dd.param(params); }
		/**
		 * \brief Returns the minimum value potentially generated by the distribution.
		 * \return The minimum value potentially generated by the distribution.
		 */
		result_type min() const noexcept { return dd.min(); }
		/**
		 * \brief Returns the maximum value potentially generated by the distribution.
		 * \return The maximum value potentially generated by the distribution.
		 */
		result_type max() const noexcept { return dd.max(); }
	private:
		std::discrete_distribution<IntType> dd;
	};
#endif // !DISCRETE_TRIANGULAR_DISTRIBUTION_H
#ifndef RANDOM_NUMBER_GENERATOR_H
#define RANDOM_NUMBER_GENERATOR_H
	/**
	 * \class random_number_generator
	 *
	 * \brief Pseudo-random number generator for random values over a specified `Distribution`
	 *        using a given `Generator` engine.
	 *
	 * A convenience wrapper around a generator engine and random number distribution used for 
	 * generating random values quickly and simply. Any pre-defined generator from the C++ `<random>`
	 * header may be used as the `Generator` type-param and any distribution from this header
	 * may be used for the `Distribution` type-param. The next value in the random distribution
	 * is generated via a call to `random_number_generator::operator()`. Resetting the internal
	 * state of the distribution such that the next generating call is not dependent upon the last
	 * call is achieved via a call to `random_number_generator::reset_distribution_state()`.
	 *
	 * Note that the type `Ty` (referenced as `result_type` in the class API) must match the type of
	 * distribution `Distribution` (referenced as `distribution_type` in the class API) used, e.g.
	 * if the `result_type` is `int` then it is undefined behaviour to use a distribution type 
	 * intended for floating point types.
	 *
	 * \tparam Ty The type of the values to generate, must satisfy `std::is_arithmetic<Ty>`. Defaults
	 *         to the integral type `int`.
	 * \tparam Generator The type of the generator engine to use for pseudo-random generation, must
	 *         meet the requirement of `UniformRandomBitGenerator` (see C++ Concepts). Defaults to
	 *         the engine type `std::mt19937`.
	 * \tparam Distribution The type of distribution over which to calculate the random numbers. The
	 *         value type of the distribution must match the result type `Ty` of this class. Must meet
	 *         the requirement of `RandomNumberDistribution` (see C++ Conecpts). Defaults to the
	 *         distribution type `std::uniform_int_distribution<Ty>`.
	 */
	template<class Ty = int,
		class Generator = std::mt19937,
		class Distribution = std::uniform_int_distribution<Ty>,
		class = std::enable_if_t<std::is_arithmetic<Ty>::value>
	> class random_number_generator {
	public:
		// PUBLIC API TYPE DEFINITIONS
		typedef Ty result_type;
		typedef Generator generator_type;
		typedef Distribution distribution_type;
		// CONSTRUCTION/ASSIGNMENT
		/**
		 * \brief Move constructs the generator with the values of the engine `_eng` and the
		 *        distribution `_dist`. This is also the default constructor.
		 *
		 * \param _eng Generator engine to use.
		 * \param _dist Distribution for random numbers.
		 */
		explicit random_number_generator(Generator&& _eng = Generator{std::random_device{}()}, Distribution&& _dist = Distribution())
			: eng(std::move(_eng)), dist(std::move(_dist)) {}
		/**
		 * \brief Move constructs the generator with the values of the distribution `_dist`
		 *       and the engine `_eng`, the latter of which is default-initialised with a
		 *       single `std::random_device{}()` whilst the former is not default-initialised.
		 *
		 * \param _dist Distribution for random numbers.
		 * \param _eng Generator engine to use.
		 */
		explicit random_number_generator(Distribution&& _dist, Generator&& _eng = Generator{std::random_device{}()})
			: eng(std::move(_eng)), dist(std::move(_dist)) {}
		/**
		 * \brief Constructs the generator with a copy of the values of the engine `_eng` and
		 *        the distribution `_dist`.
		 *
		 * \param _eng Generator engine to use.
		 * \param _dist Distribution for random numbers.
		 */
		explicit random_number_generator(const Generator& _eng, const Distribution& _dist)
			: eng(_eng), dist(_dist) {}
		/**
		 * \brief Copy constructor. Constructs the generator with a copy of the fields of `other`.
		 *
		 * \param other `random_number_generator` instance to use as data source.
		 */
		random_number_generator(const random_number_generator& other)
			: eng(other.eng), dist(other.dist) {}
		/**
		 * \brief Move constructor. Constructs the generator with the fields of `other` using
		 *        move-semantics such that `other` is left in a valid but unspecified state.
		 *
		 * \param other `random_number_generator` instance to use as data source.
		 */
		random_number_generator(random_number_generator&& other)
			: eng(std::move(other.eng)), dist(std::move(other.dist)) {}
		/**
		 * \brief Copy-assignment operator. Replaces the generator with a copy of the fields of `other`.
		 * \param other `random_number_generator` instance to use as data source.
		 * \return `*this`.
		 */
		random_number_generator& operator=(const random_number_generator& other) {
			if (this != &other)
				random_number_generator(other).swap(*this); // copy-and-swap
			return *this;
		}
		/**
		 * \brief Move-assignment operator. Replaces the generator with the field of `other` 
		 *        using move-semantics.
		 * \param other `random_number_generator` instance to use as data source.
		 * \return `*this`.
		 */
		random_number_generator& operator=(random_number_generator&& other) {
			if (this != &other) {
				eng = std::move(other.eng);
				dist = std::move(other.dist);
			}
			return *this;
		}
		// GENERATING OPERATOR()
		/**
		 * \brief Generates the next random number in the distribution.
		 *
		 * \return The generated random number.
		 */
		result_type operator()() { return dist(eng); }
		// GENERATOR AND DISTRIBUTION OBJECT ACCESS
		/**
		 * \brief Returns a copy of the underlying generator engine.
		 * \return A copy of the underlying engine used for random number generation.
		 */
		generator_type get_generator() const noexcept { return eng; }
		/**
		 * \brief Returns a copy of the underlying distribution.
		 * \return A copy of the underlying distribution over which the random
		 *         numbers are generated.
		 */
		distribution_type get_distribution() const noexcept { return dist; }
		// PROPERTIES
		/**
		 * \brief Returns the minimum potentially generated value.
		 * \return The minimum value potentially generated by the underlying distribution.
		 */
		result_type min() const { return dist.min(); }
		/**
		 * \brief Returns the maximum potentially generated value.
		 * \return The maximum value potentially generated by the underlying distribution.
		 */
		result_type max() const { return dist.max(); }
		// MODIFIERS
		/**
		 * \brief Resets the internal state of the underlying distribution object. After calling this function,
		 *        the next call to `operator()` on the generator will not be dependent upon previous calls
		 *        to `operator()`.
		 */
		void reset_distribution_state() { dist.reset(); }
		/**
		 * \brief Exchanges the fields of the generator with those of `other`.
		 *
		 * \param other `random_number_generator` object to swap with.
		 */
		void swap(random_number_generator& other) {
			std::swap(eng, other.eng);
			std::swap(dist, other.dist);
		}
	private:
		generator_type eng;
		distribution_type dist;
	};
#endif // !RANDOM_NUMBER_GENERATOR_H
#ifndef UNIFORM_RANDOM_PROBABILITY_GENERATOR_H
#define UNIFORM_RANDOM_PROBABILITY_GENERATOR_H
	/**
	 * \class uniform_random_probability_generator
	 *
	 * \brief Pseudo-random number generator for random floating point values distributed
	 *        uniformly over the range [0.0, 1.0] using a given `Generator` engine.
	 *
	 * A convenience wrapper around a template specialisation of `crsc::random_number_generator` providing
	 * a class to produce uniformly distributed floating point values in the range [0.0, 1.0]. Any pre-defined
	 * generator from the C++ `<random>` header may be used as the `Generator` type-param. The next value in
	 * the random distribution is generated via a call to `uniform_random_probability_generator::operator()`. 
	 * Resetting the internal state of the distribution such that the next generating call is not dependent upon
	 * the last call is achieved via a call to `uniform_random_probability_generator::reset_distribution_state()`.
	 *
	 * \tparam FloatType The type of the probabilities to generator, must satisfy `std::is_floating_point<FloatType>`.
	 *         Defaults to the type `double`.
	 * \tparam Generator The type of the generator engine to use for pseudo-random generation, must
	 *         meet the requirement of `UniformRandomBitGenerator` (see C++ Concepts). Defaults to
	 *         the engine type `std::mt19937`. 
	 */
	template<class FloatType = double,
		class Generator = std::mt19937,
		class = std::enable_if_t<std::is_floating_point<FloatType>::value>
	> class uniform_random_probability_generator {
		typedef random_number_generator<FloatType, Generator, std::uniform_real_distribution<FloatType>> uniform_pr_gen;
	public:
		// PUBLIC API TYPE DEFINITIONS
		typedef typename uniform_pr_gen::result_type result_type;
		typedef typename uniform_pr_gen::generator_type generator_type;
		typedef typename uniform_pr_gen::distribution_type distribution_type;
		// CONSTRUCTION/ASSIGNMENT
		/**
		 * \brief Move constructs the generator with the value of the pre-defined generator `engine`. This
		 *        is also the default constructor.
		 *
		 * \param engine Generator engine to use.
		 */
		explicit uniform_random_probability_generator(Generator&& engine = Generator{std::random_device{}()})
			: generator(std::move(engine)) {}
		/**
		 * \brief Constructs the gnerator with a copy of the pre-defined generator `engine`.
		 *
		 * \param engine Generator engine to use.
		 */
		explicit uniform_random_probability_generator(const Generator& engine)
			: generator(engine, distribution_type()) {}
		/**
		 * \brief Copy constructor. Constructs the generator with a copy of the fields of `other`.
		 *
		 * \param other `uniform_random_probability_generator` instance to use as data source.
		 */
		uniform_random_probability_generator(const uniform_random_probability_generator& other)
			: generator(other.generator) {}
		/**
		 * \brief Move constructor. Constructs the generator with the fields of `other` using
		 *        move-semantics such that `other` is left in a valid but unspecified state.
		 *
		 * \param other `uniform_random_probability_generator` instance to use as data source.
		 */
		uniform_random_probability_generator(uniform_random_probability_generator&& other)
			: generator(std::move(other.generator)) {}
		/**
		 * \brief Copy assignment operator. Replaces the generator with a copy of the fields of `other`.
		 * \param other `uniform_random_probability_generator` instance to use as data source.
		 * \return `*this`.
		 */
		uniform_random_probability_generator& operator=(const uniform_random_probability_generator& other) {
			if (this != &other)
				generator = other.generator;
			return *this;
		}
		/**
		 * \brief Move-assignment operator. Replaces the generator with the fields of `other` using move-semantics.
		 * \param other `uniform_random_probability_generator` instance to use as data source.
		 * \return `*this`.
		 */
		uniform_random_probability_generator& operator=(uniform_random_probability_generator&& other) {
			if (this != &other)
				generator = std::move(other.generator);
			return *this;
		}
		// GENERATING OPERATOR()
		/**
		 * \brief Generates the next random number in the distribution.
		 * \return The generated random number.
		 */
		result_type operator()() { return generator(); }
		// GENERATOR AND DISTRIBUTION OBJECT ACCESS
		/**
		 * \brief Returns a copy of the underlying distribution.
		 * \return A copy of the underlying distribution over which the random
		 *         numbers are generated.
		 */
		generator_type get_generator() const noexcept { return generator.get_generator(); }
		/**
		 * \brief Returns a copy of the underlying distribution.
		 * \return A copy of the underlying distribution over which the random
		 *         numbers are generated.
		 */
		distribution_type get_distribution() const noexcept { return generator.get_distribution(); }
		// PROPERTIES
		/**
		 * \brief Returns the minimum potentially generated value (always 0.0).
		 * \return The minimum value potentially generated by the underlying distribution.
		 */
		constexpr result_type min() const { return generator.min(); }
		/**
		 * \brief Returns the maximum potentially generated value (always 1.0).
		 * \return The maximum value potentially generated by the underlying distribution.
		 */
		constexpr result_type max() const { return generator.max(); }
		// MODIFIERS
		/**
		 * \brief Resets the internal state of the underlying distribution object. After calling this function,
		 *        the next call to `operator()` on the generator will not be dependent upon previous calls
		 *        to `operator()`.
		 */
		void reset_distribution_state() { generator.reset_distribution_state(); }
		/**
		 * \brief Exchanges the fields of the generator with those of `other`.
		 *
		 * \param other `uniform_random_probability_generator` object to swap with.
		 */
		void swap(uniform_random_probability_generator& other) { generator.swap(other.generator); }
	private:
		uniform_pr_gen generator;
	};
#endif // !UNIFORM_RANDOM_PROBABILITY_GENERATOR_H
#ifndef RANDOM_COMPLEX_GENERATOR_H
#define RANDOM_COMPLEX_GENERATOR_H
	/**
	 * \class random_complex_generator
	 *
	 * \brief Generator for random complex numbers in `std::complex` object form over a specified `Distribution`
	 *        using a given `Generator` engine.
	 *
	 * A convenience wrapper around a generator engine and random number distribution used for generating random complex
	 * numbers quickly and simply. Any pre-defined generator from the C++ `<random>` header may be used as the `Generator`
	 * type-param and any distribution from this header may be used for the `Distribution` type-param. The next value in
	 * the random distribution is generated via a call to `random_complex_generator::operator(bool)` where the `bool` param
	 * indicates whether the random complex number generated shall have equal Real and Imaginary parts. Resetting the internal
	 * state of the distribution such that the next generating call is not dependent upon the last call is achieved via a 
	 * call to `random_complex_generator::reset_distribution_state()`.
	 *
	 * Note that the type `Ty` (referenced as `real_imaginary_type` in the class API) must match the type of distribution
	 * `Distribution` (referenced as `distribution_type` in the class API) used, e.g. if the `real_imaginary_type` is `int`
	 * then it is undefined behaviour to use a distribution type intended for floating point types.
	 *
	 * \tparam Ty The type of the real and imaginary parts of the complex number, must
	 *         satisfy `std::is_arithmetic<Ty>`. Defaults to the floating type `double`.
	 * \tparam Generator The type of the generator engine to use for pseudo-random generation,
	 *         must meet the requirement of `UniformRandomBitGenerator` (see C++ Concepts). Defaults
	 *         to the engine type `std::mt19937`.
	 * \tparam Distribution The type of distribution over which to calculate the random numbers. The
	 *         value type of the distribution must match the value type `Ty` of this class. Must meet
	 *         the requirement of `RandomNumberDistribution` (see C++ Conecpts). Defaults to the
	 *         distribution type `std::uniform_real_distribution<Ty>`.
	 */
	template<class Ty = double,
		class Generator = std::mt19937,
		class Distribution = std::uniform_real_distribution<Ty>,
		class = std::enable_if_t<std::is_arithmetic<Ty>::value>
	> class random_complex_generator {
	public:
		// PUBLIC API TYPE DEFINITIONS
		typedef Ty real_imaginary_type;
		typedef typename std::complex<Ty> result_type;
		typedef Generator generator_type;
		typedef Distribution distribution_type;
		// CONSTRUCTION/ASSIGNMENT
		/**
		 * \brief Move constructs the generator with the values of the engine `_eng` and the
		 *        distribution `_dist`. This is also the default constructor.
		 *
		 * \param _eng Generator engine to use.
		 * \param _dist Distribution for random complex numbers.
		 */
		explicit random_complex_generator(Generator&& _eng = Generator{std::random_device{}()}, Distribution&& _dist = Distribution())
			: eng(std::move(_eng)), dist(std::move(_dist)) {}
		/**
		 * \brief Move constructs the generator with the values of the distribution `_dist` and
		 *        the engine `_eng`.
		 *
		 * \param _eng Generator engine to use.
		 * \param _dist Distribution to use.
		 */
		explicit random_complex_generator(Distribution&& _dist, Generator&& _eng = Generator{std::random_device{}()})
			: eng(std::move(_eng)), dist(std::move(_dist)) {}
		/**
		 * \brief Constructs the generator with a copy of the values of the engine `_eng` and
		 *        the distribution `_dist`.
		 *
		 * \param _eng Generator engine to use.
		 * \param _dist Distribution for random complex numbers.
		 */
		explicit random_complex_generator(const Generator& _eng, const Distribution& _dist)
			: eng(_eng), dist(_dist) {}
		/**
		 * \brief Copy constructor. Constructs the generator with a copy of the fields of `other`.
		 *
		 * \param other `random_complex_generator` instance to use as data source.
		 */
		random_complex_generator(const random_complex_generator& other)
			: eng(other.eng), dist(other.dist) {}
		/**
		 * \brief Move constructor. Constructs the generator with the fields of `other` using
		 *        move-semantics such that `other` is left in a valid but unspecified state.
		 *
		 * \param other `random_complex_generator` instance to use as data source.
		 */
		random_complex_generator(random_complex_generator&& other)
			: eng(std::move(other.eng)), dist(std::move(other.dist)) {}
		// GENERATING OPERATOR()
		/**
		 * \brief Generates the next random complex number in the distribution.
		 *
		 * \param is_equal_re_im Determines whether the real and imaginary
		 *        parts of the generated complex number will be equal or not.
		 * \return The generated random complex number.
		 */
		result_type operator()(bool is_equal_re_im = false) {
			if (!is_equal_re_im) return result_type(dist(eng), dist(eng));
			real_imaginary_type re_im = dist(eng);
			return result_type(re_im, re_im);
		}
		// GENERATOR AND DISTRIBUTION OBJECT ACCESS
		/**
		 * \brief Returns a copy of the underlying generator engine.
		 * \return A copy of the underlying engine used for random number generation.
		 */
		generator_type get_generator() const noexcept { return eng; }
		/**
		 * \brief Returns a copy of the underlying distribution.
		 * \return A copy of the underlying distribution over which the random
		 *         numbers are generated.
		 */
		distribution_type get_distribution() const noexcept { return dist; }
		// PROPERTIES
		/**
		 * \brief Returns the minimum potentially generated complex number where both the
		 *        real and imaginary parts are the minimum of the distribution.
		 * \return The minimum `std::complex` number potentially generated by the underlying distribution.
		 */
		result_type min() const { return result_type(dist.min(), dist.min()); }
		/**
		 * \brief Returns the maximum potentially generated complex number where both the
		 *        real and imaginary parts are the maximum of the distribution.
		 * \return The maximum `std::complex` number potentially generated by the underlying distribution.
		 */
		result_type max() const { return result_type(dist.max(), dist.max()); }
		// MODIFIERS
		/**
		 * \brief Resets the internal state of the underlying distribution object. After calling this function,
		 *        the next call to `operator()` on the generator will not be dependent upon previous calls
		 *        to `operator()`.
		 */
		void reset_distribution_state() { dist.reset(); }
	private:
		generator_type eng;
		distribution_type dist;
	};
#endif // !RANDOM_COMPLEX_GENERATOR_H
}



